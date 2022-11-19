#pragma once

#include "Game/AI/FinalBoss/Behaviours/Behaviour.h"
#include "AI/FinalBoss/States/FinalBossState.h"

namespace Engine
{
	class AnimatorComponent;
	class VFXComponent;
}

namespace FB
{
	class FinalBossBody;
	struct PoisonCloudDesc;

	class BodyPoisonBreath : public Behaviour
	{
	public:
		BodyPoisonBreath(FinalBossBody& aBody);

		void Update() override;

		bool IsFinished() const override;

		void OnEnter() override;
		void OnExit() override;

		void Reflect(Engine::Reflector& aReflector) override;

	private:
		void AttackUpdate();
		void SpawnCloud();

		void PoisonBreathSound(const bool aEnable) const;

	private:
		FinalBossBody& myBody;

		bool myIsFinished = false;

		float myStartDelayProgress = 0.0f;
		float myDmgTickProgress = 0.0f;
		float myCloudSpawnProgress = 0.0f;
		bool myShouldDealDamage = false;

		GameObject* myVFXObject = nullptr;
		Engine::VFXComponent* myVFXComponent = nullptr;

		inline const PoisonCloudDesc& Desc() const;

		struct CloudSpawnInfo
		{
			Vec3f myPos;
			float myDuration;
			bool myHasSpawnedCloud = false;
		};
		std::vector<CloudSpawnInfo> myCloudPositions;
		void CreatePoisonVFX();
	};
}