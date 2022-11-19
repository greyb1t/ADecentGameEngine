#include "pch.h"
#include "DisengageState.h"

#include "Components/HealthComponent.h"

#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/VFXComponent.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"

LevelBossStates::DisengageState::DisengageState(LevelBoss& aBoss, DisengageVars& someVariables) : EngageState(aBoss), myVars(someVariables)
{
	if (myVars.myHealingVFXRef && myVars.myHealingVFXRef->IsValid())
	{
		auto& transform = GetTransform();

		if (auto go_parent = myBoss.GetGameObject()->GetScene()->AddGameObject<GameObject>())
		{
			go_parent->SetName("Boss Healing VFX");
			
			go_parent->GetTransform().SetRotation(Quatf(Vec3f(-CU::HALF_PI, -CU::HALF_PI, 0)));
			go_parent->GetTransform().SetPosition(transform.GetPosition());

			if (auto go = myBoss.GetGameObject()->GetScene()->AddGameObject<GameObject>())
			{
				go->GetTransform().SetParent(&go_parent->GetTransform());
				go->GetTransform().SetPositionLocal(Vec3f(0, 0, 0));
				myHealingVFX = go->AddComponent<Engine::VFXComponent>(myVars.myHealingVFXRef->Get());
			}
		}
	}
}

void LevelBossStates::DisengageState::Update()
{
	auto pollingStation = GetPollingStation();
	TurnTowards(pollingStation->GetPlayerPos());

	myVars.myWaitUntilHealProgress -= Time::DeltaTime;
	if (myVars.myWaitUntilHealProgress <= 0.0f)
	{
		OnRapidHealing();
	}
}

void LevelBossStates::DisengageState::OnEnterState()
{
	myVars.myHealingProgress = myVars.myHealingDuration;
	myVars.myWaitUntilHealProgress = myVars.myWaitUntilHealDuration;

	if (myHealingVFX)
	{
		myHealingVFX->Play();
	}
}

void LevelBossStates::DisengageState::OnExitState()
{
	if (myHealingVFX)
	{
		myHealingVFX->Stop();
	}
}

void LevelBossStates::DisengageState::OnRapidHealing()
{
	myVars.myHealingProgress -= Time::DeltaTime;
	if (myVars.myHealingProgress <= 0.0f)
	{
		myBoss.GetHealth()->ApplyHealing(myVars.myHealingAmountPerTick);
		myVars.myHealingProgress = myVars.myHealingDuration;
	}
}
