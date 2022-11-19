#include "pch.h"
#include "HandGatlingEye_Loop.h"
#include "AI/FinalBoss/Behaviours/HandGattlingEye.h"
#include "AI/FinalBoss/LeftPunchHand.h"
#include "Engine/GameObject/GameObject.h"
#include "AI/PollingStation/PollingStationComponent.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "Engine/GameObject/Components/AudioComponent.h"
#include "AI/FinalBoss/Constants.h"
#include "AI/FinalBoss/RightProjectileHand.h"
#include "Engine/ResourceManagement/Resources/GameObjectPrefabResource.h"
#include "Engine/GameObject/Prefab/GameObjectPrefab.h"
#include "AI/FinalBoss/HandGatlingProjectile.h"
#include "AI/FinalBoss/FinalBoss.h"
#include "Engine/GameObject/Components/PointLightComponent.h"

FB::HandGatlingEye_Loop::HandGatlingEye_Loop(HandGattlingEye& aParent)
	: myEye(aParent),
	myDesc(aParent.GetRightHand().GetGattlingEyeDesc())
{
	if (myDesc.myTelegraphOrbPrefab)
	{
		myTelegraphOrbPrefab = &myDesc.myTelegraphOrbPrefab->Get();
	}
	else
	{
		LOG_ERROR(LogType::Game) << "Gatling eye missing telegraph orb prefab";
	}
}

void FB::HandGatlingEye_Loop::Update()
{
	myTimer.Tick(Time::DeltaTime);

	auto& rightHand = myEye.GetRightHand();

	const auto playerPos = rightHand.GetFinalBoss().GetPlayerPos();

	auto r = rightHand.GetTransform().GetLookAtResult(playerPos);
	rightHand.SetTargetRotation(r);

	const auto playerMoveDir = (playerPos - myTargetPos).GetNormalized();
	myTargetPos = Math::Lerp(myTargetPos, playerPos, Time::DeltaTime * myDesc.myTargetPlayerSpeed);

	GDebugDrawer->DrawSphere3D(DebugDrawFlags::Gameplay, myTargetPos, 100.f, 0.f, Vec4f(1, 0, 0, 1));

	RayCastHit groundHit;
	if (myEye.GetRightHand().GetGameObject()->GetScene()->RayCast(myTargetPos, Vec3f(0.f, -1.f, 0.f), Constants::RaycastDistance, /*eLayer::NAVMESH*/eLayer::GROUND | eLayer::DEFAULT, groundHit))
	{
		UpdateTelegraphOrb(groundHit.Position - GetLaserOutputPosition());
	}
	else
	{
		LOG_ERROR(LogType::Game) << "gatling eye raycast failed ground below player";
	}

	myRapidFireTimer -= Time::DeltaTime * myDesc.myRapidFireSpeedFactor * myEye.GetRightHand().GetEnragedDamageMultiplier();

	if (myRapidFireTimer < 0.f)
	{
		const float rapidFireWaitTime = 0.1f;
		myRapidFireTimer += rapidFireWaitTime;

		if (myDesc.myLaserPrefab && myDesc.myLaserPrefab->IsValid())
		{
			auto& laserGameObject = myDesc.myLaserPrefab->Get().Instantiate(*rightHand.GetGameObject()->GetScene());

			if (auto gatlingProjectile = laserGameObject.GetComponent<HandGatlingProjectile>())
			{
				const auto targetPos = myTargetPos + Random::InsideUnitSphere() * myDesc.mySpreadRadius;
				const auto handToTarget = targetPos - GetLaserOutputPosition();
				const auto dir = (handToTarget).GetNormalized();

				gatlingProjectile->SetDamage(myDesc.myProjectileDamage.GetDamage());
				gatlingProjectile->SetSecondsUntilSelfDestroy(myDesc.mySingleLaserDuration);

				gatlingProjectile->Launch(
					GetLaserOutputPosition(),
					dir,
					handToTarget.Length());

				if (myFlashPointlightObj && myFlashPointlight)
				{
					myFlashPointlightObj->SetActive(true);
					myFlashPointlightObj->GetTransform().SetPosition(GetLaserOutputPosition());

					myFlashPointlight->SetColor(Vec3f(0.945f, 0.460f, 0.290f));
					myFlashPointlight->SetRange(5000.f);
					myFlashPointlight->SetIntensity(4000000.f);
				}
				else
				{
					LOG_ERROR(LogType::Game) << "Gatling eye point light is null";
				}
			}
			else
			{
				LOG_ERROR(LogType::Game) << "Homing Laser prefab is missing HandHomingProjectile component";
			}
		}
	}

	if (myFlashPointlight)
	{
		const float intensity = myFlashPointlight->GetIntensity();
		const float lightDecreaseSpeed = 15.f;
		const float newIntensity = Math::Lerp(intensity, 0.f, Time::DeltaTime * lightDecreaseSpeed);
		myFlashPointlight->SetIntensity(std::max(newIntensity, 0.f));
	}

	myPlayerLastPos = playerPos;
}

bool FB::HandGatlingEye_Loop::IsFinished()
{
	return myTimer.IsFinished();
}

void FB::HandGatlingEye_Loop::OnEnter()
{
	auto polling = myEye.GetRightHand().GetGameObject()->GetSingletonComponent<PollingStationComponent>();

	myFlashPointlightObj = polling->GetPointlightFromPool();
	myFlashPointlight = myFlashPointlightObj->GetComponent<Engine::PointLightComponent>();

	myEye.GetRightHand().GetAnimator().GetController().SetInt(Constants::GatlingEyeStateIntName, 1);

	const auto playerPos = polling->GetPlayerPos();

	const auto& durationRange = myEye.GetRightHand().GetGattlingEyeDesc().myShootingDurationRange;

	myTimer = TickTimer::FromSeconds(Random::RandomFloat(durationRange.x, durationRange.y));

	myTargetPos = playerPos;

	if (myTelegraphOrbPrefab)
	{
		myTelegraphOrb = &myTelegraphOrbPrefab->Instantiate(
			*myEye.GetRightHand().GetGameObject()->GetScene());
	}
	else
	{
		LOG_ERROR(LogType::Game) << "Gatling eye missing telegraph orb prefab";
	}

	GatlingLaserSound(true);
}

void FB::HandGatlingEye_Loop::OnExit()
{
	auto pollingStation = myEye.GetRightHand().GetGameObject()->GetSingletonComponent<PollingStationComponent>();
	pollingStation->ReturnPointLightToPool(myFlashPointlightObj);
	myFlashPointlightObj = nullptr;
	myFlashPointlight = nullptr;

	if (myTelegraphOrb)
	{
		myTelegraphOrb->Destroy();
		myTelegraphOrb = nullptr;
	}

	GatlingLaserSound(false);
}

void FB::HandGatlingEye_Loop::UpdateTelegraphOrb(const Vec3f& aDirection)
{
	if (myTelegraphOrb)
	{
		RayCastHit hit;

		if (myEye.GetRightHand().GetGameObject()->GetScene()->RayCast(
			GetLaserOutputPosition(),
			aDirection.GetNormalized(),
			Constants::RaycastDistance,
			eLayer::GROUND | eLayer::DEFAULT,
			hit))
		{
			myTelegraphOrb->GetTransform().SetPosition(hit.Position);
		}
		else
		{
			LOG_ERROR(LogType::Game) << "gatling eye raycast navmesh ground failed";
		}
	}
}

Vec3f FB::HandGatlingEye_Loop::GetLaserOutputPosition() const
{
	const auto forward = myEye.GetRightHand().GetTransform().Forward();

	Vec3f t;
	myEye.GetRightHand().GetEyeWorldTransform(&t, nullptr, nullptr);
	return t + forward * 200.f;
}

void FB::HandGatlingEye_Loop::GatlingLaserSound(const bool aEnable) const
{
	if (aEnable)
	{
		myEye.GetRightHand().GetAudio().PlayEvent("GatlingLaser");
	}
	else
	{
		myEye.GetRightHand().GetAudio().StopEvent("GatlingLaser");
	}
}
