#pragma once

#include "AI/FinalBoss/States/FinalBossState.h"
#include "Utils/TickTimer.h"

namespace Engine
{
	class GameObjectPrefab;
	class PointLightComponent;
}

namespace FB
{
	class HandGattlingEye;
	struct GattlingEyeDesc;

	class HandGatlingEye_Loop : public BaseFinalBossState
	{
	public:
		HandGatlingEye_Loop(HandGattlingEye& aParent);

		void Update() override;

		bool IsFinished() override;

		void OnEnter() override;
		void OnExit() override;

	private:
		void UpdateTelegraphOrb(const Vec3f& aDirection);
		Vec3f GetLaserOutputPosition() const;
		void GatlingLaserSound(const bool aEnable) const;

	private:
		HandGattlingEye& myEye;
		const GattlingEyeDesc& myDesc;

		TickTimer myTimer;

		float myRapidFireTimer = 0.f;

		Vec3f myPlayerLastPos;
		Vec3f myTargetPos;

		Engine::GameObjectPrefab* myTelegraphOrbPrefab = nullptr;
		GameObject* myTelegraphOrb = nullptr;

		GameObject* myFlashPointlightObj = nullptr;
		Engine::PointLightComponent* myFlashPointlight = nullptr;
	};
}