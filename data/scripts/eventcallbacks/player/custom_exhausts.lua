-- ============================================================
-- Independent exhausts by Johncorex
-- Potion: 800ms | Runes: 1000ms | Healing spells: 1000ms
-- Each lane has its own timestamp, one never blocks the other.
-- (Attack spells keep the engine COMBAT exhaust from spells.xml.)
-- ============================================================

local POTION_EXHAUST_MS = 800
local RUNE_EXHAUST_MS = 1000
local HEAL_EXHAUST_MS = 1000

local HEAL_SPELLS = {
	["Light Healing"] = true, -- exura
	["Intense Healing"] = true, -- exura gran
	["Ultimate Healing"] = true, -- exura vita
	["Heal Friend"] = true, -- exura sio
	["Divine Healing"] = true, -- exura san
	["Mass Healing"] = true, -- exura gran mas res
	["Cure Poison"] = true, -- exana pox
}

local RUNE_SPELLS = {
	["Avalanche Rune"] = true,
	["Energy Bomb Rune"] = true,
	["Energy Field Rune"] = true,
	["Energy Wall Rune"] = true,
	["Explosion Rune"] = true,
	["Fire Bomb Rune"] = true,
	["Fire Field Rune"] = true,
	["Fire Wall Rune"] = true,
	["Fireball Rune"] = true,
	["Great Fireball Rune"] = true,
	["Heavy Magic Missile Rune"] = true,
	["Holy Missile Rune"] = true,
	["Icicle Rune"] = true,
	["Magic Wall Rune"] = true,
	["Poison Bomb Rune"] = true,
	["Poison Field Rune"] = true,
	["Poison Wall Rune"] = true,
	["Soulfire Rune"] = true,
	["Stalagmite Rune"] = true,
	["Stone Shower Rune"] = true,
	["Sudden Death Rune"] = true,
	["Thunderstorm Rune"] = true,
	["Cure Poison Rune"] = true,
	["Intense Healing Rune"] = true,
	["Ultimate Healing Rune"] = true,
	["Animate Dead Rune"] = true,
	["Convince Creature Rune"] = true,
	["Chameleon Rune"] = true,
	["Disintegrate Rune"] = true,
	["Destroy Field Rune"] = true,
	["Wild Growth Rune"] = true,
	["Paralyze Rune"] = true,
}

local function checkExhaust(player, storageKey, windowMs)
	if os.mtime() < player:getStorageValue(storageKey) then
		player:sendCancelMessage(RETURNVALUE_YOUAREEXHAUSTED)
		return false
	end
	player:setStorageValue(storageKey, os.mtime() + windowMs)
	return true
end

-- Potion lane (used by data/actions/scripts/other/potions.lua)
function checkPotionExhaust(player)
	if player:getGroup():getAccess() then
		return true -- staff bypass
	end
	return checkExhaust(player, PlayerStorageKeys.exhaustPotion, POTION_EXHAUST_MS)
end

-- Rune + Heal lanes (hooked on every spell cast, engine exhaust removed)
local exhaustEvent = Event()

function exhaustEvent.onSpellCheck(player, spell)
	if player:getGroup():getAccess() then
		return true -- staff bypass
	end
	local spellName = spell:name()
	if HEAL_SPELLS[spellName] then
		return checkExhaust(player, PlayerStorageKeys.exhaustHeal, HEAL_EXHAUST_MS)
	end
	if RUNE_SPELLS[spellName] then
		return checkExhaust(player, PlayerStorageKeys.exhaustRune, RUNE_EXHAUST_MS)
	end
	return true -- attack instants: engine COMBAT exhaust applies
end

exhaustEvent:register()
