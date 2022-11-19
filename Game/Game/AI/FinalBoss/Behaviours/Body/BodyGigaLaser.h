#pragma once

#include "Game/AI/FinalBoss/Behaviours/Behaviour.h"
#include "AI/FinalBoss/States/FinalBossState.h"
#include "Engine/CameraShake/PerlinShake.h"
#include "Utils/TickTimer.h"

namespace Engine
{
	class AnimatorComponent;
	class ModelComponent;
	class VFXComponent;
	class PointLightComponent;
}

namespace FB
{
	struct GigaLaserDesc;
	class FinalBossBody;

	class BodyGigaLaser : public Behaviour
	{
	public:
		enum class State
		{
			Telegraphing,
			ShootingLaser,
			None
		};

		BodyGigaLaser(FinalBossBody& aBody);

		void Update() override;

		bool IsFinished() const override;

		void OnEnter() override;
		void OnExit() override;

		void Reflect(Engine::Reflector& aReflector) override;

	private:
		void SpawnLaser();
		void OnFinishedAnimation();
		void SpawnTelegraphLaser();
		void PointTelegraphLaserAtPlayer();
		void DestroyTelegraphLaser();
		void PointLaserAtPosition(const Vec3f& aTargetPosition);

		void StartShake();
		void DestroyLaser();

		float GetLaserDuration() const;
		float GetLaserDotWaitTime() const;
		void UpdateLaserDamageDots();
		void DoDamage();
		const GigaLaserDesc& Desc() const;
		void StopTelegraphImpactVFX();
		void StopTelegraphMouthSuckVFX();
		void StopGigaLaserImpactVFX();

	private:
		FinalBossBody& myBody;
		bool myIsFinished = false;
		int myEyeBoneIndex = -1;
		State myState = State::Telegraphing;

		// Giga laser
		GameObject* myLaser = nullptr;
		Engine::PerlinShakeDesc myLaserShake;
		float myDamageElapsed = 0.f;
		float myTotalDamage = 0.f;
		float myTotalDamageDone = 0.f;
		Weak<GameObject> myGigaLaserImpactVFXObj;
		Engine::VFXComponent* myGigaLaserImpactVFX = nullptr;

		// Telegraph laser
		GameObject* myTelegraphLaser = nullptr;
		Engine::ModelComponent* myTelegraphModel = nullptr;
		float myTelegraphElapsed = 0.f;
		Vec2f myTelegraphVars = Vec2f(-100.f, 35.f);
		Weak<GameObject> myImpactVFXObj;
		Engine::VFXComponent* myImpactVFX = nullptr;

		Weak<GameObject> myMouthSuckVFXObj;
		Engine::VFXComponent* myMouthSuckVFX = nullptr;

		Vec3f myCurrentTargetPosition;

		GameObject* myOutputFlashPointlightObj = nullptr;
		Engine::PointLightComponent* myOutputFlashPointlight = nullptr;

		GameObject* myImpactFlashPointlightObj = nullptr;
		Engine::PointLightComponent* myImpactFlashPointlight = nullptr;
	};
}