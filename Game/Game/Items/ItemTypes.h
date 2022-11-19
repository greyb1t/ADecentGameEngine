#pragma once
#include "Common/BetterEnums/enum.h"

class GameObject;

BETTER_ENUM(eItemEvent, unsigned,  
	OnShoot,			//Triggered when the player shoots
	OnPlayerDamaged,	//Triggered when the player takes damage
	OnEnemyHit,			//Triggered when an enemy is hit
	OnEnemyKill,		//Triggered when an enemy is killed
	OnDash,				//Triggered when the player dashes
	OnPickup,			//Triggered when the item is picked up
	OnCrit,				//Triggered when the player crits on an enemy
	OnShotImpact		//Triggered when the player attacks hit a surface
	)

BETTER_ENUM(eItemTarget, unsigned,
	Player,
	Enemy,
	PlayerAOE,
	TargetAOE,
	EnemyExclusiveAOE
	)

BETTER_ENUM(eItemEffect, unsigned,
	Damage,
	Knockback,
	Stun,
	Heal,
	DamageOverTime,
	Barrier,
	StatShield, 
	StatAttackSpeed,
	StatDamage,
	StatJumpForce,
	Homing,
	Invulnerability,
	DragTowardPlayer,
	DragTowardTarget,
	SpawnChainLightning,
	StatRecievedDamageMod,
	StatMaxHealth,
	StatMaxHealthPercent,
	Pull,
	JumpAmountChange,
	CooldownMultiplierChange,
	Explosion,
	Evasion,
	StatCritChance,
	StatCritDamageMod,
	StatMoveSpeed,
	StatDamageModifier,
	DamagePercentage
)

BETTER_ENUM(eRarity, unsigned,
	Common, 
	//Uncommon,
	Rare,
	//Epic,
	Legendary
)

struct ItemEventData
{
	CU::Vector3f direction;
	GameObject* targetObject;
	Vec3f hitPos;
	float damage;
};