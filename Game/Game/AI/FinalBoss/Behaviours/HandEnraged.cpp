#include "pch.h"
#include "HandEnraged.h"
#include "AI/FinalBoss/Constants.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "AI/FinalBoss/FinalBossHand.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/VFXComponent.h"
#include "Engine/GameObject/Components/CameraShakeComponent.h"
#include "Components/HealthComponent.h"

FB::HandEnraged::HandEnraged(FinalBossHand& aHand, Engine::AnimatorComponent& aAnimator)
	: myAnimator(aAnimator),
	myHand(aHand)
{
	myAnimator.GetController().AddStateOnExitCallback("Base", "Enraged", [this]() { myIsFinished = true; });

	myAnimator.GetController().AddEventCallback("CameraShake",
		[this]()
		{
			auto mainCam = myHand.GetGameObject()->GetScene()->GetMainCameraGameObject();
			auto shakeComponent = mainCam->GetComponent<Engine::CameraShakeComponent>();
			if (shakeComponent)
			{
				shakeComponent->AddPerlinShake("HandSlamHitGround");
			}

			const auto& desc = myHand.GetEnragedDesc();

			if (desc.myHitGroundVFX && desc.myHitGroundVFX->IsValid())
			{
				auto g = myHand.GetGameObject()->GetScene()->AddGameObject<GameObject>();
				g->GetTransform().SetPosition(myHitGroundPos);
				auto myVFX = g->AddComponent<Engine::VFXComponent>(desc.myHitGroundVFX->Get());
				myVFX->Play();
				myVFX->AutoDestroy();
			}
			else
			{
				LOG_ERROR(LogType::Game) << "Missing hit ground VFX";
			}
		});
}

void FB::HandEnraged::Update()
{
	switch (myState)
	{
	case State::IsMovingToYLevel:
	{
		myMoveToYLevelTimer.Tick(Time::DeltaTime);

		const float t = myMoveToYLevelTimer.Percent();

		auto p = Math::Lerp(myStartPos, myTargetPos, t);

		myHand.GetTransform().SetPosition(p);

		auto rot = myHand.GetTransform().GetRotation().EulerAngles();
		rot.x = 0.f;
		rot.z = 0.f;
		myHand.SetTargetRotation(rot);

		if (myMoveToYLevelTimer.IsFinished())
		{
			myState = State::BangGround;

			myAnimator.GetController().Trigger(Constants::EnragedTriggerName);
		}
	} break;

	case State::BangGround:
	{

	} break;

	default:
		break;
	}
}

bool FB::HandEnraged::IsFinished() const
{
	return myIsFinished;
}

void FB::HandEnraged::OnEnter()
{
	myIsFinished = false;
	myState = State::IsMovingToYLevel;
	myMoveToYLevelTimer = TickTimer::FromSeconds(0.5f);

	myHand.GetHealth().SetIsInvincible(true);

	const auto up = Vec3f(0.f, -1.f, 0.f);

	myStartPos = myHand.GetTransform().GetPosition();

	RayCastHit hit;

	if (myHand.GetGameObject()->GetScene()->RayCast(
		myStartPos,
		up,
		Constants::RaycastDistance,
		eLayer::GROUND | eLayer::DEFAULT,
		hit))
	{
		const float targetYAboveGround = 900.f;
		myTargetPos = hit.Position + Vec3f(0.f, 1.f, 0.f) * targetYAboveGround;

		myHitGroundPos = hit.Position;
	}
	else
	{
		myTargetPos = myStartPos;
		LOG_ERROR(LogType::Game) << "Enraged raycast ground failed";
	}
}

void FB::HandEnraged::OnExit()
{
	myHand.GetHealth().SetIsInvincible(false);
}

void FB::HandEnraged::Reflect(Engine::Reflector& aReflector)
{
}

bool FB::HandEnraged::IsAllowedToMoveExternally() const
{
	return false;
}
