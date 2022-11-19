#pragma once

#include "EngageState.h"

#include "Engine/CameraShake/PerlinShake.h"

namespace Engine
{
	class Reflector;
	class VFXComponent;
}

namespace LevelBossStates
{
	class GatlingEyeState : public LBS::EngageState
	{
	public:
		GatlingEyeState() = delete;
		GatlingEyeState(LevelBoss& aBoss, GatlingEyeVars& someVariables);

		void Update() override;
		void ForceUpdate() override;

		void OnEnterState() override;
		void OnExitState() override;

		void OnEnableAttack() override;
		void OnDisableAttack() override;

	private:
		void OnIdle(const Vec3f& aPlayerPosition);
		void OnAttack(const Vec3f& aPlayerPosition);

		void Telegraph();

	private:
		GatlingEyeVars& myVars;
		GameObject* myTempObj = nullptr;

		Vec3f myLastPlayerPosition = {};
		Vec3f myEyePosition = Vec3f(0.0f, 1500.0f, 0.0f);

		GameObject* myBossOrb = nullptr;
		GameObject* myPlayerOrb = nullptr;
		Engine::VFXComponent* myChargeVFX = nullptr;
		Engine::PerlinShakeDesc myLaserShake;

		enum class AnimState
		{
			Opening,
			Closing,
			None
		} myAnimState = AnimState::None;
	};
}

