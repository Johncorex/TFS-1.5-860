// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#include "otpch.h"

#include "connection.h"

#include "configmanager.h"
#include "outputmessage.h"
#include "protocol.h"
#include "scheduler.h"
#include "server.h"

namespace {
constexpr uint32_t MAX_CONNECTIONS_PER_IP = 3;
constexpr uint32_t MAX_GLOBAL_CONNECTIONS = 2000;
constexpr uint32_t MAX_NEW_CONNECTIONS_PER_SECOND = 20;
} // namespace

Connection_ptr ConnectionManager::createConnection(boost::asio::io_service& io_service,
                                                   ConstServicePort_ptr servicePort)
{
	std::lock_guard<std::mutex> lockClass(connectionManagerLock);

	// Global connection limit — prevents server overload from volumetric DDoS
	if (connections.size() >= MAX_GLOBAL_CONNECTIONS) {
		std::cout << "Connection rejected: global connection limit reached ("
		          << connections.size() << "/" << MAX_GLOBAL_CONNECTIONS << ")" << std::endl;
		return nullptr;
	}

	auto connection = std::make_shared<Connection>(io_service, servicePort);

	// Per-IP connection limit (anti-WPE / anti-DDoS)
	if (connection->lastIp != 0 && ipConnectionCount[connection->lastIp] > MAX_CONNECTIONS_PER_IP) {
		std::cout << "Connection rejected: too many connections from IP "
		          << convertIPToString(connection->lastIp) << std::endl;
		connection->close(Connection::FORCE_CLOSE);
		return nullptr;
	}

	// Per-IP connection rate limit — blocks IPs opening connections too fast
	if (connection->lastIp != 0 && !isConnectionRateAllowed(connection->lastIp)) {
		std::cout << "Connection rejected: rate limit exceeded for IP "
		          << convertIPToString(connection->lastIp) << std::endl;
		connection->close(Connection::FORCE_CLOSE);
		return nullptr;
	}

	connections.insert(connection);
	return connection;
}

void ConnectionManager::releaseConnection(const Connection_ptr& connection)
{
	std::lock_guard<std::mutex> lockClass(connectionManagerLock);

	connections.erase(connection);
}

bool ConnectionManager::isIPAllowed(uint32_t ip) const
{
	std::lock_guard<std::mutex> lockClass(connectionManagerLock);
	auto it = ipConnectionCount.find(ip);
	if (it == ipConnectionCount.end()) {
		return true;
	}
	return it->second < MAX_CONNECTIONS_PER_IP;
}

void ConnectionManager::trackIP(uint32_t ip)
{
	std::lock_guard<std::mutex> lockClass(connectionManagerLock);
	++ipConnectionCount[ip];
}

void ConnectionManager::untrackIP(uint32_t ip)
{
	std::lock_guard<std::mutex> lockClass(connectionManagerLock);
	auto it = ipConnectionCount.find(ip);
	if (it != ipConnectionCount.end()) {
		if (it->second <= 1) {
			ipConnectionCount.erase(it);
		} else {
			--it->second;
		}
	}
}

bool ConnectionManager::isConnectionRateAllowed(uint32_t ip)
{
	auto now = static_cast<uint64_t>(OTSYS_TIME());
	auto& timestamps = connectionRateMap[ip];

	// Remove timestamps older than 1 second
	while (!timestamps.empty() && timestamps.front() <= now - 1000) {
		timestamps.erase(timestamps.begin());
	}

	if (timestamps.size() >= MAX_NEW_CONNECTIONS_PER_SECOND) {
		return false;
	}

	timestamps.push_back(now);
	return true;
}

uint32_t ConnectionManager::getConnectionCount() const
{
	std::lock_guard<std::mutex> lockClass(connectionManagerLock);
	return static_cast<uint32_t>(connections.size());
}

bool ConnectionManager::isGlobalConnectionLimitReached() const
{
	std::lock_guard<std::mutex> lockClass(connectionManagerLock);
	return connections.size() >= MAX_GLOBAL_CONNECTIONS;
}

void ConnectionManager::closeAll()
{
	std::lock_guard<std::mutex> lockClass(connectionManagerLock);

	for (const auto& connection : connections) {
		try {
			boost::system::error_code error;
			connection->socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);
			connection->socket.close(error);
		} catch (boost::system::system_error&) {
		}
	}
	connections.clear();
}

// Connection

void Connection::close(bool force)
{
	// any thread
	ConnectionManager::getInstance().releaseConnection(shared_from_this());

	std::lock_guard<std::recursive_mutex> lockClass(connectionLock);
	if (closed) {
		return;
	}
	closed = true;

	if (protocol) {
		g_dispatcher.addTask([protocol = protocol]() { protocol->release(); });
	}

	if (messageQueue.empty() || force) {
		closeSocket();
	} else {
		// will be closed by the destructor or onWriteOperation
	}
}

void Connection::markAuthenticated()
{
	authenticated = true;
}

void Connection::closeSocket()
{
	if (socket.is_open()) {
		try {
			readTimer.cancel();
			writeTimer.cancel();
			boost::system::error_code error;
			socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);
			socket.close(error);
		} catch (boost::system::system_error& e) {
			std::cout << "[Network error - Connection::closeSocket] " << e.what() << std::endl;
		}
	}
}

Connection::~Connection()
{
	if (lastIp != 0) {
		ConnectionManager::getInstance().untrackIP(lastIp);
	}
	closeSocket();
}

void Connection::accept(Protocol_ptr protocol)
{
	this->protocol = protocol;
	g_dispatcher.addTask([=]() { protocol->onConnect(); });

	accept();
}

void Connection::accept()
{
	std::lock_guard<std::recursive_mutex> lockClass(connectionLock);
	try {
		// Slowloris protection: unauth'd connections get shorter timeout
		int32_t timeout = authenticated ? CONNECTION_READ_TIMEOUT : CONNECTION_UNAUTHENTICATED_TIMEOUT;
		readTimer.expires_from_now(std::chrono::seconds(timeout));
		readTimer.async_wait(
		    [thisPtr = std::weak_ptr<Connection>(shared_from_this())](const boost::system::error_code& error) {
			    Connection::handleTimeout(thisPtr, error);
		    });

		// Read size of the first packet
		boost::asio::async_read(
		    socket, boost::asio::buffer(msg.getBuffer(), NetworkMessage::HEADER_LENGTH),
		    [thisPtr = shared_from_this()](const boost::system::error_code& error, auto /*bytes_transferred*/) {
			    thisPtr->parseHeader(error);
		    });
	} catch (boost::system::system_error& e) {
		std::cout << "[Network error - Connection::accept] " << e.what() << std::endl;
		close(FORCE_CLOSE);
	}
}

void Connection::parseHeader(const boost::system::error_code& error)
{
	std::lock_guard<std::recursive_mutex> lockClass(connectionLock);
	readTimer.cancel();

	if (error) {
		close(FORCE_CLOSE);
		return;
	} else if (closed) {
		return;
	}

	uint32_t timePassed = std::max<uint32_t>(1, (time(nullptr) - timeConnected) + 1);
	if ((++packetsSent / timePassed) > getInteger(ConfigManager::MAX_PACKETS_PER_SECOND)) {
		std::cout << convertIPToString(getIP()) << " disconnected for exceeding packet per second limit." << std::endl;
		close();
		return;
	}

	if (timePassed > 2) {
		timeConnected = time(nullptr);
		packetsSent = 0;
	}

	uint16_t size = msg.getLengthHeader();
	if (size == 0 || size >= NETWORKMESSAGE_MAXSIZE - 16) {
		close(FORCE_CLOSE);
		return;
	}

	try {
		int32_t timeout = authenticated ? CONNECTION_READ_TIMEOUT : CONNECTION_UNAUTHENTICATED_TIMEOUT;
		readTimer.expires_from_now(std::chrono::seconds(timeout));
		readTimer.async_wait(
		    [thisPtr = std::weak_ptr<Connection>(shared_from_this())](const boost::system::error_code& error) {
			    Connection::handleTimeout(thisPtr, error);
		    });

		// Read packet content
		msg.setLength(size + NetworkMessage::HEADER_LENGTH);
		boost::asio::async_read(
		    socket, boost::asio::buffer(msg.getBodyBuffer(), size),
		    [thisPtr = shared_from_this()](const boost::system::error_code& error, auto /*bytes_transferred*/) {
			    thisPtr->parsePacket(error);
		    });
	} catch (boost::system::system_error& e) {
		std::cout << "[Network error - Connection::parseHeader] " << e.what() << std::endl;
		close(FORCE_CLOSE);
	}
}

void Connection::parsePacket(const boost::system::error_code& error)
{
	std::lock_guard<std::recursive_mutex> lockClass(connectionLock);
	readTimer.cancel();

	if (error) {
		close(FORCE_CLOSE);
		return;
	} else if (closed) {
		return;
	}

	// Check packet checksum
	uint32_t checksum;
	int32_t len = msg.getLength() - msg.getBufferPosition() - NetworkMessage::CHECKSUM_LENGTH;
	if (len > 0) {
		checksum = adlerChecksum(msg.getBuffer() + msg.getBufferPosition() + NetworkMessage::CHECKSUM_LENGTH, len);
	} else {
		checksum = 0;
	}

	uint32_t recvChecksum = msg.get<uint32_t>();
	if (recvChecksum != checksum) {
		// it might not have been the checksum, step back
		msg.skipBytes(-NetworkMessage::CHECKSUM_LENGTH);
	}

	if (!receivedFirst) {
		// First message received
		receivedFirst = true;
		lastIp = getIP();

		if (!protocol) {
			// Game protocol has already been created at this point
			protocol = service_port->make_protocol(recvChecksum == checksum, msg, shared_from_this());
			if (!protocol) {
				close(FORCE_CLOSE);
				return;
			}
		} else {
			msg.skipBytes(1); // Skip protocol ID
		}

		protocol->onRecvFirstMessage(msg);
	} else {
		protocol->onRecvMessage(msg); // Send the packet to the current protocol
	}

	try {
		int32_t timeout = authenticated ? CONNECTION_READ_TIMEOUT : CONNECTION_UNAUTHENTICATED_TIMEOUT;
		readTimer.expires_from_now(std::chrono::seconds(timeout));
		readTimer.async_wait(
		    [thisPtr = std::weak_ptr<Connection>(shared_from_this())](const boost::system::error_code& error) {
			    Connection::handleTimeout(thisPtr, error);
		    });

		// Wait to the next packet
		boost::asio::async_read(
		    socket, boost::asio::buffer(msg.getBuffer(), NetworkMessage::HEADER_LENGTH),
		    [thisPtr = shared_from_this()](const boost::system::error_code& error, auto /*bytes_transferred*/) {
			    thisPtr->parseHeader(error);
		    });
	} catch (boost::system::system_error& e) {
		std::cout << "[Network error - Connection::parsePacket] " << e.what() << std::endl;
		close(FORCE_CLOSE);
	}
}

void Connection::send(const OutputMessage_ptr& msg)
{
	std::lock_guard<std::recursive_mutex> lockClass(connectionLock);
	if (closed) {
		return;
	}

	bool noPendingWrite = messageQueue.empty();
	messageQueue.emplace_back(msg);
	if (noPendingWrite) {
		internalSend(msg);
	}
}

void Connection::internalSend(const OutputMessage_ptr& msg)
{
	if (!protocol) {
		close(FORCE_CLOSE);
		return;
	}
	protocol->onSendMessage(msg);
	try {
		writeTimer.expires_from_now(std::chrono::seconds(CONNECTION_WRITE_TIMEOUT));
		writeTimer.async_wait(
		    [thisPtr = std::weak_ptr<Connection>(shared_from_this())](const boost::system::error_code& error) {
			    Connection::handleTimeout(thisPtr, error);
		    });

		boost::asio::async_write(
		    socket, boost::asio::buffer(msg->getOutputBuffer(), msg->getLength()),
		    [thisPtr = shared_from_this()](const boost::system::error_code& error, auto /*bytes_transferred*/) {
			    thisPtr->onWriteOperation(error);
		    });
	} catch (boost::system::system_error& e) {
		std::cout << "[Network error - Connection::internalSend] " << e.what() << std::endl;
		close(FORCE_CLOSE);
	}
}

uint32_t Connection::getIP()
{
	std::lock_guard<std::recursive_mutex> lockClass(connectionLock);

	// IP-address is expressed in network byte order
	boost::system::error_code error;
	const boost::asio::ip::tcp::endpoint endpoint = socket.remote_endpoint(error);
	if (error) {
		return 0;
	}

	return htonl(endpoint.address().to_v4().to_ulong());
}

void Connection::onWriteOperation(const boost::system::error_code& error)
{
	std::lock_guard<std::recursive_mutex> lockClass(connectionLock);
	writeTimer.cancel();
	messageQueue.pop_front();

	if (error) {
		messageQueue.clear();
		close(FORCE_CLOSE);
		return;
	}

	if (!messageQueue.empty()) {
		internalSend(messageQueue.front());
	} else if (closed) {
		closeSocket();
	}
}

void Connection::handleTimeout(ConnectionWeak_ptr connectionWeak, const boost::system::error_code& error)
{
	if (error == boost::asio::error::operation_aborted) {
		// The timer has been manually canceled
		return;
	}

	if (auto connection = connectionWeak.lock()) {
		connection->close(FORCE_CLOSE);
	}
}
