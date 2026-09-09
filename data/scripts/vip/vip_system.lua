-- ============================================================
-- VIP System by Johncorex
-- Item 8981 (VIP 30 Days): click to receive +30 days (accumulates)
-- Benefits: +10% exp, +10% skills, +10% loot,
--           full addons Pirate/Beggar/Shaman (sex aware)
-- VIP check function for reuse (ex: movements): isVip(player)
-- ============================================================

local VIP_ITEMID = 8981
local VIP_DAYS_PER_USE = 30
local VIP_STORAGE = PlayerStorageKeys.vipExpires
local VIP_BONUS = 1.1
local VIP_ADDON_FULL = 3
local VIP_OUTFITS_MALE = { 155, 157, 158 } -- Pirate, Beggar, Shaman
local VIP_OUTFITS_FEMALE = { 151, 153, 154 } -- Pirate, Beggar, Shaman
local DAY_SECONDS = 24 * 60 * 60

-- Literal VIP function: returns true while the player has VIP time left.
-- Use it anywhere (movements, NPCs, talkactions): if isVip(player) then ...
function isVip(player)
	if not player then
		return false
	end
	return player:getStorageValue(VIP_STORAGE) > os.time()
end

local function vipDaysLeft(player)
	local diff = player:getStorageValue(VIP_STORAGE) - os.time()
	if diff <= 0 then
		return 0
	end
	return math.floor(diff / DAY_SECONDS)
end

local function vipOutfits(player)
	if player:getSex() == PLAYERSEX_MALE then
		return VIP_OUTFITS_MALE
	end
	return VIP_OUTFITS_FEMALE
end

local function grantVipAddons(player)
	for _, lookType in ipairs(vipOutfits(player)) do
		player:addOutfit(lookType)
		player:addOutfitAddon(lookType, VIP_ADDON_FULL)
	end
end

local function hasAnyVipAddon(player)
	for _, lookType in ipairs(vipOutfits(player)) do
		if player:hasOutfit(lookType, VIP_ADDON_FULL) then
			return true
		end
	end
	return false
end

local function stripVipAddons(player)
	for _, lookType in ipairs(vipOutfits(player)) do
		player:removeOutfitAddon(lookType, VIP_ADDON_FULL)
		player:removeOutfit(lookType)
	end
end

-- Adds VIP days (accumulates on top of remaining time) and grants addons.
-- Returns the new total of days remaining.
function addVipDays(player, days)
	local current = player:getStorageValue(VIP_STORAGE)
	local base = math.max(os.time(), current)
	player:setStorageValue(VIP_STORAGE, base + days * DAY_SECONDS)
	grantVipAddons(player)
	return vipDaysLeft(player)
end

-- == Use: VIP 30 Days scroll ==
local vipScroll = Action()

function vipScroll.onUse(player, item, fromPosition, target, toPosition, isHotkey)
	local totalDays = addVipDays(player, VIP_DAYS_PER_USE)
	item:remove(1)
	player:sendTextMessage(MESSAGE_EVENT_ADVANCE,
	                       string.format("You received %d days of VIP! Total VIP time: %d days.", VIP_DAYS_PER_USE,
	                                             totalDays))
	return true
end

vipScroll:id(VIP_ITEMID)
vipScroll:register()

-- == Login: cleanup expired VIP + announce remaining days ==
local vipLogin = CreatureEvent("vipLogin")

function vipLogin.onLogin(player)
	if isVip(player) then
		player:sendTextMessage(MESSAGE_STATUS_CONSOLE_BLUE,
		                       string.format("You have %d days of VIP remaining.", vipDaysLeft(player)))
	elseif hasAnyVipAddon(player) then
		stripVipAddons(player)
		player:sendTextMessage(MESSAGE_STATUS_CONSOLE_BLUE, "Your VIP has expired.")
	end
	return true
end

vipLogin:register()

-- == Command: !vip (remaining days) ==
local vipCommand = TalkAction("!vip")

function vipCommand.onSay(player, words, param)
	if isVip(player) then
		player:sendTextMessage(MESSAGE_STATUS_CONSOLE_BLUE,
		                       string.format("You have %d days of VIP remaining.", vipDaysLeft(player)))
	else
		player:sendTextMessage(MESSAGE_STATUS_CONSOLE_BLUE, "You don't have an active VIP.")
	end
	return false
end

vipCommand:register()

-- == Sweep: removes addons from online players whose VIP expired ==
local vipSweep = GlobalEvent("vipSweep")

function vipSweep.onThink(interval)
	for _, onlinePlayer in pairs(Game.getPlayers()) do
		if not isVip(onlinePlayer) and hasAnyVipAddon(onlinePlayer) then
			stripVipAddons(onlinePlayer)
			onlinePlayer:sendTextMessage(MESSAGE_STATUS_CONSOLE_BLUE, "Your VIP has expired.")
		end
	end
	return true
end

vipSweep:interval(5 * 60 * 1000) -- 5 minutes
vipSweep:register()

-- == Bonus: +10% experience ==
local vipExpEvent = Event()

function vipExpEvent.onGainExperience(player, source, exp, rawExp)
	if isVip(player) then
		return exp * VIP_BONUS
	end
	return exp
end

vipExpEvent:register(1)

-- == Bonus: +10% skills ==
local vipSkillEvent = Event()

function vipSkillEvent.onGainSkillTries(player, skill, tries, artificial)
	if isVip(player) then
		return tries * VIP_BONUS
	end
	return tries
end

vipSkillEvent:register(1)

-- == Bonus: +10% loot (extra roll per loot entry) ==
local vipLootEvent = Event()

function vipLootEvent.onDropLoot(monster, corpse)
	local owner = Player(corpse:getCorpseOwner())
	if not owner or not isVip(owner) then
		return
	end
	local mType = monster:getType()
	if not mType then
		return
	end
	for _, lootEntry in ipairs(mType:getLoot()) do
		if math.random(100) <= 10 then
			corpse:createLootItem(lootEntry)
		end
	end
end

vipLootEvent:register(1)
