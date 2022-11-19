#pragma once

struct SkillStats
{
	struct Primary
	{
		float damageMultiplier = 1.f;
		float attackSpeedMultiplier = 3.5f;
		float projectileSpeed = 20000;
		float projectileRange = 20000;

		float steadinessSpread = 5;
		float steadinessShootLoss = .2f;
		float steadinessWeightPower = 2;

		float projectileLength = 200.f;
	};

	struct Secondary
	{
		float baseDamage = 25;
		float playerDamageMultiplier = 1;
		float chargeTime = 1.f;
		float minDamageMultiplier = 2.f;
		float maxDamageMultiplier = 5.f;
		float damageRadius = 50;

		float explosionDamageMultiplier = 5.f;
		float explosionMinRange = 25.f;
		float explosionMaxRange = 200.f;

		float explosionTime = .25f;

		float explosionKnockback = 200.f;


		float bulletSpeed = 20000;
		float projectileRange = 5000;
		float minProjectileSpeed = 2000;
		float maxProjectileSpeed = 10000;
		float cooldown = 3;

	};

	struct Mobility
	{
		float dashDistance = 666;
		float dashInTime = 0.025f;
		float dashTime = 0.3f;
		float dashOutTime = 0.025f;
		float cooldown = 5;
	};

	struct Utility
	{
		float baseDamage = 75;
		float damageMultiplier = 10.f;
		float bounces = 6;
		float cooldown = 9;
		float projectileRange = 5000;
		float projectileSpeed = 5000;
		float bounceRange = 1000;
		float bounceProjectileSpeed = 1000;
	};

	Primary		primary;
	Secondary	secondary;
	Mobility	mobility;
	Utility		utility;
};