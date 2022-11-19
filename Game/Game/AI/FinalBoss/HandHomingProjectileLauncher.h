#pragma once

#include "Engine/GameObject/Components/Component.h"
#include "Utils/TickTimer.h"

namespace FB
{
	class HandHomingProjectileLauncher : public Component
	{
	public:
		COMPONENT(HandHomingProjectileLauncher, "FB Homing Projectile Launcher");

		void Start() override;

		void Execute(Engine::eEngineOrder aOrder) override;

		void Reflect(Engine::Reflector& aReflector) override;

		void SetProjectileDamage(const float aDamage);
		void SetProjectilesCount(const int aProjectilesCount);
		void SetSphereOffsetLength(const float aOffsetLength);
		void SetSecondsUntilSelfDestroy(const float aSeconds);
		void SetProjectileSpeed(const float aSpeed);

		void Implode();

	private:
		int myProjectilesCount = 5;
		GameObjectPrefabRef myProjectilePrefab;
		float mySphereOffsetLength = 700.f;
		AnimationCurveRef myImplodeScale;

		TickTimer myImplodeTimer;

		float myProjectileDamage = 0.f;

		float mySecondsUntilSelfDestroy = 15.f;
		float myProjectileSpeed = 3000.f;
	};
};