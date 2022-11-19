#include "pch.h"
#include "GatlingEyeState.h"

#include "Game/Components/HealthComponent.h"

#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/GameObjectRef.h"
#include "Engine/GameObject/Prefab/GameObjectPrefab.h"

#include "Engine/GameObject/Components/CameraShakeComponent.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "Engine/GameObject/Components/VFXComponent.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"

#include "Engine/CameraShake/CameraShakeManager.h"

LevelBossStates::GatlingEyeState::GatlingEyeState(LevelBoss& aBoss, GatlingEyeVars& someVariables) : EngageState(aBoss), myVars(someVariables)
{
	myVars.myCooldownDuration = Random::RandomFloat(myVars.myCooldownDurationMin, myVars.myCooldownDurationMax);
	myVars.myCooldownProgress = myVars.myCooldownDuration;

	myAnimState = AnimState::None;

	if (myVars.myTelegraphingOrbPrefab->IsValid())
	{
		Engine::GameObjectPrefab& prefab = myVars.myTelegraphingOrbPrefab->Get();
		
		auto scene = myBoss.GetGameObject()->GetScene();
		myBossOrb = &prefab.Instantiate(*scene);
		myPlayerOrb = &prefab.Instantiate(*scene);

		myBossOrb->SetName("Boss Orb");
		myPlayerOrb->SetName("Player Orb");

		myBossOrb->SetActive(false);
		myPlayerOrb->SetActive(false);
	}

	if (myVars.myGatlingChargeVFXRef->IsValid())
	{
		if (auto go = myBoss.GetGameObject()->GetScene()->AddGameObject<GameObject>())
		{
			if (myChargeVFX = go->AddComponent<Engine::VFXComponent>(myVars.myGatlingChargeVFXRef->Get()))
			{
				myChargeVFX->Stop();
			}
		}
	}

	auto& shakeManager = GetEngine().GetCameraShakeManager();
	myLaserShake = shakeManager.GetPerlinShake("FinalBossGigaLaser");
}

void LevelBossStates::GatlingEyeState::Update()
{
	auto pollingStation = myBoss.GetGameObject()->GetSingletonComponent<PollingStationComponent>();

	Vec3f playerPosition = pollingStation->GetPlayerPos();
	Vec3f targetDirection = (playerPosition - myLastPlayerPosition);

	const float distance = 1.0f;
	if (targetDirection.LengthSqr() > distance)
	{
		targetDirection.Normalize();
		const float followSpeed = myState == State::AttackStart ? myVars.myTelegraphFollowTargetSpeed : myVars.myFollowTargetSpeed;
		myLastPlayerPosition += targetDirection * followSpeed * Time::DeltaTime;
	}

	//RayCastHit hit;
	//auto scene = myBoss.GetGameObject()->GetScene();
	//if (scene->RayCast(myLastPlayerPosition, Vec3f(0, -1, 0), FLT_MAX, eLayer::DEFAULT | eLayer::GROUND, hit))
	//{
	//	myLastPlayerPosition.y = hit.Position.y;
	//}

	auto boneIndex = GetController().GetBoneIndexFromName("Eye");
	if (boneIndex < 0)
	{
		LOG_ERROR(LogType::Game) << "Eye was not found on level boss skeleton";
		return;
	}

	auto boneTransform = GetController().GetBoneTransformWorld(boneIndex);

	Quatf dec_rot;
	Vec3f dec_scale;

	boneTransform.Decompose(myEyePosition, dec_rot, dec_scale);

	myState == State::Idle ? OnIdle(playerPosition) : OnAttack(playerPosition);
}

void LevelBossStates::GatlingEyeState::ForceUpdate()
{
	myVars.myCooldownProgress -= Time::DeltaTime;
	if (myVars.myCooldownProgress <= 0.0f)
	{
		myVars.myCooldownProgress = 0.0f;
	}

	myVars.myOverrideProgress -= Time::DeltaTime;
	if (myAnimState == AnimState::Opening)
	{
		if (myVars.myOverrideProgress <= 0.0f)
		{
			myVars.myOverrideProgress = 0.0f;
			myAnimState = AnimState::None;
		}
		float ratio = myVars.myOverrideProgress / myVars.myOverrideDuration;
		GetController().SetLayerWeight("Gatling Is Disabled - (Override)", ratio);
	}
	else if (myAnimState == AnimState::Closing)
	{
		if (myVars.myOverrideProgress <= 0.0f)
		{
			myVars.myOverrideProgress = 0.0f;
			myAnimState = AnimState::None;

			myBoss.SetShowEye(false);
		}
		float ratio = 1.0f - (myVars.myOverrideProgress / myVars.myOverrideDuration);
		GetController().SetLayerWeight("Gatling Is Disabled - (Override)", ratio);
	}
}

void LevelBossStates::GatlingEyeState::OnEnterState()
{
	GetController().Trigger("TriggerGatling");
	GetController().SetBool("UsingGatling", true);

	myTempObj = nullptr;

	if (myChargeVFX)
	{
		if (auto go = myChargeVFX->GetGameObject())
		{
			auto& transform = go->GetTransform();
			transform.SetPosition(myBoss.GetEye()->GetTransform().GetPosition());

			myChargeVFX->Play();
		}
	}

	GetAudio().PlayEvent("GattlingEyeStart");
}

void LevelBossStates::GatlingEyeState::OnExitState()
{
	GetController().SetBool("UsingGatling", false);

	auto pollingStation = myBoss.GetGameObject()->GetSingletonComponent<PollingStationComponent>();
	Vec3f playerPosition = pollingStation->GetPlayerPos();
	myLastPlayerPosition = playerPosition;

	myVars.myCooldownDuration = Random::RandomFloat(myVars.myCooldownDurationMin, myVars.myCooldownDurationMax);
	myVars.myCooldownProgress = myVars.myCooldownDuration;

	if (myPlayerOrb)
		myPlayerOrb->SetActive(false);

	if (myBossOrb)
		myBossOrb->SetActive(false);

	if (myChargeVFX && myChargeVFX->IsPlaying())
	{
		myChargeVFX->Stop();
	}

	myState = State::Idle;
}

void LevelBossStates::GatlingEyeState::OnEnableAttack()
{
	if (!myBoss.IsAbilityUnlocked(LevelBossState::GatlingEye))
	{
		myAnimState = AnimState::Opening;
		myVars.myOverrideProgress = myVars.myOverrideDuration;

		myBoss.SetShowEye(true);
	}
}

void LevelBossStates::GatlingEyeState::OnDisableAttack()
{
	if (!myBoss.IsAbilityUnlocked(LevelBossState::GatlingEye))
	{
		myAnimState = AnimState::Closing;
		myVars.myOverrideProgress = myVars.myOverrideDuration;
	}
}

void LevelBossStates::GatlingEyeState::OnIdle(const Vec3f& aPlayerPosition)
{
	myLastPlayerPosition = aPlayerPosition;

	myVars.myTelegraphDuration = GetController().FindState("BASE - (Main)", "Gatling Start")->GetDurationSeconds();
	myVars.myTelegraphProgress = myVars.myTelegraphDuration;
	myState = State::AttackStart;

	TurnTowards(aPlayerPosition);
}

void LevelBossStates::GatlingEyeState::OnAttack(const Vec3f& aPlayerPosition)
{
	Telegraph();

	if (myState == State::AttackStart)
	{
		TurnTowards(myLastPlayerPosition);

		if (myVars.myTelegraphProgress <= 0.0f)
		{
			myVars.myTelegraphProgress = 0.0f;
			myVars.myFollowTargetProgress = myVars.myFollowTargetDuration;

			myState = State::AttackLoop;

			if (auto camera = myBoss.GetGameObject()->GetScene()->GetMainCameraGameObject())
			{
				if (auto camShake = camera->GetComponent<Engine::CameraShakeComponent>())
				{
					const float laserDurationSec = myVars.myFollowTargetDuration;
					myLaserShake.myIntensityDesc.mySustainIntensityDurationSeconds = laserDurationSec;
					camShake->AddPerlinShake(myLaserShake);
				}
			}
			GetAudio().StopEvent("GattlingEyeStart");
			GetAudio().PlayEvent("GattlingEyeLoop");
		}

		if (myBoss.GetHealth()->GetHealth() <= 0.0f)
		{
			myBoss.ChangeState(LevelBossState::Death);
		}
		return;
	}
	myVars.myFollowTargetProgress -= Time::DeltaTime;
	myVars.myTimeBetweenShotsProgress -= Time::DeltaTime;

	if (myVars.myTimeBetweenShotsProgress <= 0.0f)
	{
		Vec3f offsetPlayerPos = myLastPlayerPosition /*+ Vec3f(0, 90, 0)*/;
		Vec3f direction = (offsetPlayerPos - myEyePosition).GetNormalized();
		float distance = (offsetPlayerPos - myEyePosition).Length();

		Vec3f targetPosition = myEyePosition + direction * (distance + myVars.mySpread);
		targetPosition.x += Random::RandomFloat(-myVars.mySpread, myVars.mySpread);
		targetPosition.y += Random::RandomFloat(-myVars.mySpread, myVars.mySpread);
		targetPosition.z += Random::RandomFloat(-myVars.mySpread, myVars.mySpread);

		direction = (targetPosition - myEyePosition).GetNormalized();
		distance = (targetPosition - myEyePosition).Length();
		distance += 1000.f;

		RayCastHit hit;
		auto scene = myBoss.GetGameObject()->GetScene();
		if (scene->RayCast(myEyePosition, direction, FLT_MAX, eLayer::DEFAULT | eLayer::GROUND | eLayer::PLAYER_DAMAGEABLE, hit))
		{
			if (hit.GameObject->GetLayer() & (eLayer::DEFAULT | eLayer::GROUND))
			{
				if (myVars.myGatlingImpactVFXRef->IsValid())
				{
					if (auto go = myBoss.GetGameObject()->GetScene()->AddGameObject<GameObject>())
					{
						auto& transform = go->GetTransform();
						transform.SetPosition(hit.Position);
						if (auto vfx = go->AddComponent<Engine::VFXComponent>(myVars.myGatlingImpactVFXRef->Get()))
						{
							vfx->Play();
							vfx->AutoDestroy();
						}
					}
				}
			}
			else if (hit.GameObject->GetLayer() & eLayer::PLAYER_DAMAGEABLE)
			{
				if (auto pollingStation = myBoss.GetGameObject()->GetSingletonComponent<PollingStationComponent>())
				{
					if (auto gameObject = pollingStation->GetPlayer())
					{
						if (hit.GameObject == gameObject)
						{
							if (auto player = gameObject->GetComponent<HealthComponent>())
							{
								const float damage = Random::RandomFloat(myVars.myMinDamage, myVars.myMaxDamage);
								player->ApplyDamage(damage * myBoss.GetDmgMultiplier());
							}
						}
					}
				}
			}

			distance = (hit.Position - myEyePosition).Length() * 0.01f;
		}

		if (myVars.myGatlingLaserPrefab->IsValid())
		{
			Engine::GameObjectPrefab& prefab = myVars.myGatlingLaserPrefab->Get();

			auto scene = myBoss.GetGameObject()->GetScene();
			auto laser = &prefab.Instantiate(*scene);

			laser->GetTransform().SetPosition(myEyePosition);
			const float thickness = laser->GetTransform().GetScale().x;
			laser->GetTransform().SetScale({ thickness, thickness, distance });
			laser->GetTransform().LookAt(myEyePosition - direction);
		}

		myVars.myTimeBetweenShotsProgress = myVars.myTimeBetweenShotsDuration;
	}

	TurnTowards(myLastPlayerPosition);

	if (myVars.myFollowTargetProgress <= 0.0f)
	{
		GetAudio().StopEvent("GattlingEyeLoop");
		GetAudio().PlayEvent("GattlingEyeEnd");
		myVars.myFollowTargetProgress = 0.0f;
		myBoss.ChangeState(LevelBossState::Idle);
	}
}

void LevelBossStates::GatlingEyeState::Telegraph()
{
	myVars.myTelegraphProgress -= Time::DeltaTime;

	if (myChargeVFX && myChargeVFX->IsPlaying())
	{
		if (auto go = myChargeVFX->GetGameObject())
		{
			auto& transform = go->GetTransform();
			transform.SetPosition(myBoss.GetEye()->GetTransform().GetPosition());
		}
	}

	if (myVars.myTelegraphProgress <= 0.0f)
	{
		myVars.myTelegraphProgress = 0.0f;

		if (myChargeVFX)
		{
			myChargeVFX->Stop();
		}
	}

	const float ratio = 1.0f - (myVars.myTelegraphProgress / myVars.myTelegraphDuration);

	if (myPlayerOrb)
	{
		auto& transform = myPlayerOrb->GetTransform();
		transform.SetPosition(myLastPlayerPosition);
		if (!myPlayerOrb->IsActive())
			myPlayerOrb->SetActive(true);

		const float scale = CU::Lerp(4.0f, 1.0f, ratio);
		transform.SetScale(scale);
	}

	if (myBossOrb)
	{
		auto& transform = myBossOrb->GetTransform();
		transform.SetPosition(myEyePosition);
		if (!myBossOrb->IsActive())
			myBossOrb->SetActive(true);

		const float scale = CU::Lerp(40.0f, 4.5f, ratio);
		transform.SetScale(scale);
	}
}
