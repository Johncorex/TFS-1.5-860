// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#include "otpch.h"

#include "mailbox.h"

#include "game.h"
#include "iologindata.h"

extern Game g_game;

namespace {
constexpr int32_t MAX_PARCEL_ITEMS = 200;
constexpr int32_t MAX_PARCEL_DEPTH = 5;

int32_t countParcelItems(const Container* container, int32_t depth = 0)
{
	if (depth > MAX_PARCEL_DEPTH) {
		return MAX_PARCEL_ITEMS + 1;
	}

	int32_t count = 0;
	for (const Item* item : container->getItemList()) {
		++count;
		if (count > MAX_PARCEL_ITEMS) {
			return count;
		}
		if (const Container* subContainer = item->getContainer()) {
			count += countParcelItems(subContainer, depth + 1);
			if (count > MAX_PARCEL_ITEMS) {
				return count;
			}
		}
	}
	return count;
}
} // namespace

ReturnValue Mailbox::queryAdd(int32_t, const Thing& thing, uint32_t, uint32_t, Creature*) const
{
	const Item* item = thing.getItem();
	if (item && Mailbox::canSend(item)) {
		if (const Container* container = item->getContainer()) {
			int32_t itemCount = countParcelItems(container);
			if (itemCount > MAX_PARCEL_ITEMS) {
				return RETURNVALUE_NOTPOSSIBLE;
			}
		}
		return RETURNVALUE_NOERROR;
	}
	return RETURNVALUE_NOTPOSSIBLE;
}

ReturnValue Mailbox::queryMaxCount(int32_t, const Thing&, uint32_t count, uint32_t& maxQueryCount, uint32_t) const
{
	maxQueryCount = std::max<uint32_t>(1, count);
	return RETURNVALUE_NOERROR;
}

ReturnValue Mailbox::queryRemove(const Thing&, uint32_t, uint32_t, Creature* /*= nullptr */) const
{
	return RETURNVALUE_NOTPOSSIBLE;
}

Cylinder* Mailbox::queryDestination(int32_t&, const Thing&, Item**, uint32_t&) { return this; }

void Mailbox::addThing(Thing* thing) { return addThing(0, thing); }

void Mailbox::addThing(int32_t, Thing* thing)
{
	Item* item = thing->getItem();
	if (item && Mailbox::canSend(item)) {
		if (const Container* container = item->getContainer()) {
			if (countParcelItems(container) > MAX_PARCEL_ITEMS) {
				return;
			}
		}
		sendItem(item);
	}
}

void Mailbox::updateThing(Thing*, uint16_t, uint32_t)
{
	//
}

void Mailbox::replaceThing(uint32_t, Thing*)
{
	//
}

void Mailbox::removeThing(Thing*, uint32_t)
{
	//
}

void Mailbox::postAddNotification(Thing* thing, const Cylinder* oldParent, int32_t index, cylinderlink_t)
{
	getParent()->postAddNotification(thing, oldParent, index, LINK_PARENT);
}

void Mailbox::postRemoveNotification(Thing* thing, const Cylinder* newParent, int32_t index, cylinderlink_t)
{
	getParent()->postRemoveNotification(thing, newParent, index, LINK_PARENT);
}

bool Mailbox::sendItem(Item* item) const
{
	std::string receiver;
	uint32_t depotId = 0;
	if (!getReceiver(item, receiver, depotId)) {
		return false;
	}

	/**No need to continue if its still empty**/
	if (receiver.empty() || depotId == 0) {
		return false;
	}

	Player* player = g_game.getPlayerByName(receiver);
	if (player) {
		DepotLocker* depotLocker = player->getDepotLocker(depotId);
		if (depotLocker) {
			if (g_game.internalMoveItem(item->getParent(), depotLocker, INDEX_WHEREEVER, item, item->getItemCount(),
			                            nullptr) == RETURNVALUE_NOERROR) {
				g_game.transformItem(item, item->getID() + 1);
				player->onReceiveMail();
				return true;
			}
		}
	} else {
		Player tmpPlayer(nullptr);
		if (!IOLoginData::loadPlayerByName(&tmpPlayer, receiver)) {
			return false;
		}

		if (DepotLocker* depotLocker = tmpPlayer.getDepotLocker(depotId)) {
			if (g_game.internalMoveItem(item->getParent(), depotLocker, INDEX_WHEREEVER, item, item->getItemCount(),
			                            nullptr) == RETURNVALUE_NOERROR) {
				g_game.transformItem(item, item->getID() + 1);
				IOLoginData::savePlayer(&tmpPlayer);
				return true;
			}
		}
	}
	return false;
}

bool Mailbox::getReceiver(Item* item, std::string& name, uint32_t& depotId) const
{
	const Container* container = item->getContainer();
	if (container) {
		for (Item* containerItem : container->getItemList()) {
			if (containerItem->getID() == ITEM_LABEL && getReceiver(containerItem, name, depotId)) {
				return true;
			}
		}
		return false;
	}

	auto text = item->getText();
	if (text.empty()) {
		return false;
	}

	name = getFirstLine(text);
	boost::algorithm::trim(name);
	std::string townName = getStringLine(text, 2);

	Town* town = g_game.map.towns.getTown(townName);
	if (town) {
		depotId = town->getID();
		return true;
	}

	return false;
}

bool Mailbox::canSend(const Item* item) { return item->getID() == ITEM_PARCEL || item->getID() == ITEM_LETTER; }
