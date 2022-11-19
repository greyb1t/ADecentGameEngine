#pragma once

#include "Engine/Reflection/Enum.h"
#include "Engine/Reflection/Reflectable.h"
#include "Engine/Reflection/Reflector.h"

namespace Engine
{
	class AnimationController;
}

namespace Enemy
{
	struct eRapidShootData : public Engine::Reflectable
	{
		Vec3f mySpawnPosition = { 0.f, 0.f, 0.f };

		float myShootRange = 1500.f;
		float mySpeed = 0.f;
		void Reflect(Engine::Reflector& aReflector) override
		{
			aReflector.Reflect(mySpeed, "Projectile Speed");
		}
	};

	struct eGroundPopcornShootData : public Engine::Reflectable
	{
		Vec3f mySpawnPosition = { 0.f, 0.f, 0.f };

		float myShootRange = 1500.f;
		float mySpeed = 300.f;
		float myProjectileRadius = 5.f;
		void Reflect(Engine::Reflector& aReflector) override
		{
			aReflector.Reflect(mySpeed, "Projectile Speed");
			aReflector.Reflect(myProjectileRadius, "Projectile Radius");
		}
	};

	struct eBurstShootData : public Engine::Reflectable
	{
		float myDamage = 0.f;
		Vec3f mySpawnPosition = { 0.f, 0.f, 0.f };

		float myShootRange = 1500.f;
		float mySpeed = 0.f;
		float myProjectileRadius = 0.f;
		float myProjectileExplosionRadius = 0.f;
		void Reflect(Engine::Reflector& aReflector) override
		{
			aReflector.Reflect(mySpeed, "Projectile Speed");
			aReflector.Reflect(myProjectileRadius, "ProjectileRadius");
			aReflector.Reflect(myProjectileExplosionRadius, "Projectile Explosion Radius");
		}
	};

	struct eMortarShootData : public Engine::Reflectable
	{
		float myDamage = 0.f;
		Vec3f mySpawnPosition = { 0.f, 0.f, 0.f };

		float myShootRange = 1500.f;
		float mySpeed = 0.f;
		float myProjectileRadius = 0.f;
		float myExplosionRadius = 0.f;
		void Reflect(Engine::Reflector& aReflector) override
		{
			aReflector.Reflect(mySpeed, "Projectile Speed");
			aReflector.Reflect(myProjectileRadius, "Projectile Radius");
			aReflector.Reflect(myExplosionRadius, "Explosion Radius");
		}
	};

	void EnemyTrigger(Engine::AnimationController& animController, const std::string& aTriggerName);
	void SetEnemyCondition(Engine::AnimationController& animController, const std::string& aConditionName, bool aValue);

	enum class eEnemyType
	{
		Ground,
		Flying,

		Count
	};
	class EnemyType : public Engine::Enum
	{
		std::string EnumToString(int aValue) override
		{
			switch (static_cast<eEnemyType>(aValue))
			{
			case eEnemyType::Ground: return "Ground";
			case eEnemyType::Flying: return "Flying";
			}
			return "undefined";
		};
		int GetCount() override { return static_cast<int>(eEnemyType::Count); }

	private:
	};
}

