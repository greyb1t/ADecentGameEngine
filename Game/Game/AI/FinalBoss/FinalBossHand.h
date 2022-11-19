#pragma once

#include "Engine/GameObject/Components/Component.h"
#include "Engine/GameObject/GameObjectRef.h"
#include "Behaviours/HandCrawler.h"
#include "Utils/TickTimer.h"
#include "Game/AI/FinalBoss/FinalBossPhase.h"
#include "FinalBossPart.h"
#include <stack>
#include "FinalBossWaypoint.h"

namespace FB
{
	class FinalBossWaypoint;
	class Behaviour;
	class FinalBoss;

	struct HandEnragedDesc : public Engine::Reflectable
	{
		VFXRef myHitGroundVFX;

		void Reflect(Engine::Reflector& aReflector) override
		{
			aReflector.Reflect(myHitGroundVFX, "Hit Ground VFX");
		}
	};

	class FinalBossHand : public FinalBossPart
	{
	public:
		FinalBossHand();
		FinalBossHand(GameObject* aGameObject);
		virtual ~FinalBossHand();

		void Start() override;

		void InitAudio();

		virtual bool IsIntroFinished() const override;

		const Vec3f& GetDefaultPos() const;

		void TargetWaypoint(FinalBossWaypoint* aWaypoint);
		void SetTargetRotation(const Quatf& aRotation, const float aRotationSpeed = -1.f);

		Engine::AudioComponent& GetAudio();

		const VFXRef& GetDeathVFX() const;
		const VFXRef& GetExplosionAftermathVFX() const;
		const VFXRef& GetDeathAfterMathVFX() const;

		const FB::HandCrawlerDesc& GetHandCrawlerDesc() const;
		const HandEnragedDesc& GetEnragedDesc() const;

		void GetEyeWorldTransform(Vec3f* aPosition, Vec3f* aScale, Quatf* aRotation);

		float GetEnragedDamageMultiplier() const;
		float GetEnragedDurationMultiplier() const;

		virtual float GetMaxAllowedDistanceAbovePortal() = 0;

	protected:
		virtual void Execute(Engine::eEngineOrder aOrder) override;

		void Reflect(Engine::Reflector& aReflector) override;

		void Enrage(const BehaviourEnumValue aEnragedEnumValue);

	private:
		void UpdateDefaultPositions();
		void MoveTowardsWaypoint();
		void UpdateRotation();

	protected:
		VFXRef myDeathVFX;
		VFXRef myExplosionAftermathVFX;
		VFXRef myDeathAfterMathVFX;

		Engine::GameObjectRef myHandDefaultPosGameObject;
		Vec3f myDefaultPos;
		Quatf myDefaultRot;
		Vec3f mySmoothDampVelocity;
		float myCurrentTime = 0.f;

		FinalBossWaypoint* myTargetWaypoint = nullptr;

		Engine::AudioComponent* myAudio = nullptr;

		bool myHasEnraged = false;

		FinalBossWaypoint myDefaultWaypoint;

		HandCrawlerDesc myCrawlerDesc;
		HandEnragedDesc myEnragedDesc;

		TickTimer myTargetWaypointCooldown;

		int myEyeBoneIndex = -1;

		// X: is the default, Y is the value when damaged
		Vec2f myEnragedGlowMinMax = Vec2f(0.2f, 0.5f);

		Quatf myCurrentRotation;
		Quatf myTargetRotation;
		float myCurrentRotationTime = 0.f;
		float myRotationSpeed = 4.f; // do not change
		float myCurrentRotationSpeed = 4.f;

		float myEnragedDamageAndCountMultiplier = 2.f;
		float myEnragedDurationMultiplier = 0.5f;
	private:
		void Failsafe();
	};
}