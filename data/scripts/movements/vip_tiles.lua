-- ============================================================
-- VIP only tiles by Johncorex
-- SQM 1: action id 40040 | SQM 2: action id 40041
-- Only players with active VIP (isVip from vip_system.lua) may step in.
-- ============================================================

local VIP_TILE_AID_1 = 40040
local VIP_TILE_AID_2 = 40041

local function checkVipTile(creature, item, position, fromPosition)
	local player = creature:getPlayer()
	if not player then
		return true
	end
	if not isVip(player) then
		player:teleportTo(fromPosition, true)
		player:sendTextMessage(MESSAGE_STATUS_CONSOLE_BLUE, "Only VIP players may pass here.")
	end
	return true
end

local vipTile1 = MoveEvent()
vipTile1:type("stepin")

function vipTile1.onStepIn(creature, item, position, fromPosition)
	return checkVipTile(creature, item, position, fromPosition)
end

vipTile1:aid(VIP_TILE_1)
vipTile1:register()

local vipTile2 = MoveEvent()
vipTile2:type("stepin")

function vipTile2.onStepIn(creature, item, position, fromPosition)
	return checkVipTile(creature, item, position, fromPosition)
end

vipTile2:aid(VIP_TILE_2)
vipTile2:register()
