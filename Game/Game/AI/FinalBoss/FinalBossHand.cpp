#include "pch.h"
#include "FinalBossHand.h"
#include "Engine/GameObject/GameObject.h"
#include "Components/HealthComponent.h"
#include "Engine/GameObject/Components/RigidBodyComponent.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "Engine/GameObject/Components/AudioComponent.h"
#include "Engine/GameObject/Components/ModelComponent.h"
#include "FinalBoss.h"
#include "AI/PollingStation/PollingStationComponent.h"
#include "AI/FinalBoss/FinalBossPortal.h"

FB::FinalBossHand::FinalBossHand()
{
}

FB::FinalBossHand::FinalBossHand(GameObject* aGameObject)
	: FinalBossPart(aGameObject)
{
}

FB::FinalBossHand::~FinalBossHand()
{
}

void FB::FinalBossHand::Start()
{
	FinalBossPart::Start();
	InitAudio();

	myEyeBoneIndex = myAnimator->GetController().GetBoneIndexFromName("Eye");

	if (myEyeBoneIndex == -1)
	{
		LOG_ERROR(LogType::Game) << "cannot find 'Eye' bone";
	}
};

void FB::FinalBossHand::InitAudio()
{
	myAudio = myGameObject->GetComponent<Engine::AudioComponent>();

	// AddEvent loads the sound from BANK, dyr
	myAudio->AddEvent("GatlingLaserStart", "event:/SFX/NPC/END BOSS/PHASE 1/GattlingLaserStart");
	myAudio->AddEvent("GatlingLaser", "event:/SFX/NPC/END BOSS/PHASE 1/GattlingLaser");
	myAudio->AddEvent("HomingLaser", "event:/SFX/NPC/END BOSS/PHASE 1/HomingLaser");
	myAudio->AddEvent("CrawlingHand", "event:/SFX/NPC/END BOSS/PHASE 1/CrawlingHand");

	myAudio->AddEvent("HandJab", "event:/SFX/NPC/END BOSS/PHASE 1/HandJab");
	myAudio->AddEvent("HandSlam", "event:/SFX/NPC/END BOSS/PHASE 1/HandSlam");
	myAudio->AddEvent("HandSlamImpact", "event:/SFX/NPC/END BOSS/PHASE 1/HandSlamImpact");
}

bool FB::FinalBossHand::IsIntroFinished() const
{
	// TODO(filip): Do an into behaviour for the hands and then use
	// the same way as in FinalBossBody::IsIntroFinished() to check if it finished
	return true;
}

const Vec3f& FB::FinalBossHand::GetDefaultPos() const
{
	return myDefaultPos;
}

void FB::FinalBossHand::TargetWaypoint(FinalBossWaypoint* aWaypoint)
{
	if (myTargetWaypoint)
	{
		myTargetWaypoint->Release();
	}

	myTargetWaypoint = aWaypoint;
	myTargetWaypoint->Take();
}

void FB::FinalBossHand::SetTargetRotation(const Quatf& aRotation, const float aRotationSpeed)
{
	if (aRotationSpeed > -1.01f && aRotationSpeed < 0.99f)
	{
		myCurrentRotationSpeed = myRotationSpeed;
	}
	else
	{
		myCurrentRotationSpeed = aRotationSpeed;
	}

	myTargetRotation = aRotation;
}

Engine::AudioComponent& FB::FinalBossHand::GetAudio()
{
	return *myAudio;
}

const VFXRef& FB::FinalBossHand::GetDeathVFX() const
{
	return myDeathVFX;
}

const VFXRef& FB::FinalBossHand::GetExplosionAftermathVFX() const
{
	return myExplosionAftermathVFX;
}

const VFXRef& FB::FinalBossHand::GetDeathAfterMathVFX() const
{
	return myDeathAfterMathVFX;
}

const FB::HandCrawlerDesc& FB::FinalBossHand::GetHandCrawlerDesc() const
{
	return myCrawlerDesc;
}

const FB::HandEnragedDesc& FB::FinalBossHand::GetEnragedDesc() const
{
	return myEnragedDesc;
}

void FB::FinalBossHand::GetEyeWorldTransform(Vec3f* aPosition, Vec3f* aScale, Quatf* aRotation)
{
	const auto transform = myAnimator->GetController().GetBoneTransformWorld(myEyeBoneIndex);

	Vec3f t;
	Quatf r;
	Vec3f s;
	transform.Decompose(t, r, s);

	if (aPosition)
	{
		*aPosition = t;
	}

	if (aScale)
	{
		*aScale = s;
	}

	if (aRotation)
	{
		*aRotation = r;
	}
}

float FB::FinalBossHand::GetEnragedDamageMultiplier() const
{
	if (myHasEnraged)
	{
		return myEnragedDamageAndCountMultiplier;
	}
	else
	{
		return 1.f;
	}
}

float FB::FinalBossHand::GetEnragedDurationMultiplier() const
{
	if (myHasEnraged)
	{
		return myEnragedDurationMultiplier;
	}
	else
	{
		return 1.f;
	}
}

void FB::FinalBossHand::Execute(Engine::eEngineOrder aOrder)
{
	if (IsDemolished())
	{
		return;
	}

	Failsafe();

	UpdateDamagedColor();
	UpdateEmissionGlow();

	UpdateActiveBehaviour();

	UpdateDefaultPositions();

	if (!IsDead())
	{
		UpdateRotation();
	}

	const bool isDoingAttack = GetAttackBehaviour(GetBehaviour()) != nullptr;

	if (Behaviour* behaviour = GetActiveBehaviour())
	{
		if (behaviour->IsAllowedToMoveExternally())
		{
			// not allowed to move when dead
			if (!IsDead())
			{
				MoveTowardsWaypoint();

				// Look at player
				SetTargetRotation(GetTransform().GetLookAtResult(myFinalBoss->GetPlayerPos()));
			}
		}
	}

	switch (myFinalBoss->GetPhase())
	{
	case FinalBossPhase::Introduction:
		// do an intro for this hand
		break;
	case FinalBossPhase::Phase1:
		if (!IsDead())
		{
			RestockAttackBehaviourStack();
			UpdateAttackTransitions();
		}
		break;
	default:
		break;
	}
}

void FB::FinalBossHand::Reflect(Engine::Reflector& aReflector)
{
	FinalBossPart::Reflect(aReflector);

	aReflector.Reflect(myHandDefaultPosGameObject, "Hand Default Pos GameObject");
	aReflector.Reflect(myDeathVFX, "Death VFX");
	aReflector.Reflect(myExplosionAftermathVFX, "Explosion Aftermath VFX");
	aReflector.Reflect(myDeathAfterMathVFX, "Death Aftermath VFX");
	aReflector.Reflect(myCrawlerDesc, "Crawler");
	aReflector.Reflect(myEnragedDesc, "Enraged");
	aReflector.Reflect(myEnragedGlowMinMax, "Enraged Glow Min Max ");
	aReflector.Reflect(myEnragedDurationMultiplier, "Enraged Duration Multiplier");
	aReflector.Reflect(myEnragedDamageAndCountMultiplier, "Enraged Damage And Count Multiplier");

	Vec3f r = myTargetRotation.EulerAngles();
	aReflector.Reflect(r, "R");
	myTargetRotation = Quatf(r);
}

void FB::FinalBossHand::Enrage(const BehaviourEnumValue aEnragedEnumValue)
{
	if (myHasEnraged)
	{
		return;
	}

	myHasEnraged = true;

	ChangeBehaviour(aEnragedEnumValue);

	SetGlowMinMax(myEnragedGlowMinMax);

	myHealth->ApplyHealing(myHealth->GetMaxHealth() * 0.25f);
}

void FB::FinalBossHand::UpdateDefaultPositions()
{
	if (myHandDefaultPosGameObject.IsValid())
	{
		myDefaultPos = myHandDefaultPosGameObject.Get()->GetTransform().GetPosition();
		myDefaultRot = myHandDefaultPosGameObject.Get()->GetTransform().GetRotation();
		myDefaultWaypoint = FinalBossWaypoint(myDefaultPos);
	}
	else
	{
		LOG_ERROR(LogType::Game) << "Boss hand missing hand default pos gameobject";
	}
}

void FB::FinalBossHand::MoveTowardsWaypoint()
{
	auto polling = myGameObject->GetSingletonComponent<PollingStationComponent>();
	const auto playerPos = polling->GetPlayerPos();

	FinalBossWaypoint* waypoint =
		myFinalBoss->FindFreeWaypointClosestToPosition(playerPos, myTargetWaypoint);

	myTargetWaypointCooldown.Tick(Time::DeltaTime);

	if (waypoint)
	{
		// Add a cooldown to make sure not changing position every frame
		if (myTargetWaypointCooldown.IsFinished())
		{
			TargetWaypoint(waypoint);
			LOG_INFO(LogType::Game) << "Targeting new waypoint";
			myTargetWaypointCooldown = TickTimer::FromSeconds(1.f * GetEnragedDurationMultiplier());
		}
	}

	if (myTargetWaypoint != nullptr)
	{
		GDebugDrawer->DrawSphere3D(DebugDrawFlags::Gameplay, myTargetWaypoint->GetPosition(), 200.f);

		auto pos = GetTransform().GetPosition();
		//pos = Math::SmoothDamp(pos, myDefaultPos, mySmoothDampVelocity, 0.3f, 2000.f, Time::DeltaTime);
		pos = Math::SmoothDamp(
			pos,
			myTargetWaypoint->GetPosition(),
			mySmoothDampVelocity,
			0.3f * GetEnragedDurationMultiplier(),
			1500.f * GetEnragedDamageMultiplier(),
			Time::DeltaTime);

		GetTransform().SetPosition(pos);

		if (myCurrentTime >= 1.f)
		{
			myCurrentTime = 0.f;
		}

		// Quatf rot = GetTransform().GetRotation();
		// rot = Quatf::SmoothSlerp(rot, myDefaultRot, myCurrentTime, 1.f, Time::DeltaTime);
		// GetTransform().SetRotation(rot);
	}
	else
	{
		myTargetWaypoint = &myDefaultWaypoint;
	}
}

void FB::FinalBossHand::UpdateRotation()
{
	myCurrentRotation = Quatf::Slerp(myCurrentRotation, myTargetRotation, Time::DeltaTime * myCurrentRotationSpeed);
	GetTransform().SetRotation(myCurrentRotation);
}

void FB::FinalBossHand::Failsafe()
{
	if (auto portal = myFinalBoss->GetPortal())
	{
		const float maxHeight = portal->GetTransform().GetPosition().y + GetMaxAllowedDistanceAbovePortal();

		const bool isAbovePortal = GetTransform().GetPosition().y > maxHeight;

		if (isAbovePortal)
		{
			GetTransform().SetPosition(GetDefaultPos());
		}
	}
	else
	{
		LOG_ERROR(LogType::Game) << "final boss missing boss portal";
	}
}

