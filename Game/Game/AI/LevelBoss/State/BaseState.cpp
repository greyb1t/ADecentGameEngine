#include "pch.h"
#include "BaseState.h"

namespace LevelBossStates
{
	BaseState::BaseState(LevelBoss& aBoss) : myBoss(aBoss)
	{
	}

	BaseState::~BaseState()
	{
	}

	Engine::AudioComponent& BaseState::GetAudio()
	{
		return *myBoss.GetAudioSource();
	}

	Engine::AnimationController& BaseState::GetController()
	{
		return myBoss.GetAnimator()->GetController();
	}

	Transform& BaseState::GetTransform()
	{
		return myBoss.GetTransform();
	}
}