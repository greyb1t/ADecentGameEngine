#pragma once

#include "Physics/Types.h"
#include "Engine/Reflection/Enum.h"

// Must be same order as eTagStrings
enum class eTag
{
	DEFAULT,

	PLAYER,
	PLAYERATTACK,

	ENEMY,
	ENEMYHEAD,
	ENEMYATTACK,
	ENEMYPROJECTILE,

	PLAYERSHIELD,

	ROOT,
	DOOR,

	RESPAWNZONE,

	DESTRUCTABLE,

	SCRIPTEDEVENTACTOR,
	EVENTBUFF,

	CHECKPOINT,

	CUBEMAPLERP,

	END,

	FINALBOSS,

	COUNT
};

// Must be same order as eTag
const static std::string eTagStrings[] =
{
	"DEFAULT",
	"PLAYER",
	"PLAYERATTACK",
	"ENEMY",
	"ENEMYHEAD",
	"ENEMYATTACK",
	"ENEMYPROJECTILE",
	"PLAYERSHIELD",
	"ROOT",
	"DOOR",
	"RESPAWNZONE",
	"DESTRUCTABLE",
	"SCRIPTEDEVENTACTOR",
	"EVENTBUFF",
	"CHECKPOINT",
	"CUBEMAPLERP",
	"END",
	"FINALBOSS",
	"COUNT"
};

static_assert((std::size(eTagStrings) - 1) == static_cast<int>(eTag::COUNT) && 
	"eTagStrings and eTag must be same size");

class eTagClass : public Engine::Enum
{
public:
	eTagClass(const int aValue) : Engine::Enum(aValue)
	{
	}

	std::string EnumToString(int aValue) override
	{
		return eTagStrings[aValue];
	}

	int GetCount() override
	{
		return static_cast<int>(eTag::COUNT);
	}
};

struct eLayer
{
	enum Enum
	{
		DEFAULT = (1 << 0),
		GROUND = (1 << 1),

		PLAYER = (1 << 2),
		ENEMY = (1 << 3),

		ZONE = (1 << 4),
		POTION = (1 << 5),

		ROOT = (1 << 6),
		DOOR = (1 << 7),
		EVENTBUFF = (1 << 8),
		MOVABLE = (1 << 9),
		IMMOVABLE = (1 << 10),

		PROPS = (1 << 13),

		RESPAWNZONE = (1 << 14),

		// DESTRUCTABLES
		DESTRUCTABLE = (1 << 15),
		FRACTURE = (1 << 16),
		// -----------

		// DAMAGEABLE specifies that it contains a HealthComponent
		PLAYER_DAMAGEABLE = (1 << 20), // Object that is player object and damageable
		ENEMY_DAMAGEABLE = (1 << 21), // Object that is an enemy object and damageable
		ENVIRONMENT_DAMAGEABLE = (1 << 22), // Object that is environment object and damageable
		DAMAGEABLE = PLAYER_DAMAGEABLE | ENEMY_DAMAGEABLE | ENVIRONMENT_DAMAGEABLE, // All damageable objects
		NONPLAYERDAMAGEABLE = ENEMY_DAMAGEABLE | ENVIRONMENT_DAMAGEABLE, // All damageable objects

		PLAYER_PROJECTILE_BLOCKING = (1 << 23),
		PLAYER_SHIELD = (1 << 24),

		LOD = (1 << 25),

		PROTOTYPE = (1 << 28),
		TERRAIN = (1 << 29),
		NAVMESH = (1 << 31),


		NONE = 0,
		CLICKABLE = TERRAIN | NAVMESH,
		ENVIRONMENT = TERRAIN | GROUND,
		MASK_BURSTPROJECTILE = DEFAULT | GROUND | PLAYER_DAMAGEABLE | PLAYER_SHIELD,

		PLAYERSHOOTABLE = DEFAULT | GROUND | ENEMY,
		PLAYER_PROJECTILE_HIT = DEFAULT | GROUND | NONPLAYERDAMAGEABLE | PLAYER_PROJECTILE_BLOCKING,
		SHOTOBSTACLE = DEFAULT | GROUND | PLAYER_SHIELD, // Enemy shot layer

		CAMERACOLLIDABLE = DEFAULT | ENVIRONMENT,

		ALL = INT_MAX
	};

	static const std::string ToString(const LayerMask aEnum)
	{
		std::string result;

		if (aEnum & DEFAULT)
		{
			result += "DEFAULT, ";
		}
		if (aEnum & GROUND)
		{
			result += "GROUND, ";
		}
		if (aEnum & PLAYER)
		{
			result += "PLAYER, ";
		}
		if (aEnum & ENEMY)
		{
			result += "ENEMY, ";
		}
		if (aEnum & ZONE)
		{
			result += "ZONE, ";
		}
		if (aEnum & POTION)
		{
			result += "POTION, ";
		}
		if (aEnum & PLAYER_SHIELD)
		{
			result += "PLAYER SHIELD, ";
		}
		if (aEnum & ROOT)
		{
			result += "ROOT, ";
		}
		if (aEnum & DOOR)
		{
			result += "DOOR, ";
		}
		if (aEnum & RESPAWNZONE)
		{
			result += "RESPAWNZONE, ";
		}
		if (aEnum & EVENTBUFF)
		{
			result += "EVENTBUFF, ";
		}
		if (aEnum & MOVABLE)
		{
			result += "MOVABLE, ";
		}
		if (aEnum & PROPS)
		{
			result += "PROPS, ";
		}
		if (aEnum & DESTRUCTABLE)
		{
			result += "DESTRUCTABLE, ";
		}
		if (aEnum & FRACTURE)
		{
			result += "FRACTURE, ";
		}
		if (aEnum & PLAYER_DAMAGEABLE)
		{
			result += "PLAYER_DAMAGEABLE, ";
		}
		if (aEnum & ENEMY_DAMAGEABLE)
		{
			result += "ENEMY_DAMAGEABLE, ";
		}
		if (aEnum & ENVIRONMENT_DAMAGEABLE)
		{
			result += "ENVIRONMENT_DAMAGEABLE, ";
		}
		if (aEnum & LOD)
		{
			result += "LOD, ";
		}
		if (aEnum & PROTOTYPE)
		{
			result += "PROTOTYPE, ";
		}
		if (aEnum & TERRAIN)
		{
			result += "TERRAIN, ";
		}
		if (aEnum & NAVMESH)
		{
			result += "NAVMESH, ";
		}
		if (aEnum & PLAYER_PROJECTILE_BLOCKING)
		{
			result += "PLAYER_PROJECTILE_BLOCKING, ";
		}
		if (aEnum & IMMOVABLE)
		{
			result += "IMMOVABLE, ";
		}

		if (!result.empty())
		{
			result = result.substr(0, result.size() - 2);
		}

		return result;
	}
};