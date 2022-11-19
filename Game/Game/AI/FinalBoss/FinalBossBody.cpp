#include "pch.h"
#include "FinalBossBody.h"
#include "Engine/GameObject/GameObject.h"
#include "Components/HealthComponent.h"
#include "Engine/GameObject/Components/ModelComponent.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "Engine/GameObject/Components/ModelComponent.h"
#include "FinalBoss.h"
#include "Behaviours/Behaviour.h"
#include "Behaviours/Body/BodyIdle.h"
#include "Behaviours/Body/BodyDeath.h"
#include "Behaviours/Body/BodyGigaLaser.h"
#include "Behaviours/Body/BodyMouthMortar.h"
#include "Behaviours/Body/BodyShockwaveScream.h"
#include "Behaviours/Body/BodyPoisonBreath.h"
#include "AI/PollingStation/PollingStationComponent.h"
#include "Behaviours/Body/BodyIntro.h"
#include "Behaviours/Body/BodyPhaseSwitch.h"
#include "Engine/GameObject/Components/AudioComponent.h"

FB::FinalBossBody::FinalBossBody(GameObject* aGameObject)
	: FinalBossPart(aGameObject)
{
}

void FB::FinalBossBody::Awake()
{
	FinalBossPart::Awake();
}

void FB::FinalBossBody::Start()
{
	FinalBossPart::Start();

	AddNormalBehaviour(FinalBossBodyBehaviours::Idle, MakeOwned<BodyIdle>(*this));
	AddNormalBehaviour(FinalBossBodyBehaviours::Death, MakeOwned<BodyDeath>(*this));
	AddNormalBehaviour(FinalBossBodyBehaviours::Intro, MakeOwned<BodyIntro>(*this));
	AddNormalBehaviour(FinalBossBodyBehaviours::PhaseSwitch, MakeOwned<BodyPhaseSwitch>(*this));

	AddAttackBehaviour(FinalBossBodyBehaviours::GigaLaser, MakeOwned<BodyGigaLaser>(*this));
	AddAttackBehaviour(FinalBossBodyBehaviours::MouthMortar, MakeOwned<BodyMouthMortar>(*this));
	AddAttackBehaviour(FinalBossBodyBehaviours::ShockwaveScream, MakeOwned<BodyShockwaveScream>(*this));
	AddAttackBehaviour(FinalBossBodyBehaviours::PoisonBreath, MakeOwned<BodyPoisonBreath>(*this));

	InitAudio();

	myCurrentRotation = GetTransform().GetRotation();
}

void FB::FinalBossBody::InitAudio()
{
	Engine::AudioComponent& audio = *GetBodyAudio();

	audio.AddEvent("Death", "event:/CINE/BossDeath");
	audio.AddEvent("Falling", "event:/SFX/NPC/END BOSS/Falling");
	audio.AddEvent("GigaLaser", "event:/SFX/NPC/END BOSS/PHASE 2/GigaLaser");
	audio.AddEvent("MouthMortar", "event:/SFX/NPC/END BOSS/PHASE 2/MouthMortar");
	audio.AddEvent("PoisonBreath", "event:/SFX/NPC/END BOSS/PHASE 2/PoisonBreath");
	audio.AddEvent("ShockwaveScream", "event:/SFX/NPC/END BOSS/PHASE 2/ShockwaveScream");
}

void FB::FinalBossBody::Reflect(Engine::Reflector& aReflector)
{
	FinalBossPart::Reflect(aReflector);

	aReflector.Header("Body");

	aReflector.Reflect(myGigaLaserDesc, "Giga Laser");
	aReflector.Reflect(myMortarDesc, "Mortar");
	aReflector.Reflect(myPoisonCloudDesc, "Poison Cloud");
	aReflector.Reflect(myShockwaveDesc, "Shockwave");

	aReflector.Separator();

	FinalBossBodyBehaviours behaviour = FinalBossBodyBehaviours::_from_integral(myActiveBehaviourValue);
	if (aReflector.ReflectEnum(behaviour, "Behaviour"))
	{
		ChangeBehaviour(behaviour);
	}
}

void FB::FinalBossBody::Execute(Engine::eEngineOrder aOrder)
{
	if (IsDemolished())
	{
		return;
	}

	if (IsIntroFinished() && myShouldRotateTowardsPlayer && !IsDead())
	{
		RotateTowardsPlayer();
	}

	// make boss glow always
	SetEmissionGlowTarget(10.f);

	UpdateDamagedColor();
	UpdateEmissionGlow();

	UpdateActiveBehaviour();

	switch (myFinalBoss->GetPhase())
	{
	case FinalBossPhase::Introduction:
		// do an intro for this hand
		break;
	case FinalBossPhase::Phase2:
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

void FB::FinalBossBody::EnterPhase(const FinalBossPhase aPhase)
{
	switch (aPhase)
	{
	case FinalBossPhase::Introduction:
		ChangeBehaviour(FinalBossBodyBehaviours::Intro);
		break;
	case FinalBossPhase::Phase1:
		break;
	case FinalBossPhase::Phase2:
		ChangeBehaviour(FinalBossBodyBehaviours::PhaseSwitch);
		// Change the phase to make it choose another idle animation
		myAnimator->GetController().SetInt("Phase", 1);

		myHealth->SetIsInvincible(false);
		break;
	default:
		break;
	}
}

const FB::GigaLaserDesc& FB::FinalBossBody::GetGigaLaserDesc() const
{
	return myGigaLaserDesc;
}

const FB::MortarDesc& FB::FinalBossBody::GetMortarDesc() const
{
	return myMortarDesc;
}

FB::PoisonCloudDesc& FB::FinalBossBody::GetPoisonCloudDesc()
{
	return myPoisonCloudDesc;
}

const FB::ShockwaveDesc& FB::FinalBossBody::GetShockwaveDesc() const
{
	return myShockwaveDesc;
}

Engine::AudioComponent* FB::FinalBossBody::GetAudio()
{
	if (!myPoisonCloudDesc.mySpawnPoint)
	{
		assert(false && "No spawn point attached to poison cloud. Should be TopJoint object.");
		return nullptr;
	}

	Engine::AudioComponent* comp = myPoisonCloudDesc.mySpawnPoint.Get()->GetComponent<Engine::AudioComponent>();
	if (!comp)
	{
		assert(false && "No audio component attached to TopJoint object.");
		return nullptr;
	}

	return comp;
}

Engine::AudioComponent* FB::FinalBossBody::GetBodyAudio()
{
	if (myAudio != nullptr)
		return myAudio;
	else
	{
		myAudio = myGameObject->AddComponent<Engine::AudioComponent>();
		return myAudio;
	}
}

bool FB::FinalBossBody::IsIntroFinished() const
{
	return myIsIntroFinished;
	/*
	if (Behaviour* b = GetNormalBehaviour(FinalBossBodyBehaviours::Intro))
	{
		return b->IsFinished();
	}
	else
	{
		LOG_ERROR(LogType::Game) << "Unable to find behaviour intro";
		return true;
	}
	*/
}

bool FB::FinalBossBody::IsDemolished() const
{
	return GetNormalBehaviour(FinalBossBodyBehaviours::Death)->IsFinished();
}

void FB::FinalBossBody::SetShouldRotateTowardsPlayer(const bool aValue)
{
	myShouldRotateTowardsPlayer = aValue;
}

void FB::FinalBossBody::SetIsIntroFinished(const bool aIsFinished)
{
	myIsIntroFinished = true;
}

void FB::FinalBossBody::OnDamaged(const float aDamage)
{
	myDamagedTimer = TickTimer::FromSeconds(0.3f);
}

void FB::FinalBossBody::OnDeath()
{
	ChangeBehaviour(FinalBossBodyBehaviours::Death);
}

void FB::FinalBossBody::RotateTowardsPlayer()
{
	auto polling = myGameObject->GetSingletonComponent<PollingStationComponent>();
	const auto playerPos = polling->GetPlayerPos();

	Vec3f position = GetTransform().GetPosition();
	Vec3f direction = (playerPos - position).GetNormalized();

	const float angle = std::atan2f(direction.x, direction.z);

	Quatf startRot = GetTransform().GetRotation();
	Quatf endRot = Quatf(Vec3f(
		startRot.EulerAngles().x,
		-angle,
		startRot.EulerAngles().z
	));

	GetTransform().SetRotation(endRot);

	myCurrentRotation = Quatf::Slerp(myCurrentRotation, endRot, Time::DeltaTime * 6.f);
	GetTransform().SetRotation(myCurrentRotation);
}
