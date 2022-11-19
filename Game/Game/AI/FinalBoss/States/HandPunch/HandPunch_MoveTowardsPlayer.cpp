#include "pch.h"
#include "HandPunch_MoveTowardsPlayer.h"
#include "AI/FinalBoss/Behaviours/HandPunch.h"
#include "AI/FinalBoss/LeftPunchHand.h"
#include "Engine/GameObject/GameObject.h"
#include "AI/PollingStation/PollingStationComponent.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "AI/FinalBoss/Constants.h"
#include "Engine/ResourceManagement/Resources/AnimationCurveResource.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"
#include "Engine/AnimationCurve/Curve.h"
#include "Engine/GameObject/Components/VFXComponent.h"
#include "Engine/GameObject/Components/CameraShakeComponent.h"

FB::HandPunch_MoveTowardsPlayer::HandPunch_MoveTowardsPlayer(HandPunch& aParent)
	: myHandPunch(aParent)
{
	if (aParent.GetLeftHand().GetHandPunchDesc().mySpeedCurve &&
		aParent.GetLeftHand().GetHandPunchDesc().mySpeedCurve->IsValid())
	{
		mySpeedCurve = &aParent.GetLeftHand().GetHandPunchDesc().mySpeedCurve->Get();
	}
	else
	{
		LOG_ERROR(LogType::Game) << "Hand punch is missing speed curve";
	}
}

void FB::HandPunch_MoveTowardsPlayer::Update()
{
	myInitialDelay.Tick(Time::DeltaTime);

	if (!myInitialDelay.IsFinished())
	{
		return;
	}

	// Prevent it from moving through obstacle more than necessary
	if (myCollidedWithObstacle)
	{
		return;
	}

	myTimer.Tick(Time::DeltaTime);

	auto& modelTransform = myHandPunch.GetLeftHand().GetTransform();

	float speedT = 1.f;

	if (mySpeedCurve)
	{
		speedT = mySpeedCurve->Evaluate(myTimer.Percent());
	}
	else
	{
		LOG_ERROR(LogType::Game) << "Hand punch is missing speed curve";
	}

	const float t = myTimer.Percent() * speedT;

	const Vec3f curvePos = myBezier.Evaluate(t);
	const Vec3f nextCurvePos = myBezier.Evaluate(t + 0.001f);
	const Vec3f direction = (nextCurvePos - curvePos).GetNormalized();
	modelTransform.SetPosition(curvePos);

	auto r = modelTransform.GetLookAtResult(modelTransform.GetPosition() + direction);
	r = r * Quatf(Vec3f(0.4f, 0.f, 0.f));
	// modelTransform.SetRotation(r);

	myHandPunch.GetLeftHand().SetTargetRotation(r);

	SpawnTrailVFX();

	if (!myPlayedSound && myTimer.Percent() >= 0.2f)
	{
		myPlayedSound = true;

		myHandPunch.HandPunchSound(true);
	}

	if (myTimer.JustFinished())
	{
		SpawnCollidedVFX();

		if (!myHandPunch.HasHitPlayer())
		{
			HitGroundShake();
		}
	}
}

bool FB::HandPunch_MoveTowardsPlayer::IsFinished()
{
	return myTimer.IsFinished() || myCollidedWithObstacle/* || myHandPunch.HasHitPlayer()*/;
}

void FB::HandPunch_MoveTowardsPlayer::OnEnter()
{
	myHandPunch.GetLeftHand().GetAnimator().GetController().SetInt(Constants::JabStateIntName, 1);

	myInitialDelay = TickTimer::FromSeconds(0.5f);

	myTimer = TickTimer::FromSeconds(myHandPunch.GetLeftHand().GetHandPunchDesc().myTimeToReachPlayerSec * myHandPunch.GetLeftHand().GetEnragedDurationMultiplier());

	auto polling = myHandPunch.GetLeftHand().GetGameObject()->GetSingletonComponent<PollingStationComponent>();
	const auto playerPos = polling->GetPlayerPos();

	const auto origin = myHandPunch.GetLeftHand().GetTransform().GetPosition();
	const auto dirToPlayer = (playerPos - origin).GetNormalized();

	RayCastHit hit;
	if (myHandPunch.GetLeftHand().GetGameObject()->GetScene()->RayCast(
		origin,
		dirToPlayer,
		/*Constants::RaycastDistance*/10000.f * 2.f,
		eLayer::DEFAULT | eLayer::GROUND,
		hit))
	{
		myBezier = {};
		myBezier.AddPoint(origin);
		myBezier.AddPoint(hit.Position);
	}
	else
	{
		// fail safe
		LOG_ERROR(LogType::Game) << "hand punch failed to raycast to player";

		myBezier = {};
		myBezier.AddPoint(origin);
		myBezier.AddPoint(playerPos);
	}

	myCollidedWithObstacle = false;

	myPlayedSound = false;
}

void FB::HandPunch_MoveTowardsPlayer::OnExit()
{
	myHandPunch.HandPunchSound(false);
}

void FB::HandPunch_MoveTowardsPlayer::OnCollidedWithObstacle()
{
	myCollidedWithObstacle = true;

	SpawnCollidedVFX();

	// Spawn obstacle collision VFX
}

void FB::HandPunch_MoveTowardsPlayer::SpawnTrailVFX()
{
	// disabled because looks weird

	/*
	if (Desc().myPunchTrailVFX && Desc().myPunchTrailVFX->IsValid())
	{
		auto scene = myHandPunch.GetLeftHand().GetGameObject()->GetScene();
		auto g = scene->AddGameObject<GameObject>();
		g->GetTransform().SetPosition(myHandPunch.GetLeftHand().GetTransform().GetPosition());
		auto myVFX = g->AddComponent<Engine::VFXComponent>(Desc().myPunchTrailVFX->Get());
		myVFX->Play();
		myVFX->AutoDestroy();
	}
	else
	{
		LOG_ERROR(LogType::Game) << "Missing VFX";
	}
	*/
}

const FB::HandPunchDesc& FB::HandPunch_MoveTowardsPlayer::Desc() const
{
	return myHandPunch.GetLeftHand().GetHandPunchDesc();
}

void FB::HandPunch_MoveTowardsPlayer::HitGroundShake()
{
	auto mainCam = myHandPunch.GetLeftHand().GetGameObject()->GetScene()->GetMainCameraGameObject();
	auto shakeComponent = mainCam->GetComponent<Engine::CameraShakeComponent>();
	if (shakeComponent)
	{
		shakeComponent->AddPerlinShake("HandSlamHitGround");
		shakeComponent->AddKickShake("HandSlamHitGround", Vec3f(-1.f, 0.f, 0.f));
	}
}

void FB::HandPunch_MoveTowardsPlayer::SpawnCollidedVFX()
{
	const auto& desc = myHandPunch.GetLeftHand().GetHandPunchDesc();

	if (desc.myPunchCollidedVFX && desc.myPunchCollidedVFX->IsValid())
	{
		auto g = myHandPunch.GetLeftHand().GetGameObject()->GetScene()->AddGameObject<GameObject>();
		g->GetTransform().SetRotation(myHandPunch.GetLeftHand().GetTransform().GetRotation() * Quatf(Vec3f(0.f, Math::PI, 0.f)));
		g->GetTransform().SetPosition(myHandPunch.GetLeftHand().GetTransform().GetPosition());
		auto myVFX = g->AddComponent<Engine::VFXComponent>(desc.myPunchCollidedVFX->Get());
		myVFX->Play();
		myVFX->AutoDestroy();
	}
	else
	{
		LOG_ERROR(LogType::Game) << "Missing hand punch collided VFX";
	}
}
