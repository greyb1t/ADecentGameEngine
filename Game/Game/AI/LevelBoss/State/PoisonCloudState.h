#pragma once

#include "EngageState.h"

namespace LevelBossStates
{
	struct Cloud
	{
		float mySpawnTime = 0.0f;

		float myProgress = 0.0f;
		float myDuration = 0.0f;

		float myTickProgress = 0.0f;
		float myTickDuration = 0.0f;

		Vec3f myPosition = {};

		Engine::VFXComponent* myVFX = nullptr;
	};

	class PoisonCloudState : public LBS::EngageState
	{
	public:
		PoisonCloudState() = delete;
		PoisonCloudState(LevelBoss& aBoss, PoisonCloudVars& someVariables);

		void OnEnterState() override;
		void OnExitState() override;

		void OnEnableAttack() override;
		void OnDisableAttack() override;

		void Update() override;
		void ForceUpdate() override;

	private:
		void OnUpdate(const Vec3f& aPlayerPosition);

		void CreateCloud();

	private:
		PoisonCloudVars& myVars;
		std::vector<Cloud> myPoisonClouds;
		Vec3f myShootPosition{};

		enum class AnimState
		{
			Opening,
			Closing,
			None
		} myAnimState = AnimState::None;


		float myProgress = 0.0f;
		float myDuration = 10.0f;
	};
}
