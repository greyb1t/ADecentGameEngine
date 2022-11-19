#include "pch.h"
#include "DashState.h"

#include "MovementState.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "Engine/GameObject/Components/ModelComponent.h"
#include "Engine/GameObject/Components/VFXComponent.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"
#include "Engine/VFX/VFXUtilities.h"
#include "Items/ItemManager.h"
#include "Player/Player.h"

DashState::DashState(Player* aPlayer)
	: State(aPlayer)
{

}

void DashState::Enter()
{
	myDistanceDashed = 0;

	auto& stats = GetPlayer()->GetSkillStats();
	myDashTotalTime = (stats.mobility.dashInTime + stats.mobility.dashTime + stats.mobility.dashOutTime);
	myDashSpeed = stats.mobility.dashDistance / myDashTotalTime;

	GetStatus().upperBodyWeightGoal = 0;
	GetStatus().overrideVelocity = true;
	EnterIn();
}

void DashState::Update()
{
	switch (myState)
	{
	case eDashState::DASH_IN: 
		UpdateIn();
		break;
	case eDashState::DASH_UPDATE:
		UpdateDash();
		break;
	case eDashState::DASH_OUT: 
		UpdateOut();
		break;
	default: ;
	}
}

void DashState::Exit()
{
	GetPlayer()->GetAnimatorComponent().GetController().SetBool("IsDashing", false);
	GetPlayer()->GetStatus().upperBodyWeightGoal = 1;
	GetStatus().overrideVelocity = false;

	GetPlayer()->GetModelComponent().SetActive(true);
}

void DashState::EnterIn()
{
	Main::GetItemManager().PostEvent(eItemEvent::OnDash, nullptr);
	myTimer = 0;
	myState = eDashState::DASH_IN;
	GetPlayer()->GetVelocity().Set({ 0,0,0 });

	GetPlayer()->GetAnimatorComponent().GetController().SetBool("IsDashing", true);
	GetPlayer()->GetAnimatorComponent().GetController().Trigger("Dash");

	const Vec3f input = GetPlayer()->GetInputDir();

	bool forward = false, back = false, right = false, left = false;

	if (input.LengthSqr() > 0.1f)
	{
		myDashDirection = input.z * GetTransform().Forward() + input.x * GetTransform().Right();
		myDashDirection.Normalize();

		forward = input.z > 0.5f;
		back = input.z < -0.5;

		right = input.x > 0.5 && !(forward || back);
		left = input.x < -0.5 && !(forward || back);
	} else
	{
		myDashDirection = GetPlayer()->GetTransform().Forward();
		forward = true;
	}
	myDistanceDashed = 0;

	if (GetPlayer()->GetStatus().isAiming) 
	{
		GetPlayer()->GetAnimatorComponent().GetController().SetBool("Forward", forward);
		GetPlayer()->GetAnimatorComponent().GetController().SetBool("Backward", back);
		GetPlayer()->GetAnimatorComponent().GetController().SetBool("Right", right);
		GetPlayer()->GetAnimatorComponent().GetController().SetBool("Left", left);
	} else 
	{
		GetPlayer()->GetAnimatorComponent().GetController().SetBool("Forward", true);
		GetPlayer()->GetAnimatorComponent().GetController().SetBool("Backward", false);
		GetPlayer()->GetAnimatorComponent().GetController().SetBool("Right", false);
		GetPlayer()->GetAnimatorComponent().GetController().SetBool("Left", false);
	}

	auto* vfxObj = GetPlayer()->GetGameObject()->GetScene()->AddGameObject<GameObject>();
	vfxObj->GetTransform().SetPosition(GetPlayer()->GetTransform().GetPosition());

	myVFXObject = GetGameObject().GetScene()->AddGameObject<GameObject>();

	if (GetPlayer()->GetVFXHolder().dashOutVFX->IsValid())
	{
		myVFXObject->GetTransform().SetPosition(GetGameObject().GetTransform().GetPosition());
		myVFXObject->GetTransform().LookAt(myVFXObject->GetTransform().GetPosition() + myDashDirection * 100.0f);
		auto* vfx = myVFXObject->AddComponent<Engine::VFXComponent>(GetPlayer()->GetVFXHolder().dashOutVFX->Get());
		vfx->Play();
		vfx->AutoDestroy();
	}

	if (GetPlayer()->GetVFXHolder().dashVFX->IsValid())
	{
		myDashVFX = GetGameObject().AddComponent<Engine::VFXComponent>(GetPlayer()->GetVFXHolder().dashVFX->Get());
		myDashVFX->Play();
	}
}

void DashState::UpdateIn()
{
	myTimer += Time::DeltaTime;
	UpdateMovement();
	if (myTimer >= GetPlayer()->GetSkillStats().mobility.dashInTime)
	{
		EnterUpdateDash();
	}
}

void DashState::EnterUpdateDash()
{
	myTimer = 0;
	myState = eDashState::DASH_UPDATE;

	auto* vfxObj = GetPlayer()->GetGameObject()->GetScene()->AddGameObject<GameObject>();
	vfxObj->GetTransform().SetParent(&GetPlayer()->GetTransform());
	vfxObj->GetTransform().SetPositionLocal({0,0,80});

	//if (myDashOutVFX->IsValid())
	//{
	//	auto* vfx = GetGameObject().AddComponent<Engine::VFXComponent>(myDashOutVFX->Get());
	//	vfx->Play();
	//	//vfx->AutoDestroy();
	//}

	GetPlayer()->GetModelComponent().SetActive(false);
}

void DashState::UpdateDash()
{
	auto& stats = GetPlayer()->GetSkillStats();

	UpdateMovement();

	if (myDistanceDashed >= stats.mobility.dashDistance - myDashSpeed * stats.mobility.dashOutTime)
	{
		EnterOut();
	}
}

void DashState::EnterOut()
{
	myTimer = 0;
	myState = eDashState::DASH_OUT;
	GetPlayer()->GetModelComponent().SetActive(true);
	GetPlayer()->GetAnimatorComponent().GetController().SetBool("IsDashing", false);

	auto* vfxObj = GetPlayer()->GetGameObject()->GetScene()->AddGameObject<GameObject>();
	vfxObj->GetTransform().SetPosition(GetPlayer()->GetTransform().GetPosition());

	//if (myDashVFX) 
	//{
	//	myDashVFX->Stop();
	//}

	if (GetPlayer()->GetVFXHolder().dashOutVFX->IsValid())
	{
		myVFXObject->GetTransform().SetPosition(GetGameObject().GetTransform().GetPosition());
		myVFXObject->GetTransform().LookAt(myVFXObject->GetTransform().GetPosition() + myDashDirection * 100.0f);
		auto* vfx = myVFXObject->AddComponent<Engine::VFXComponent>(GetPlayer()->GetVFXHolder().dashOutVFX->Get());
		vfx->Play();
		vfx->AutoDestroy();
	}
}

void DashState::UpdateOut()
{
	myTimer += Time::DeltaTime;

	UpdateMovement();

	if (myTimer >= GetPlayer()->GetSkillStats().mobility.dashInTime)
	{
		ExitDash();
	}
}

void DashState::ExitDash()
{
	GetPlayer()->GetStateMachine().SetState(GetPlayer()->GetMovementState());
	GetStatus().isSprinting = true;
}

void DashState::UpdateMovement()
{
	auto& stats = GetPlayer()->GetSkillStats();
	float distance = (stats.mobility.dashDistance / stats.mobility.dashTime) * Time::DeltaTime;

	if (myDistanceDashed + distance > stats.mobility.dashDistance)
	{
		distance = stats.mobility.dashDistance - myDistanceDashed;
	}

	myDistanceDashed += distance;

	// TODO: Better fix plz
	GetPlayer()->GetVelocity().Set({ 0,0,0 });
	Move(distance);
}

void DashState::Move(float aDistance)
{
	GetPlayer()->GetTransform().Move(myDashDirection * aDistance);
}
