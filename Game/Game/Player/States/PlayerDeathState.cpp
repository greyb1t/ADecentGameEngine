#include "pch.h"
#include "PlayerDeathState.h"
#include "Engine/GameObject/GameObject.h"
#include "Player/Player.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"
#include "Engine/GameObject/Components/VFXComponent.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "Engine/GameObject/Components/ModelComponent.h"
#include "HUD\HUDHandler.h"

PlayerDeathState::PlayerDeathState(Player* aPlayer)
	: State(aPlayer)
{

}

void PlayerDeathState::Enter()
{
	GetPlayer()->GetAnimatorComponent().GetController().SetBool("IsDead", true);
	myIsVisible = true;
	GetPlayer()->GetHUDHandler()->ActivateDeathScreen();

	auto deathVfxRef = GetPlayer()->GetVFXHolder().death;
	if (deathVfxRef && deathVfxRef->IsValid())
	{
		auto obj = GetPlayer()->GetGameObject()->GetScene()->AddGameObject<GameObject>();
		obj->GetTransform().SetParent(&GetGameObject().GetTransform());
		obj->GetTransform().SetPositionLocal({ 0,0,0 });
		auto vfx = obj->AddComponent<Engine::VFXComponent>(deathVfxRef->Get());
		vfx->Play();
		vfx->AutoDestroy();
	}
}

void PlayerDeathState::Update()
{
	if (myIsVisible) 
	{
		myInvisibilityTimer += Time::DeltaTime;
		if (myInvisibilityTimer > GetPlayer()->GetSettings().death.invisibleDelay) 
		{
			myIsVisible = false;
			GetPlayer()->GetModelComponent().SetActive(myIsVisible);
		}
	}
}

void PlayerDeathState::Exit()
{
	GetPlayer()->GetAnimatorComponent().GetController().SetBool("IsDead", false);
	myIsVisible = true;
	GetPlayer()->GetModelComponent().SetActive(myIsVisible);
}
