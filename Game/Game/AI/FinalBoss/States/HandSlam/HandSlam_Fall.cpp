#include "pch.h"
#include "HandSlam_Fall.h"
#include "AI/FinalBoss/Behaviours/HandSlam.h"
#include "AI/FinalBoss/LeftPunchHand.h"
#include "Engine/GameObject/GameObject.h"
#include "AI/PollingStation/PollingStationComponent.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "AI/FinalBoss/Constants.h"
#include "Engine/ResourceManagement/Resources/AnimationCurveResource.h"
#include "Engine/AnimationCurve/Curve.h"
#include "Engine/GameObject/Components/CameraShakeComponent.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"
#include "Engine/GameObject/Components/VFXComponent.h"
#include "Engine/GameObject/Components/RigidBodyComponent.h"
#include "Components/HealthComponent.h"
#include "AI/FinalBoss/FinalBoss.h"

FB::HandSlam_Fall::HandSlam_Fall(HandSlam& aParent)
	: myHandSlam(aParent)
{
	/*
	if (aParent.GetLeftHand().GetHandSlamDesc().mySlamSpeedCurve &&
		aParent.GetLeftHand().GetHandSlamDesc().mySlamSpeedCurve->IsValid())
	{
		mySpeedCurve = &aParent.GetLeftHand().GetHandSlamDesc().mySlamSpeedCurve->Get();
	}
	else
	{
		LOG_ERROR(LogType::Game) << "Hand slam is missing slam speed curve";
	}
	*/

	myHandSlam.GetLeftHand().GetAnimator().GetController().AddStateOnExitCallback("Base", "Slam Fall",
		[this]()
		{
			myIsFinished = true;
		});
}

void FB::HandSlam_Fall::Update()
{
	myTimer.Tick(Time::DeltaTime);

	auto polling = myHandSlam.GetLeftHand().GetGameObject()->GetSingletonComponent<PollingStationComponent>();
	const auto playerPos = polling->GetPlayerPos();

	auto& modelTransform = myHandSlam.GetLeftHand().GetTransform();

	float speedT = 1.f;

	const auto& speedCurve = myHandSlam.GetLeftHand().GetHandSlamDesc().mySlamSpeedCurve;

	if (speedCurve && speedCurve->IsValid())
	{
		speedT = speedCurve->Get().Evaluate(myTimer.Percent());
	}
	else
	{
		LOG_ERROR(LogType::Game) << "Hand slam is missing slam speed curve";
	}

	const float t = myTimer.Percent() * speedT;

	const Vec3f curvePos = myBezier.Evaluate(t);
	modelTransform.SetPosition(curvePos);

	if (t >= 1.f && !myHitGround)
	{
		myHitGround = true;

		OnHitGround();
	}

	CheckPlayerCollision();
}

bool FB::HandSlam_Fall::IsFinished()
{
	return myIsFinished;
}

void FB::HandSlam_Fall::OnEnter()
{
	myHandSlam.GetLeftHand().GetAnimator().GetController().SetInt(Constants::SlamStateIntName, 3);

	myIsFinished = false;
	myHitGround = false;
	myHasHitPlayer = false;

	auto polling = myHandSlam.GetLeftHand().GetGameObject()->GetSingletonComponent<PollingStationComponent>();
	const auto playerPos = polling->GetPlayerPos();

	myTimer = TickTimer::FromSeconds(myHandSlam.GetLeftHand().GetHandSlamDesc().myFallDurationSec * myHandSlam.GetLeftHand().GetEnragedDurationMultiplier());
	float distanceToGround = 500.f;

	const auto down = Vec3f(0.f, -1.f, 0.f);

	const auto startPos = myHandSlam.GetLeftHand().GetTransform().GetPosition();

	RayCastHit hit;

	if (myHandSlam.GetLeftHand().GetGameObject()->GetScene()->RayCast(startPos, down, Constants::RaycastDistance, /*eLayer::NAVMESH*/eLayer::GROUND | eLayer::DEFAULT, hit))
	{
		distanceToGround = hit.Distance;
	}
	else
	{
		LOG_ERROR(LogType::Game) << "Finalboss raycast navmesh failed";
	}

	myBezier = { };
	myBezier.AddPoint(startPos);
	myBezier.AddPoint(myHandSlam.GetLeftHand().GetTransform().GetPosition() + down * distanceToGround);
}

void FB::HandSlam_Fall::OnExit()
{
	myHandSlam.HandSlamImpactSound(false);
}

void FB::HandSlam_Fall::OnHitGround()
{
	auto mainCam = myHandSlam.GetLeftHand().GetGameObject()->GetScene()->GetMainCameraGameObject();
	auto shakeComponent = mainCam->GetComponent<Engine::CameraShakeComponent>();
	if (shakeComponent)
	{
		shakeComponent->AddPerlinShake("HandSlamHitGround");
		shakeComponent->AddKickShake("HandSlamHitGround", Vec3f(-1.f, 0.f, 0.f));
	}

	if (myHandSlam.Desc().myHitGroundVFX && myHandSlam.Desc().myHitGroundVFX->IsValid())
	{
		auto g = myHandSlam.GetLeftHand().GetGameObject()->GetScene()->AddGameObject<GameObject>();
		g->GetTransform().SetPosition(myHandSlam.GetLeftHand().GetTransform().GetPosition());
		auto myVFX = g->AddComponent<Engine::VFXComponent>(myHandSlam.Desc().myHitGroundVFX->Get());
		myVFX->Play();
		myVFX->AutoDestroy();
	}
	else
	{
		LOG_ERROR(LogType::Game) << "Missing slam hit ground VFX";
	}

	myHandSlam.HandSlamImpactSound(true);

	myIsFinished = true;
}

void FB::HandSlam_Fall::CheckPlayerCollision()
{
	std::vector<OverlapHit> hits;

	auto& leftHand = myHandSlam.GetLeftHand();

	auto rb = leftHand.GetColliderGameObject()->GetComponent<Engine::RigidBodyComponent>();

	if (!myHasHitPlayer)
	{
		// NOTE(filip): Must do box cast/sphere cast because the trigger does not work
		const bool hitSomething = leftHand.GetGameObject()->GetScene()->BoxCastAll(
			rb->GetTransform().GetPosition(),
			rb->GetTransform().GetRotation(),
			rb->GetShapes()[0].GetBoxHalfSize(),
			rb->GetGameObject()->GetDetectionLayerMask() | eLayer::DEFAULT /* DEFAULT to make pillars block */,
			hits);

		if (hitSomething)
		{
			for (int i = static_cast<int>(hits.size()) - 1; i >= 0; --i)
			{
				const auto& hit = hits[i];

				if (hit.GameObject->GetTag() == eTag::PLAYER)
				{
					if (auto health = hit.GameObject->GetComponent<HealthComponent>())
					{
						LOG_ERROR(LogType::Filip) << "Collided with player";

						health->ApplyDamage(leftHand.GetHandSlamDesc().myDamage.GetDamage());

						//const auto upStrength = Vec3f(0, 1, 0) * leftHand.GetHandPunchDesc().myHitForce;
						//const auto punchDirectionStrength = leftHand.GetTransform().Forward() * leftHand.GetHandPunchDesc().myHitForce;
						//health->ApplyKnockback(punchDirectionStrength + upStrength);

						myHasHitPlayer = true;
					}
				}
				/*
				else
				{
					LOG_ERROR(LogType::Filip) << "Collided with wall";

					if (myStateSequencer.GetActiveStateIndex() == HandPunchStateType::MoveTowardsPlayer)
					{
						auto moveTowardsPlayer = reinterpret_cast<HandPunch_MoveTowardsPlayer*>(myStateSequencer.GetActiveState());
						moveTowardsPlayer->OnCollidedWithObstacle();
					}

					// Avoid colling with other object such as the player
					break;
				}
				*/
			}
		}
	}
}
