#include "pch.h"
#include "HealHandler.h"
#include "Engine/GameObject/GameObject.h"
#include <Engine/GameObject/Components/VFXComponent.h>
#include "Engine/ResourceManagement/Resources/VFXResource.h"
#include <Player/Player.h>

void HealHandler::Init(Player* aPlayer)
{
	myPlayer = aPlayer;
	if (!myPlayer->GetVFXHolder().heal || !myPlayer->GetVFXHolder().heal->IsValid())
		return;

	auto* obj = myPlayer->GetGameObject()->GetScene()->AddGameObject<GameObject>();
	obj->GetTransform().SetParent(&myPlayer->GetTransform());
	obj->GetTransform().SetPositionLocal({ 0, 0, 0 });
	obj->SetName("Healing VFX");

	myVFX = obj->AddComponent<Engine::VFXComponent>(myPlayer->GetVFXHolder().heal->Get());
	myVFX->Play();
}

void HealHandler::Update()
{
	myHealActiveTimer -= Time::DeltaTime;
	if (myHealActiveTimer <= 0) 
	{
		Stop();
	}
}

void HealHandler::Heal()
{
	myHealActiveTimer = myHealingTime;
	Play();
}

void HealHandler::Play()
{
	if (!myVFX)
		return;

	if (!myVFX->IsPlaying())
	{
		myVFX->Play();
	}
}

void HealHandler::Stop()
{
	if (!myVFX)
		return;

	if (myVFX->IsPlaying())
	{
		myVFX->Stop();
	}
}

float& HealHandler::HealTimeRef()
{
	return myHealingTime;
}
