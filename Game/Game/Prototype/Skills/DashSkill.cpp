#include "pch.h"
#include "DashSkill.h"

#include "Engine/Shortcuts.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/CharacterControllerComponent.h"
#include "Engine/GameObject/Components/RigidBodyComponent.h"
#include "HUD\HUDHandler.h"
#include "Player/Player.h"

#include "Items/ItemManager.h"

DashSkill::DashSkill(Player* aPlayer)
	: Skill(aPlayer)
{
	myCooldown = 3.f;
}

void DashSkill::Activate()
{
	//StartDash();
	if (myOnActivate)
		myOnActivate();

	myPlayer->GetHUDHandler()->StartCooldown(HUDHandler::AbilityType::Mobility);

	Skill::Activate();
}

void DashSkill::Update()
{
	if (myIsDashing)
		DashUpdate();

	Skill::Update();
}

void DashSkill::Observe(const std::function<void()>& aObserver)
{
	myOnActivate = aObserver;
}

void DashSkill::StartDash()
{
	Main::GetItemManager().PostEvent(eItemEvent::OnDash, nullptr);
	myIsDashing = true;

	// FIX THIS
	//myPlayer->ToggleOverrideMovement(true);

	bool dirSet = false;
	Vec3f dir;
	auto& input = GetEngine().GetInputManager();
	if (input.IsKeyPressed(Common::KeyCode::W))
	{
		dir += myPlayer->GetTransform().Forward();
		dirSet = true;
	} else if (input.IsKeyPressed(Common::KeyCode::S))
	{
		dir -= myPlayer->GetTransform().Forward();
		dirSet = true;
	}

	if (input.IsKeyPressed(Common::KeyCode::D))
	{
		dir += myPlayer->GetTransform().Right();
		dirSet = true;
	} else if (input.IsKeyPressed(Common::KeyCode::A))
	{
		dir -= myPlayer->GetTransform().Right();
		dirSet = true;
	}

	if (dirSet)
	{
		myDashDirection = dir.GetNormalized();
	} else
	{
		myDashDirection = myPlayer->GetTransform().Forward();
	}

	myDistanceDashed = 0;
	LOG_INFO(LogType::Viktor) << "Start dash";
}

void DashSkill::DashUpdate()
{
	float distance = (myDashDistance / myDashTime) * Time::DeltaTime;

	if (myDistanceDashed + distance > myDashDistance)
	{
		distance = myDashDistance - myDistanceDashed;
	}

	myDistanceDashed += distance;

	const Vec3f movement = myDashDirection * distance;

	myPlayer->GetTransform().Move(movement);
	//myPlayer->GetRigidBody()->SetVelocity({ 0,0,0 });
	myPlayer->GetController().SetVelocity({ 0,0,0 });

	GDebugDrawer->DrawLine3D(DebugDrawFlags::Always, myPlayer->GetTransform().GetPosition(), myPlayer->GetTransform().GetPosition() + movement, .3f, Vec4f(0, 0, 1.f, .7f));

	if (myDistanceDashed >= myDashDistance)
		EndDash();
}

void DashSkill::EndDash()
{
	myIsDashing = false;
	//myPlayer->ToggleOverrideMovement(false);
	LOG_INFO(LogType::Viktor) << "End dash";
}