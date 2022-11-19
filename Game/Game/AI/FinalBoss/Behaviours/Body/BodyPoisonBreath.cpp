#include "pch.h"
#include "BodyPoisonBreath.h"
#include "AI/FinalBoss/Constants.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "AI/FinalBoss/FinalBossBody.h"
#include "Engine/GameObject/GameObject.h"
#include "AI/PollingStation/PollingStationComponent.h"
#include "Components/HealthComponent.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"
#include "Engine/GameObject/Components/VFXComponent.h"
#include "Engine/GameObject//Components/AudioComponent.h"
#include "AI/FinalBoss/FinalBossBody.h"
#include "AI/FinalBoss/FinalBoss.h"
#include "Game/AI/FinalBoss/Behaviours/Body/PoisonCloudDamageComponent.h"

FB::BodyPoisonBreath::BodyPoisonBreath(FinalBossBody& aBody)
	: myBody(aBody)
{
	myBody.GetAnimator().GetController().AddEventCallback("AttackStart", [this]()
		{
			myShouldDealDamage = true;
			if (myVFXComponent)
			{
				myVFXComponent->Play();
			}
			else
			{
				LOG_INFO(LogType::Game) << "Poison cloud vfx is null";
			}
		});

	myBody.GetAnimator().GetController().AddEventCallback("AttackEnd", [this]()
		{
			myShouldDealDamage = false;
			if (myVFXComponent)
			{
				myVFXComponent->Stop();
			}
		});

	myBody.GetAnimator().GetController().AddStateOnExitCallback("Base", "Poison",
		[this]()
		{
			myIsFinished = true;
		});


	CreatePoisonVFX();
}

void FB::BodyPoisonBreath::Update()
{
	AttackUpdate();

	for (auto& info : myCloudPositions)
	{
		info.myDuration += Time::DeltaTime;

		if (info.myDuration > 0.5f)
		{
			if (!info.myHasSpawnedCloud)
			{
				if (Desc().myCloudVFX->IsValid())
				{
					GameObject* object = myBody.GetGameObject()->GetScene()->AddGameObject<GameObject>();
					object->GetTransform().SetPosition(info.myPos);
					Engine::VFXComponent* comp = object->AddComponent<Engine::VFXComponent>(Desc().myCloudVFX->Get());
					comp->Play();
					comp->AutoDestroy();
					object->AddComponent<PoisonCloudDamageComponent>();
				}
				info.myHasSpawnedCloud = true;
			}
		}
	}
}

bool FB::BodyPoisonBreath::IsFinished() const
{
	return myIsFinished;
}

void FB::BodyPoisonBreath::OnEnter()
{
	LOG_INFO(LogType::William) << "Poison cloud enter";

	myIsFinished = false;
	myStartDelayProgress = 0.0f;
	myCloudPositions.clear();
	myDmgTickProgress = 0.0f;
	myCloudSpawnProgress = 0.0f;
	myShouldDealDamage = false;

	auto& spawnObject = Desc().mySpawnPoint;
	if (!spawnObject.Get())
	{
		LOG_ERROR(LogType::Game) << "spawn object is null";
		return;
	}

	myBody.SetShouldRotateTowardsPlayer(false);
	myBody.GetAnimator().GetController().Trigger("Poison");

	PoisonBreathSound(true);
}

void FB::BodyPoisonBreath::OnExit()
{
	LOG_INFO(LogType::William) << "Poison cloud exit";

	myBody.SetShouldRotateTowardsPlayer(true);

	if (myVFXComponent)
	{
		myVFXComponent->Stop();
	}
	else
	{
		LOG_INFO(LogType::Game) << "Poison cloud vfx is null";
	}

	PoisonBreathSound(false);
}

void FB::BodyPoisonBreath::Reflect(Engine::Reflector& aReflector)
{
}

void FB::BodyPoisonBreath::AttackUpdate()
{
	auto* pollingStation = myBody.GetGameObject()->GetSingletonComponent<PollingStationComponent>();

	auto& spawnObject = Desc().mySpawnPoint;
	if (!spawnObject.Get())
	{
		LOG_ERROR(LogType::Game) << "spawn object is null";
		return;
	}

	if (!myShouldDealDamage)
		return;

	Vec3f startPos = myBody.GetTransform().GetPosition();
	startPos = spawnObject.Get()->GetTransform().GetPosition();

	Vec3f spawnDirection = spawnObject.Get()->GetTransform().Forward() * -1.f;


	if (myVFXObject)
	{
		myVFXObject->GetTransform().SetPosition(startPos);
		myVFXObject->GetTransform().LookAt(startPos + spawnDirection);
	}

	float midAngle = std::atan2(spawnDirection.z, spawnDirection.x);
	float minAngle = midAngle - Desc().myAngle * 0.5f * C::DegToRad;
	float maxAngle = midAngle + Desc().myAngle * 0.5f * C::DegToRad;

	Vec3f playerDir = (pollingStation->GetPlayerPos() - startPos);
	float playerDistance = playerDir.LengthSqr();

	playerDir.Normalize();
	float playerAngle = std::atan2(playerDir.z, playerDir.x);

	if (playerAngle < maxAngle && playerAngle > minAngle && playerDistance < Desc().myLength * Desc().myLength)
	{
		myDmgTickProgress += Time::DeltaTime;
		if (myDmgTickProgress > 1.f / Desc().myDamageTicksPerSecond)
		{
			myDmgTickProgress = 0.f;
			pollingStation->GetPlayer()->GetComponent<HealthComponent>()->ApplyDamage(Desc().myTickDamage.GetDamage(), false, nullptr, eDamageType::Poison);
		}
	}

	myCloudSpawnProgress += Time::DeltaTime;
	if (myCloudSpawnProgress > 1.f / Desc().myCloudsPerSecond)
	{
		myCloudSpawnProgress = 0.f;
		SpawnCloud();
	}

	// Visualization.
	if (Desc().myVisualizeHitbox)
	{
		int rayCount = (int)Desc().myAngle * 10.f;
		for (int i = 0; i < rayCount; ++i)
		{
			float angle = C::Lerp(minAngle, maxAngle, (float)i / (float)rayCount);
			Vec3f dir = Vec3f(cos(angle), 0.0f, sin(angle));

			GDebugDrawer->DrawLine3D(DebugDrawFlags::Always, startPos, startPos + dir * Desc().myLength, 0.0f, Vec4f(1, 0, 0, 1));
		}
	}
}

void FB::BodyPoisonBreath::SpawnCloud()
{
	auto* spawnObject = Desc().mySpawnPoint.Get();
	RayCastHit hit;

	Vec3f dir = spawnObject->GetTransform().Forward() * -1.f;
	//dir.y += 0.7f;

	//GDebugDrawer->DrawLine3D(DebugDrawFlags::Always, spawnObject->GetTransform().GetPosition(), spawnObject->GetTransform().GetPosition() + dir.GetNormalized() * 100000.f, 1);
	//GDebugDrawer->DrawLine3D(DebugDrawFlags::Always, spawnObject->GetTransform().GetPosition(), spawnObject->GetTransform().GetPosition() + spawnObject->GetTransform().Up() * 10000.f, 1, { 1, 0, 0, 1 });
	if (myBody.GetGameObject()->GetScene()->RayCast(spawnObject->GetTransform().GetPosition(), dir, 10000, eLayer::GROUND | eLayer::DEFAULT, hit))
	{
		myCloudPositions.push_back({ hit.Position, 0.0f });
	}
}

void FB::BodyPoisonBreath::PoisonBreathSound(const bool aEnable) const
{
	if (aEnable)
	{
		myBody.GetAudio()->PlayEvent("PoisonBreath");
	}
	else
	{
		myBody.GetAudio()->StopEvent("PoisonBreath");
	}
}

const FB::PoisonCloudDesc& FB::BodyPoisonBreath::Desc() const
{
	return myBody.GetPoisonCloudDesc();
}

void FB::BodyPoisonBreath::CreatePoisonVFX()
{
	myVFXObject = myBody.GetGameObject()->GetScene()->AddGameObject<GameObject>();

	if (Desc().myBreathParticle->IsValid())
	{
		myVFXComponent = myVFXObject->AddComponent<Engine::VFXComponent>(Desc().myBreathParticle->Get());
	}
}
