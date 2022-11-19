#include "pch.h"
#include "MovementState.h"

#include "Camera/CameraController.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "Engine/GameObject/Components/AudioComponent.h"
#include "Engine/GameObject/Components/ModelComponent.h"
#include "Player/Player.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"
#include "Engine/GameObject/Components/VFXComponent.h"

MovementState::MovementState(Player* aPlayer)
	: ShootState(aPlayer)
{
	myModelTransform = &aPlayer->GetGameObject()->FindComponent<Engine::ModelComponent>()->GetTransform();
	myRotator.Init(aPlayer);

	// Create Sprint Dust
	auto dustVFX = GetPlayer()->GetVFXHolder().sprintDust.normalDust;
	if (dustVFX && dustVFX->IsValid())
	{
		auto* dustObj = GetPlayer()->GetGameObject()->GetScene()->AddGameObject<GameObject>();
		dustObj->SetName("Sprint Dust VFX");
		dustObj->GetTransform().SetParent(&GetPlayer()->GetTransform());
		dustObj->GetTransform().SetPositionLocal({0,-GetPlayer()->GetHeight() * .5f,0});
		mySprintVFX = dustObj->AddComponent<Engine::VFXComponent>(GetPlayer()->GetVFXHolder().sprintDust.normalDust->Get());
	}
}

void MovementState::Enter()
{
	ShootState::Enter();
}

void MovementState::Exit()
{
	ShootState::Exit();
	GetPlayer()->GetStatus().movementDirection = {0,0,0};

	if (mySprintVFX)
		mySprintVFX->Stop();
}

void MovementState::Update()
{
	if (GetPlayer()->GetCameraController().GetIfControllingCamera())
	{
		UpdateRotation();
		UpdateMovement();
		ShootState::Update();
	}
}

void MovementState::UpdateMovement()
{
	const Vec3f input = GetPlayer()->GetInputDir();
	
	if (GetEngine().GetInputManager().IsKeyDown(Common::KeyCode::LeftControl))
		ToggleSprint();

	Vec3f dir = input.z * GetTransform().Forward() + input.x * GetTransform().Right();
	dir.Normalize();

	if (input.z <= 0)
		CancelSprint();

	myMovementVelocity = C::Lerp(myMovementVelocity, dir * GetMoveSpeed(), GetStats().GetMovementLag() * Time::DeltaTime);
	GetTransform().Move(myMovementVelocity * Time::DeltaTime);
	GetPlayer()->GetStatus().movementDirection = myMovementVelocity;

	// TODO: FIX WITH BETTER SOLUTION
	// Check with several raycasts & dynamically with collision height
	if (myMovementVelocity.LengthSqr() > 0 && GetPlayer()->IsGrounded() && GetPlayer()->GetVelocity().Get().y <= 1)
	{

		const float hheight = GetPlayer()->GetHeight() * .5f;
	
		RayCastHit firstHit;
		RayCastHit hit;
		if (GetGameObject().GetScene()->RayCast(GetTransform().GetPosition(), Vec3f(0, -1, 0), hheight + GetPlayer()->GetRadius() * 1.2f, eLayer::GROUND | eLayer::DEFAULT, firstHit))
		{
			if (GetGameObject().GetScene()->RayCast(GetTransform().GetPosition() + GetTransform().GetMovement(), Vec3f(0, -1, 0), 1000, eLayer::GROUND | eLayer::DEFAULT, hit))
			{
				//float strength = std::min(GetMoveSpeed(),	) / 100.f;
				const float movementDist = myMovementVelocity.Length() * Time::DeltaTime;
				const float deltaY = hit.Position.y - (firstHit.Position.y);

				const float angle = abs(atan2f(deltaY, movementDist)) * Math::Rad2Deg;
				//LOG_INFO(LogType::Game) << "Angle: " << angle << " -- Delta Y: " << deltaY;

				if (angle <= GetPlayer()->GetGroundingAngle())
				{
					if (deltaY > 5) {
						int a = 0;
					}
					GetTransform().Move({ 0, deltaY,0 });
				}
			}
		}
	}



	if (GetPlayer()->IsGrounded())
		GetPlayer()->GetStats().SetAmountJumped(1);

 	if ((GetPlayer()->IsGrounded() || (GetPlayer()->GetStats().GetJumpAmount() > GetPlayer()->GetStats().GetAmountJumped())) &&
		GetEngine().GetInputManager().IsKeyDown(Common::KeyCode::Space))
	{
		if (GetPlayer()->IsGrounded())
			GetPlayer()->GetStats().SetAmountJumped(0);

 		Jump();
	}

	const CU::Vector3f modelDir = GetPlayer()->GetModelObject().GetTransform().Forward();
	const CU::Vector2f modelDir2 = CU::Vector2f{ modelDir.x, modelDir.z }.GetNormalized();
	const CU::Vector2f moveDir2 = CU::Vector2f{ myMovementVelocity.x, myMovementVelocity.z }.GetNormalized();

	const float delta = atan2(modelDir2.y, modelDir2.x);
	const float animationAngle = atan2(moveDir2.y, moveDir2.x) - delta + CU::PI * 1.5f;

	CU::Vector2f animationDirection = { cos(animationAngle), sin(animationAngle) };
	animationDirection.Normalize(); 

	float movementScale = myMovementVelocity.Length() / GetMoveSpeed();
	animationDirection *= movementScale;

	// TODO: Proper velocity speed on movement
	GetPlayer()->GetAnimatorComponent().GetController().SetVec2("PL_Movement", Vec2f(animationDirection.x * 0.8f, animationDirection.y * (GetStatus().isSprinting ? 1 : .8f)));
	GetPlayer()->GetAnimatorComponent().GetController().SetBool("IsSprinting", GetStatus().isSprinting);

	GetStatus().isMoving = input.LengthSqr() > 0.1f;
	
	bool isCombatIdle = ShootState::GetState() == ShootState::eCombatState::COMBAT_IDLE;
	if ((!GetStatus().isMoving && isCombatIdle) || GetStatus().isSprinting || (!GetPlayer()->IsGrounded() && isCombatIdle))
	{
		GetStatus().upperBodyWeightGoal = 0;
	}
	else 
	{
		GetStatus().upperBodyWeightGoal = 1;
	}

	GetPlayer()->GetCameraController().SetIsRunning(GetStatus().isSprinting);

	if (mySprintVFX) 
	{
		if ((GetStatus().isSprinting && GetPlayer()->IsGrounded()) != mySprintVFX->IsPlaying())
		{
			if (GetStatus().isSprinting && GetPlayer()->IsGrounded()) {
				mySprintVFX->Play();
			}
			else {
				mySprintVFX->Stop();
			}
		}
	}
}

void MovementState::UpdateRotation()
{
	bool shouldReturn = true;
	auto& input = GetEngine().GetInputManager();

	if (GetPlayer()->GetInputDir().LengthSqr() != 0.0f)
		shouldReturn = false;

	//if (input.IsMouseKeyPressed(C::MouseButton::Left) || input.IsMouseKeyPressed(C::MouseButton::Right))
	//{
	//	shouldReturn = false;
	//}

	//if (input.IsKey(C::KeyCode::R) || input.IsKey(C::KeyCode::LeftShift)) //Simon TODO: check if abilities are active instead of inputs
	//	shouldReturn = false;

	if (GetStatus().isAiming)
		shouldReturn = false;

	if (shouldReturn)
		return;

	
	myRotator.Update();
}

void MovementState::Jump()
{
	if (!GetPlayer()->IsGrounded()) 
	{
		auto airJumpVFX = GetPlayer()->GetVFXHolder().airJump;
		if (airJumpVFX && airJumpVFX->IsValid())
		{
			auto obj = GetPlayer()->GetGameObject()->GetScene()->AddGameObject<GameObject>();
			obj->GetTransform().SetPosition(GetPlayer()->GetModelObject().GetTransform().GetPosition());
			obj->SetName("Jump_VFX");

			auto vfx = obj->AddComponent<Engine::VFXComponent>(airJumpVFX->Get());
			vfx->Play();
			vfx->AutoDestroy();
		}
	}

	const auto& vel = GetPlayer()->GetVelocity().Get();
	GetPlayer()->GetVelocity().SetY(GetPlayer()->GetStats().GetJumpForce());

	GetPlayer()->GetAnimatorComponent().GetController().Trigger("Jump");

	if(GetPlayer()->GetStats().GetAmountJumped() <= 0)
	{
		GetPlayer()->GetAudioComponent().PlayEvent("Jump");
	}
	else
	{
		GetPlayer()->GetAudioComponent().PlayEvent("ExtraJump");
	}

 	GetStats().SetAmountJumped(GetPlayer()->GetStats().GetAmountJumped() + 1.0f);

}

void MovementState::ToggleSprint()
{
	if (GetStatus().isSprinting)
	{
		CancelSprint();
	}
	else 
	{
		Sprint();
	}
}

void MovementState::Sprint()
{
	GetStatus().isSprinting = true;
}

void MovementState::CancelSprint()
{
	GetStatus().isSprinting = false;
}

float MovementState::GetMoveSpeed() const
{
	float speed = GetStats().GetMovementSpeed();

	if (GetStatus().isSprinting)
		speed *= GetStats().GetSprintMultiplier();

	return speed;
}
