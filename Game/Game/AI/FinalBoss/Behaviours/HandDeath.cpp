#include "pch.h"
#include "HandDeath.h"
#include "AI/FinalBoss/Constants.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "AI/FinalBoss/FinalBossHand.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/VFXComponent.h"

FB::HandDeath::HandDeath(FinalBossHand& aHand, Engine::AnimatorComponent& aAnimator)
	: myAnimator(aAnimator),
	myHand(aHand)
{
	myAnimator.GetController().AddStateOnExitCallback("Base", "Death",
		[this]()
		{
			DestroyHand();
		});
}

void FB::HandDeath::Update()
{
}

bool FB::HandDeath::IsFinished() const
{
	return false;
}

void FB::HandDeath::OnEnter()
{
	myAnimator.GetController().Trigger(Constants::DeathTriggerName);
}

void FB::HandDeath::OnExit()
{
}

void FB::HandDeath::Reflect(Engine::Reflector& aReflector)
{
}

void FB::HandDeath::SpawnVFX()
{
	if (myHand.GetDeathVFX() && myHand.GetDeathVFX()->IsValid())
	{
		auto g = myHand.GetGameObject()->GetScene()->AddGameObject<GameObject>();
		g->GetTransform().SetPosition(myHand.GetTransform().GetPosition());
		auto myVFX = g->AddComponent<Engine::VFXComponent>(myHand.GetDeathVFX()->Get());
		myVFX->Play();
		myVFX->AutoDestroy();
	}
	else
	{
		LOG_ERROR(LogType::Game) << "Missing VFX";
	}

	if (myHand.GetExplosionAftermathVFX() && myHand.GetExplosionAftermathVFX()->IsValid())
	{
		auto g = myHand.GetGameObject()->GetScene()->AddGameObject<GameObject>();
		g->GetTransform().SetPosition(myHand.GetTransform().GetPosition());
		auto myVFX = g->AddComponent<Engine::VFXComponent>(myHand.GetExplosionAftermathVFX()->Get());
		myVFX->Play();
		myVFX->AutoDestroy();
	}
	else
	{
		LOG_ERROR(LogType::Game) << "Missing VFX";
	}
}

void FB::HandDeath::DestroyHand()
{
	myHand.GetGameObject()->SetActive(false);

	SpawnVFX();
}
