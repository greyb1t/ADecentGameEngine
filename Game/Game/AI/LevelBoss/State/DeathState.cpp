#include "pch.h"
#include "DeathState.h"

#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"

LevelBossStates::DeathState::DeathState(LevelBoss& aBoss, SpikeAttackVars& someVariables) 
	: BaseState(aBoss), myVars(someVariables)
{
	const int size = myVars.myBossSpikes.size();
	for (int i = 0; i < size; i++)
	{
		if (auto animator = myVars.myBossSpikes[i].Get()->GetComponent<Engine::AnimatorComponent>())
		{
			myAnimators.push_back(animator);
		}
	}

	GetController().AddEventCallback("UnlockPortal", [this]() 
		{ 
			myBoss.SetCanEnterNextLevel(true); 
			myBoss.SetPortalTransition(LevelBoss::PortalTransition::Cleansed_ToBlue);
			myEventWasCalled = true;
		}
	);

	GetController().AddStateOnExitCallback("BASE - (Main)", "Death", [this]() {
		if (!myEventWasCalled)
		{
			myBoss.SetCanEnterNextLevel(true);
			myBoss.SetPortalTransition(LevelBoss::PortalTransition::Cleansed_ToBlue);
			myEventWasCalled = true;
		}
		});

	myProgress = myDuration;
}

void LevelBossStates::DeathState::OnEnterState()
{
	myBoss.SetShowEye(true);
	
	GetController().Trigger("Death");
	GetController().SetLayerWeight("Gatling Is Disabled - (Override)", 0.0f);
	GetController().SetLayerWeight("Mortar + Poison Is Disabled - (Override)", 0.0f);

	for (int i = 0; i < myAnimators.size(); i++)
	{
		auto& controller = myAnimators[i]->GetController();
		controller.Trigger("Death");
	}

	GetAudio().PlayEvent("Death");

	myBoss.GetGameObject()->SetLayer(eLayer::NONE);
}

void LevelBossStates::DeathState::OnExitState()
{
}

void LevelBossStates::DeathState::Update()
{
	myProgress -= Time::DeltaTime;
	if (myProgress <= 0.0f)
	{
		myBoss.SetCanEnterNextLevel(true);
		myBoss.SetPortalTransition(LevelBoss::PortalTransition::Cleansed_ToBlue);
		myEventWasCalled = true;
	}
}
