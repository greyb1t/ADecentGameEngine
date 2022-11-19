#pragma once

#include "../LevelBoss.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "Engine/GameObject/Components/AudioComponent.h"

namespace LevelBossStates
{
	class BaseState
	{
	public:
		BaseState() = delete;
		BaseState(LevelBoss& aBoss);
		virtual ~BaseState();

		virtual void OnEnterState() {};
		virtual void OnExitState() {};

		virtual void Update() {};
		virtual void ForceUpdate() {};

	protected:
		Engine::AudioComponent& GetAudio();
		Engine::AnimationController& GetController();
		Transform& GetTransform();

	protected:
		LevelBoss& myBoss;

	private:
		Vec3f mySmoothDampVelocity{};

	};
}

namespace LBS = LevelBossStates;