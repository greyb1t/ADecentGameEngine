#include "pch.h"
#include "BodyGigaLaser.h"
#include "AI/FinalBoss/Constants.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "Engine/GameObject/Components/AudioComponent.h"
#include "AI/FinalBoss/FinalBossBody.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/ResourceManagement/Resources/GameObjectPrefabResource.h"
#include "Engine/GameObject/Prefab/GameObjectPrefab.h"
#include "AI/PollingStation/PollingStationComponent.h"
#include "Engine/CameraShake/CameraShakeManager.h"
#include "Engine/CameraShake/PerlinShake.h"
#include "Engine/GameObject/Components/CameraShakeComponent.h"
#include "Engine/GameObject/Components/ModelComponent.h"
#include "AI/FinalBoss/FinalBoss.h"
#include "Components/HealthComponent.h"
#include "Engine/GameObject/Components/VFXComponent.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"
#include "Engine/GameObject/Components/CameraComponent.h"
#include "Engine/GameObject/Components/PointLightComponent.h"

FB::BodyGigaLaser::BodyGigaLaser(FinalBossBody& aBody)
	: myBody(aBody)
{
	myBody.GetAnimator().GetController().AddEventCallback(
		"SpawnLaser", [this]()
		{
			myDamageElapsed = 0.f;
			myTotalDamageDone = 0.f;

			SpawnLaser();
			myBody.GetAudio()->PlayEvent("GigaLaser");
			myState = State::ShootingLaser;

			StopTelegraphImpactVFX();
			StopTelegraphMouthSuckVFX();
		});

	myBody.GetAnimator().GetController().AddEventCallback(
		"DestroyLaser", [this]()
		{
			DestroyLaser();
			myState = State::None;

			StopGigaLaserImpactVFX();
		});

	myBody.GetAnimator().GetController().AddStateOnExitCallback(
		"Base", "Laser", [this]() { OnFinishedAnimation(); });

	myBody.GetAnimator().GetController().AddStateOnExitCallback(
		"Base", "Laser Telegraph", [this]() { DestroyTelegraphLaser(); });

	myEyeBoneIndex = myBody.GetAnimator().GetController().GetBoneIndexFromName(Constants::GigaLaserSpawnPosBoneName);

	auto& shakeManager = GetEngine().GetCameraShakeManager();
	myLaserShake = shakeManager.GetPerlinShake("FinalBossGigaLaser");
}

void FB::BodyGigaLaser::Update()
{
	if (myOutputFlashPointlight)
	{
		const float intensity = myOutputFlashPointlight->GetIntensity();
		const float lightDecreaseSpeed = 20.f;
		const float newIntensity = Math::Lerp(intensity, 0.f, Time::DeltaTime * lightDecreaseSpeed);
		myOutputFlashPointlight->SetIntensity(std::max(newIntensity, 0.f));
	}

	if (myImpactFlashPointlight)
	{
		const float intensity = myImpactFlashPointlight->GetIntensity();
		const float lightDecreaseSpeed = 20.f;
		const float newIntensity = Math::Lerp(intensity, 0.f, Time::DeltaTime * lightDecreaseSpeed);
		myImpactFlashPointlight->SetIntensity(std::max(newIntensity, 0.f));
	}

	// Desto närmare spelaren, desto snabbare speed
	const auto targetPos = myBody.GetFinalBoss().GetPlayerPos();
	myCurrentTargetPosition = targetPos;
	/*
	float d = (targetPos - myCurrentTargetPosition).Length();

	d = 200.f - d;
	d = std::max(d, 200.f);

	const float f = Time::DeltaTime * (d + 0.01f);
	myCurrentTargetPosition = Math::LerpClamped(myCurrentTargetPosition, targetPos, f);
	*/

	switch (myState)
	{
	case State::Telegraphing:
	{
		// At the end of telegraph, if player is behind pillar, focus on it and destroy it with laser
		// If not, shoot the player always aims directly at player using raycast

		if (myTelegraphLaser && myTelegraphModel)
		{
			PointTelegraphLaserAtPlayer();

			myTelegraphElapsed += Time::DeltaTime;

			// From seconds 0-1, change the x variable
			const float percent1 = Math::InverseLerp(0.f, 1.f, myTelegraphElapsed);
			myTelegraphVars.x = Math::Lerp(-100.f, 100.f, percent1);

			// From seconds 1-2, change the y variable
			const float percent2 = Math::InverseLerp(1.f, 2.f, myTelegraphElapsed);
			myTelegraphVars.y = std::clamp(Math::Lerp(35.f, 135.f, percent2), 35.f, 135.f);

			auto& matInstance = myTelegraphModel->GetMeshMaterialInstanceByIndex(0);

			Vec4f vars = matInstance.GetFloat4("myVar");
			vars.x = myTelegraphVars.x;
			vars.y = myTelegraphVars.y;

			LOG_INFO(LogType::Filip) << "Vars.x: " << vars.x << ", Vars.y: " << vars.y;

			matInstance.SetFloat4("myVar", vars);
		}

	} break;
	case State::ShootingLaser:
	{
		if (myLaser)
		{
			PointLaserAtPosition(myCurrentTargetPosition);

			UpdateLaserDamageDots();
		}
	} break;

	case State::None:
	{
	} break;

	default:
		break;
	}
}

bool FB::BodyGigaLaser::IsFinished() const
{
	return myIsFinished;
}

void FB::BodyGigaLaser::OnEnter()
{
	myIsFinished = false;
	myBody.GetAnimator().GetController().Trigger(Constants::LaserTriggerName);
	myState = State::Telegraphing;

	myTotalDamage = Desc().myTotalDamage.GetDamage();

	SpawnTelegraphLaser();

	myTotalDamageDone = 0.f;

	myCurrentTargetPosition = myBody.GetFinalBoss().GetPlayerPos();

	auto polling = myBody.GetFinalBoss().GetGameObject()->GetSingletonComponent<PollingStationComponent>();

	myOutputFlashPointlightObj = polling->GetPointlightFromPool();
	myOutputFlashPointlight = myOutputFlashPointlightObj->GetComponent<Engine::PointLightComponent>();

	myImpactFlashPointlightObj = polling->GetPointlightFromPool();
	myImpactFlashPointlight = myImpactFlashPointlightObj->GetComponent<Engine::PointLightComponent>();
}

void FB::BodyGigaLaser::OnExit()
{
	DestroyLaser();
	DestroyTelegraphLaser();

	StopTelegraphImpactVFX();
	StopTelegraphMouthSuckVFX();
	StopGigaLaserImpactVFX();

	auto pollingStation = myBody.GetFinalBoss().GetGameObject()->GetSingletonComponent<PollingStationComponent>();

	// Returns point light
	pollingStation->ReturnPointLightToPool(myOutputFlashPointlightObj);
	myOutputFlashPointlightObj = nullptr;
	myOutputFlashPointlight = nullptr;

	pollingStation->ReturnPointLightToPool(myImpactFlashPointlightObj);
	myImpactFlashPointlightObj = nullptr;
	myImpactFlashPointlight = nullptr;
}

void FB::BodyGigaLaser::Reflect(Engine::Reflector& aReflector)
{
}

void FB::BodyGigaLaser::SpawnLaser()
{
	myGigaLaserImpactVFX = nullptr;

	if (Desc().myLaserPrefab && Desc().myLaserPrefab->IsValid())
	{
		myLaser = &Desc().myLaserPrefab->Get().Instantiate(*myBody.GetGameObject()->GetScene());

		StartShake();
	}
	else
	{
		LOG_ERROR(LogType::Game) << "GigaLaser missing prefab";
	}

	// Spawn the giga lase rimpact VFX
	if (Desc().myGigaLaserImpactVFX && Desc().myGigaLaserImpactVFX->IsValid())
	{
		auto g = myBody.GetGameObject()->GetScene()->AddGameObject<GameObject>();
		myGigaLaserImpactVFXObj = g->GetTransform().GetGameObjectShared();
		g->GetTransform().SetPosition(myBody.GetTransform().GetPosition());
		myGigaLaserImpactVFX = g->AddComponent<Engine::VFXComponent>(Desc().myGigaLaserImpactVFX->Get());
		myGigaLaserImpactVFX->Play();
		myGigaLaserImpactVFX->AutoDestroy();
	}
	else
	{
		LOG_ERROR(LogType::Game) << "Missing giga laser impact VFX";
	}
}

void FB::BodyGigaLaser::OnFinishedAnimation()
{
	myIsFinished = true;
}

void FB::BodyGigaLaser::SpawnTelegraphLaser()
{
	myImpactVFX = nullptr;

	if (Desc().myTelegraphLaserPrefab && Desc().myTelegraphLaserPrefab->IsValid())
	{
		myTelegraphLaser = &Desc().myTelegraphLaserPrefab->Get().Instantiate(*myBody.GetGameObject()->GetScene());

		myTelegraphModel = myTelegraphLaser->GetComponent<Engine::ModelComponent>();

		assert(myTelegraphModel);

		PointTelegraphLaserAtPlayer();

		myTelegraphElapsed = 0.f;
	}
	else
	{
		LOG_ERROR(LogType::Game) << "GigaLaser missing telegraph prefab";
	}

	// Spawn the impact VFX
	if (Desc().myTelegraphImpactVFX && Desc().myTelegraphImpactVFX->IsValid())
	{
		auto g = myBody.GetGameObject()->GetScene()->AddGameObject<GameObject>();
		myImpactVFXObj = g->GetTransform().GetGameObjectShared();
		g->GetTransform().SetPosition(myBody.GetTransform().GetPosition());
		myImpactVFX = g->AddComponent<Engine::VFXComponent>(Desc().myTelegraphImpactVFX->Get());
		myImpactVFX->Play();
		myImpactVFX->AutoDestroy();
	}
	else
	{
		LOG_ERROR(LogType::Game) << "Missing telegraph impact VFX";
	}

	myMouthSuckVFX = nullptr;

	// Spawn the MOUTH SUCK VFX
	if (Desc().myTelegraphMouthSuckVFX && Desc().myTelegraphMouthSuckVFX->IsValid())
	{
		auto g = myBody.GetGameObject()->GetScene()->AddGameObject<GameObject>();
		myMouthSuckVFXObj = g->GetTransform().GetGameObjectShared();
		// g->GetTransform().SetPosition(myBody.GetTransform().GetPosition());
		myMouthSuckVFX = g->AddComponent<Engine::VFXComponent>(Desc().myTelegraphMouthSuckVFX->Get());
		myMouthSuckVFX->Play();
	}
	else
	{
		LOG_ERROR(LogType::Game) << "Missing telegraph impact VFX";
	}
}

void FB::BodyGigaLaser::PointTelegraphLaserAtPlayer()
{
	if (myEyeBoneIndex != -1)
	{
		const Mat4f transform =
			myBody.GetAnimator().GetController().GetBoneTransformWorld(myEyeBoneIndex);

		Vec3f t;
		Vec3f s;
		Quatf r;
		transform.Decompose(t, r, s);

		myTelegraphLaser->GetTransform().SetPosition(t);
		myTelegraphLaser->GetTransform().LookAt(myCurrentTargetPosition);
		// bcuz model forward is backwards
		auto r2 = myTelegraphLaser->GetTransform().GetRotation();
		r2 = r2 * Quatf(Vec3f(0.f, Math::PI, 0.f));
		myTelegraphLaser->GetTransform().SetRotation(r2);

		// auto scale = myTelegraphLaser->GetTransform().GetScale();
		// scale.z = 100.f;
		// myTelegraphLaser->GetTransform().SetScale(scale);
	}
	else
	{
		LOG_ERROR(LogType::Game) << "GigaLaser cannot find telegraph laser spawn bone";
	}

	if (!myMouthSuckVFXObj.expired() && myMouthSuckVFX)
	{
		if (myEyeBoneIndex != -1)
		{
			const Mat4f transform =
				myBody.GetAnimator().GetController().GetBoneTransformWorld(myEyeBoneIndex);

			Vec3f t;
			Vec3f s;
			Quatf r;
			transform.Decompose(t, r, s);

			myMouthSuckVFX->GetGameObject()->GetTransform().SetPosition(t);
		}
		else
		{
			LOG_ERROR(LogType::Game) << "GigaLaser cannot find laser 'Eye' bone";
		}
	}

	if (!myImpactVFXObj.expired() && myImpactVFX)
	{
		RayCastHit hit;

		const auto origin = myTelegraphLaser->GetTransform().GetPosition();
		const auto dir = (myCurrentTargetPosition - origin).GetNormalized();

		const bool hitSomething = myBody.GetFinalBoss().GetGameObject()->GetScene()->RayCast(
			origin,
			dir,
			Constants::RaycastDistance,
			eLayer::PLAYER_DAMAGEABLE | eLayer::ENVIRONMENT | eLayer::DEFAULT | eLayer::GROUND,
			hit);

		if (hitSomething)
		{
			// find first hit that is not the final boss and that is the impact point
			//for (int i = static_cast<int>(hits.size()) - 1; i >= 0; --i)
			{
				if (hit.GameObject->GetTag() != eTag::FINALBOSS)
				{
					myImpactVFX->GetGameObject()->GetTransform().SetPosition(hit.Position);

					auto scale = myTelegraphLaser->GetTransform().GetScale();
					scale.z = hit.Distance / 100.f;
					myTelegraphLaser->GetTransform().SetScale(scale);
				}
			}
		}
		else
		{
			LOG_ERROR(LogType::Game) << "raycast failed to telegraph impact point";
		}
	}
}

void FB::BodyGigaLaser::DestroyTelegraphLaser()
{
	if (myTelegraphLaser)
	{
		myTelegraphLaser->Destroy();
		myTelegraphLaser = nullptr;
		myTelegraphModel = nullptr;
	}
}

void FB::BodyGigaLaser::PointLaserAtPosition(const Vec3f& aTargetPosition)
{
	if (myEyeBoneIndex != -1)
	{
		const Mat4f transform =
			myBody.GetAnimator().GetController().GetBoneTransformWorld(myEyeBoneIndex);

		Vec3f t;
		Vec3f s;
		Quatf r;
		transform.Decompose(t, r, s);

		myLaser->GetTransform().SetPosition(t);
		myLaser->GetTransform().LookAt(aTargetPosition);
		auto r2 = myLaser->GetTransform().GetRotation();
		r2 = r2 * Quatf(Vec3f(0.f, Math::PI * 0.5f, 0.f));
		myLaser->GetTransform().SetRotation(r2);

		// Spawn light as laser output
		if (myOutputFlashPointlightObj && myOutputFlashPointlight)
		{
			myOutputFlashPointlightObj->SetActive(true);
			myOutputFlashPointlightObj->GetTransform().SetPosition(t + Vec3f(0.f, -1.f, 0.f) * 1000.f);

			myOutputFlashPointlight->SetColor(Vec3f(0.578f, 0.279f, 0.914f));
			myOutputFlashPointlight->SetRange(5000.f);
			myOutputFlashPointlight->SetIntensity(50000000.000);
		}
		else
		{
			LOG_ERROR(LogType::Game) << "giga laser point light is null";
		}

		// auto scale = myLaser->GetTransform().GetScale();
		// scale.x = 1.f;
		// myLaser->GetTransform().SetScale(scale);
	}
	else
	{
		LOG_ERROR(LogType::Game) << "GigaLaser cannot find laser spawn bone";
	}

	if (!myGigaLaserImpactVFXObj.expired() && myGigaLaserImpactVFX)
	{
		RayCastHit hit;

		const auto origin = myLaser->GetTransform().GetPosition();
		const auto dir = (myCurrentTargetPosition - origin).GetNormalized();

		const bool hitSomething = myBody.GetFinalBoss().GetGameObject()->GetScene()->RayCast(
			origin,
			dir,
			Constants::RaycastDistance,
			eLayer::PLAYER_DAMAGEABLE | eLayer::ENVIRONMENT | eLayer::DEFAULT | eLayer::GROUND,
			hit);

		if (hitSomething)
		{
			// find first hit that is not the final boss and that is the impact point
			//for (int i = static_cast<int>(hits.size()) - 1; i >= 0; --i)
			{
				if (hit.GameObject->GetTag() != eTag::FINALBOSS)
				{
					myGigaLaserImpactVFX->GetGameObject()->GetTransform().SetPosition(hit.Position);

					const float distanceMeter = hit.Distance / 100.f;

					auto scale = myLaser->GetTransform().GetScale();
					scale.x = (1.f / 40.f / 1.97f) * distanceMeter;
					myLaser->GetTransform().SetScale(scale);

					// spawn point light
					if (myImpactFlashPointlightObj && myImpactFlashPointlight)
					{
						myImpactFlashPointlightObj->SetActive(true);
						myImpactFlashPointlightObj->GetTransform().SetPosition(hit.Position);

						myImpactFlashPointlight->SetColor(Vec3f(0.578f, 0.279f, 0.914f));
						myImpactFlashPointlight->SetRange(2500.f);
						myImpactFlashPointlight->SetIntensity(5000000.f);
					}
					else
					{
						LOG_ERROR(LogType::Game) << "giga laser point light is null";
					}


					//break;
				}
			}
		}
		else
		{
			LOG_ERROR(LogType::Game) << "raycast failed to giga laser impact point";
		}
	}
}

void FB::BodyGigaLaser::DestroyLaser()
{
	if (myLaser)
	{
		myLaser->Destroy();
		myLaser = nullptr;
	}
}

float FB::BodyGigaLaser::GetLaserDuration() const
{
	return 3.4f;
}

float FB::BodyGigaLaser::GetLaserDotWaitTime() const
{
	return GetLaserDuration() / static_cast<float>(Desc().myTotalDamageTicks);
}

void FB::BodyGigaLaser::UpdateLaserDamageDots()
{
	if (myTotalDamageDone >= myTotalDamage)
	{
		return;
	}

	myDamageElapsed += Time::DeltaTime;

	if (myDamageElapsed > GetLaserDotWaitTime())
	{
		myDamageElapsed -= GetLaserDotWaitTime();

		DoDamage();
	}
}

const FB::GigaLaserDesc& FB::BodyGigaLaser::Desc() const
{
	return myBody.GetGigaLaserDesc();
}

void FB::BodyGigaLaser::StopTelegraphImpactVFX()
{
	// Stop the telegraph impact
	if (!myImpactVFXObj.expired() && myImpactVFX)
	{
		myImpactVFX->Stop();
		myImpactVFX->GetGameObject()->Destroy();
		myImpactVFXObj = {};
		myImpactVFX = nullptr;
	}
}

void FB::BodyGigaLaser::StopTelegraphMouthSuckVFX()
{
	if (!myMouthSuckVFXObj.expired() && myMouthSuckVFX)
	{
		myMouthSuckVFX->Stop();
		myMouthSuckVFX->GetGameObject()->Destroy();
		myMouthSuckVFXObj = {};
		myMouthSuckVFX = nullptr;
	}
}

void FB::BodyGigaLaser::StopGigaLaserImpactVFX()
{
	// Stop the giga laser impact
	if (!myGigaLaserImpactVFXObj.expired() && myGigaLaserImpactVFX)
	{
		myGigaLaserImpactVFX->Stop();
		myGigaLaserImpactVFX->GetGameObject()->Destroy();
		myGigaLaserImpactVFXObj = {};
		myGigaLaserImpactVFX = nullptr;
	}
}

void FB::BodyGigaLaser::DoDamage()
{
	LOG_INFO(LogType::Filip) << "DO DAMAGE";

	const auto from = myLaser->GetTransform().GetPosition();
	const auto to = (myCurrentTargetPosition - from).GetNormalized();

	RayCastHit hit;
	const bool hitSomething = myBody.GetGameObject()->GetScene()->RayCast(
		from,
		to,
		Constants::RaycastDistance,
		eLayer::PLAYER_DAMAGEABLE | eLayer::ENVIRONMENT | eLayer::DEFAULT | eLayer::GROUND,
		hit);

	GameObject* player = myBody.GetFinalBoss().GetPlayer();

	if (hitSomething)
	{
		if (hit.GameObject == player)
		{
			if (auto health = player->GetComponent<HealthComponent>())
			{
				const float finalDamage = myTotalDamage / static_cast<float>(Desc().myTotalDamageTicks);

				health->ApplyDamage(finalDamage);

				myTotalDamageDone += finalDamage;

				LOG_INFO(LogType::Game) << "Giga laser damaged player";
			}
		}
	}
}

void FB::BodyGigaLaser::StartShake()
{
	auto mainCam = myBody.GetGameObject()->GetScene()->GetMainCameraGameObject();

	auto camShake = mainCam->GetComponent<Engine::CameraShakeComponent>();
	if (camShake)
	{
		const float laserDurationSec = GetLaserDuration();
		myLaserShake.myIntensityDesc.mySustainIntensityDurationSeconds = laserDurationSec;
		camShake->AddPerlinShake(myLaserShake);
	}
}
