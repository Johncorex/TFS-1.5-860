-- ============================================================
-- Boosted Creature by Johncorex
-- Todo dia (virada 00h): 1 boss + 1 monstro boostados
-- Bonus: +5% experience e +5% loot (rolagem extra por item)
-- Sorteio deterministico pela data: igual o dia todo, sobrevive a restart.
-- Edite as listas abaixo para incluir/remover criaturas do sorteio.
-- ============================================================

local BOOSTED_BOSSES = {
	"Demodras", "Orshabaal", "Ferumbras", "Morgaroth",
	"Ghazbaran", "Dracola", "Necropharus", "The Old Widow",
}

local BOOSTED_MONSTERS = {
	"Dragon", "Dragon Lord", "Frost Dragon", "Hydra",
	"Serpent Spawn", "Behemoth", "Destroyer", "Demon",
	"Grim Reaper", "Nightmare", "Medusa", "Hellhound",
	"Juggernaut", "Spectre",
}

local BOOST_EXP = 1.05
local BOOST_LOOT_CHANCE = 5 -- % de chance extra por item do loot

local function validNames(list, kind)
	local valid = {}
	for _, name in ipairs(list) do
		if MonsterType(name) then
			valid[#valid + 1] = name
		else
			print(string.format("[Boosted Creature] '%s' nao existe (%s), ignorado.", name, kind))
		end
	end
	return valid
end

local function pickDaily(list)
	-- Dia "do save": vira junto com o server save das 23:55 (offset em segundos)
	local daySeed = tonumber(os.date("%Y%j", os.time() - (23 * 3600 + 55 * 60)))
	math.randomseed(daySeed)
	local choice = list[math.random(#list)]
	math.randomseed(os.time())
	return choice
end

local validBosses = validNames(BOOSTED_BOSSES, "boss")
local validMonsters = validNames(BOOSTED_MONSTERS, "monstro")
if #validBosses == 0 or #validMonsters == 0 then
	print("[Boosted Creature] ERRO: listas vazias, sistema desativado.")
	return
end

local boostedBoss = pickDaily(validBosses)
local boostedMonster = pickDaily(validMonsters)
print(string.format("[Boosted Creature] Hoje: boss %s, monstro %s.", boostedBoss, boostedMonster))

local function isBoosted(name)
	return name == boostedBoss or name == boostedMonster
end

-- == Login: anuncia os boostados do dia ==
local boostedLogin = CreatureEvent("boostedCreatureLogin")

function boostedLogin.onLogin(player)
	player:sendTextMessage(MESSAGE_STATUS_CONSOLE_BLUE,
	                       string.format("[Boosted Creature] O boss %s e o monstro %s estão boostados, podendo ter 5%% a mais de drop loot e experience.",
	                                             boostedBoss, boostedMonster))
	return true
end

boostedLogin:register()

-- == Bonus: +5% experience ==
local boostedExpEvent = Event()

function boostedExpEvent.onGainExperience(player, source, exp, rawExp)
	if source and source:isMonster() and isBoosted(source:getName()) then
		return exp * BOOST_EXP
	end
	return exp
end

boostedExpEvent:register(2)

-- == Bonus: +5% loot (rolagem extra por item) ==
local boostedLootEvent = Event()

function boostedLootEvent.onDropLoot(monster, corpse)
	if not isBoosted(monster:getName()) then
		return
	end
	local owner = Player(corpse:getCorpseOwner())
	if not owner then
		return
	end
	local mType = monster:getType()
	if not mType then
		return
	end
	for _, lootEntry in ipairs(mType:getLoot()) do
		if math.random(100) <= BOOST_LOOT_CHANCE then
			corpse:createLootItem(lootEntry)
		end
	end
end

boostedLootEvent:register(2)
