#include "pch.h"
#include "HandHomingLaser.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "AI/FinalBoss/RightProjectileHand.h"
#include "AI/FinalBoss/Constants.h"
#include "Engine/GameObject/Prefab/GameObjectPrefab.h"
#include "Engine/GameObject/GameObject.h"
#include "AI/FinalBoss/HandHomingProjectile.h"
#include "AI/PollingStation/PollingStationComponent.h"
#include "AI/FinalBoss/FinalBoss.h"
#include "Engine/GameObject/Components/VFXComponent.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"
#include "AI/FinalBoss/HandHomingProjectileLauncher.h"
#include "Engine/GameObject/Components/AudioComponent.h"

FB::HandHomingLaser::HandHomingLaser(RightProjectileHand& aRightHand)
	: myRightHand(aRightHand),
	myDesc(myRightHand.GetHomingLaserDesc())
{
	myRightHand.GetAnimator().GetController().AddEventCallback(
		"HomingStartBuildup", [this]() { OnHomingLaserShoot(); });

	myRightHand.GetAnimator().GetController().AddEventCallback(
		"HomingLaserShoot", [this]()
		{
			if (myLauncher)
			{
				myLauncher->Implode();
				myLauncher = nullptr;
			}

			// stop the buildup vfx
			StopBuildupVFX();
		});

	myRightHand.GetAnimator().GetController().AddStateOnExitCallback(
		"Base",
		"Homing Laser",
		[this]()
		{
			myIsFinished = true;
		});
}

void FB::HandHomingLaser::Update()
{
	if (myLauncher)
	{
		myLauncher->GetGameObject()->GetTransform().SetPosition(GetOutputPosition());
		myLauncher->GetTransform().SetRotation(myRightHand.GetTransform().GetRotation());
	}

	if (myBuilupVFX)
	{
		myBuilupVFX->GetGameObject()->GetTransform().SetPosition(GetOutputPosition());
	}

	auto r = myRightHand.GetTransform().GetLookAtResult(myRightHand.GetFinalBoss().GetPlayerPos());
	myRightHand.SetTargetRotation(r);
}

bool FB::HandHomingLaser::IsFinished() const
{
	return myIsFinished;
}

void FB::HandHomingLaser::OnEnter()
{
	myLauncher = nullptr;
	myIsFinished = false;

	myRightHand.GetAnimator().GetController().Trigger(Constants::HomingLaserTriggerName);

	SpawnBuildupVFX();
}

void FB::HandHomingLaser::OnExit()
{
	StopBuildupVFX();

	if (myLauncher)
	{
		myLauncher->GetGameObject()->Destroy();
		myLauncher = nullptr;
	}

	HomingLaserSound(false);
}

void FB::HandHomingLaser::Reflect(Engine::Reflector& aReflector)
{
}

bool FB::HandHomingLaser::IsAllowedToMoveExternally() const
{
	return false;
}

void FB::HandHomingLaser::OnHomingLaserShoot()
{
	HomingLaserSound(true);

	const auto& homingLaser = myDesc.myHomingLaserPrefab;

	if (homingLaser && homingLaser->IsValid())
	{
		auto& projLauncher = homingLaser->Get().Instantiate(*myRightHand.GetGameObject()->GetScene());

		if (myLauncher = projLauncher.GetComponent<HandHomingProjectileLauncher>())
		{
			myLauncher->SetProjectileDamage(myDesc.myProjectileDamage.GetDamage());
			myLauncher->SetProjectilesCount(myDesc.myProjectileCount * myRightHand.GetEnragedDamageMultiplier());
			myLauncher->SetSphereOffsetLength(myDesc.mySphereOffsetLength);
			myLauncher->SetSecondsUntilSelfDestroy(myDesc.mySecondsUntilSelfDestroy);
			myLauncher->SetProjectileSpeed(myDesc.myProjectileSpeed * myRightHand.GetEnragedDamageMultiplier());
		}

		projLauncher.GetTransform().SetPosition(GetOutputPosition());
		projLauncher.GetTransform().SetRotation(myRightHand.GetTransform().GetRotation());
		//projLauncher.GetTransform().LookAt(myRightHand.GetFinalBoss().GetPlayerPos());
	}
	else
	{
		LOG_ERROR(LogType::Game) << "Homing Laser prefab is null";
	}
}

void FB::HandHomingLaser::SpawnBuildupVFX()
{
	myBuilupVFX = nullptr;

	if (Desc().myBuildupVFX && Desc().myBuildupVFX->IsValid())
	{
		auto g = myRightHand.GetGameObject()->GetScene()->AddGameObject<GameObject>();
		g->GetTransform().SetPosition(GetOutputPosition());
		myBuilupVFX = g->AddComponent<Engine::VFXComponent>(Desc().myBuildupVFX->Get());
		myBuilupVFX->Play();
		myBuilupVFX->AutoDestroy();
	}
	else
	{
		LOG_ERROR(LogType::Game) << "Missing buildup VFX";
	}
}

Vec3f FB::HandHomingLaser::GetOutputPosition() const
{
	const auto forward = myRightHand.GetTransform().Forward();

	Vec3f t;
	myRightHand.GetEyeWorldTransform(&t, nullptr, nullptr);
	return t + forward * 200.f;
}

const FB::HomingLaserDesc& FB::HandHomingLaser::Desc() const
{
	return myRightHand.GetHomingLaserDesc();
}

void FB::HandHomingLaser::StopBuildupVFX()
{
	if (myBuilupVFX)
	{
		myBuilupVFX->Stop();
		myBuilupVFX = nullptr;
	}
}

void FB::HandHomingLaser::HomingLaserSound(const bool aEnable)
{
	if (aEnable)
	{
		myRightHand.GetAudio().PlayEvent("HomingLaser");
	}
	else
	{
		myRightHand.GetAudio().StopEvent("HomingLaser");
	}
}

