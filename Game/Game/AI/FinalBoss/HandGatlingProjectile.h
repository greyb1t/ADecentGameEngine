#pragma once

#include "Engine/GameObject/Components/Component.h"
#include "Engine/ResourceManagement/ResourceRef.h"
#include "Utils/TickTimer.h"

namespace FB
{
	class HandGatlingProjectile : public Component
	{
	public:
		COMPONENT(HandGatlingProjectile, "FB Hand Gatling Projectile");

		void Launch(const Vec3f& aFrom, const Vec3f& aDirection, const float aDistance);

		void SetDamage(const float aDamage);
		void SetSecondsUntilSelfDestroy(const float aSeconds);

	private:
		void OnLostTarget();
		void OnDamaged(const float aDamage);
		bool OnDeath(const float aDamage);
		void OnCollidePlayer(GameObject* aPlayerGameObject);

	protected:
		void Start() override;

		void Execute(Engine::eEngineOrder aOrder) override;
		void Reflect(Engine::Reflector& aReflector) override;
		void DamagePlayer(const Vec3f& aFrom, const Vec3f& aDirection, const float aDistance);
		void CreateImpactVFXOnGround(const Vec3f& aFrom, const Vec3f& aDirection, const float aDistance);

	private:
		VFXRef myImpactVFX;

		TickTimer myDestroyMyselfTimer;
		float mySecondsUntilSelfDestroy = 15.f;

		Vec3f myDirection;

		float myDamage = 1.f;
	};
}