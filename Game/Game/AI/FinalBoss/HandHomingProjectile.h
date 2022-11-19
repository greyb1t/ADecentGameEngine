#pragma once

#include "Engine/GameObject/Components/Component.h"
#include "Engine/ResourceManagement/ResourceRef.h"
#include "Utils/TickTimer.h"

namespace FB
{
	class HandHomingProjectile : public Component
	{
	public:
		COMPONENT(HandHomingProjectile, "FB Hand Homing Projectile");

		enum class State
		{
			Launching,
			MoveTowardsTarget
		};

		void LaunchTowards(const Vec3f& aFrom, Weak<GameObject> aTarget, const Vec3f& aOffset, const int aProjectileNumber);
		void SetDamage(const float aDamage);
		void SetSecondsUntilSelfDestroy(const float aSeconds);
		void SetProjectileSpeed(const float aSpeed);

	private:
		void OnLostTarget();
		void OnDamaged(const float aDamage);
		bool OnDeath(const float aDamage);
		void OnCollidePlayer(GameObject* aPlayerGameObject);
		void ExplodeDestroyMyself();

	protected:
		void Start() override;

		void Execute(Engine::eEngineOrder aOrder) override;
		void Reflect(Engine::Reflector& aReflector) override;

	private:
		State myState = State::Launching;

		// TODO: Should accept a curve for the speed
		// and I want it to move randomly left/right upwards, then aim down toward player and shoot it
		// basically move on a bezier curve towards the player
		AnimationCurveRef myLaunchSpeedCurve;
		VFXRef myExplosionVFX;

		float myLaunchUpDistance = 1500.f;
		float myLaunchDistance = 1500.f;
		float myLaunchDuration = 2.f;
		BezierSpline myLaunchBezier;
		TickTimer myLaunchTimer;
		float myLaunchDelay = 0.1f;
		TickTimer myLaunchDelayTimer;

		float mySpeed = 2000.f;

		Weak<GameObject> myTarget;

		TickTimer myDestroyMyselfTimer;
		float mySecondsUntilSelfDestroy = 15.f;

		float myDamage = 0.f;

		int myProjectileNumber = 0;

		bool myHasCollided = false;
	};
}