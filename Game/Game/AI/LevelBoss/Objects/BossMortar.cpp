#include "pch.h"
#include "BossMortar.h"

#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Prefab/GameObjectPrefab.h"
#include "Engine/GameObject/Components/VFXComponent.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"

#include "Engine/GameObject/Components/CameraShakeComponent.h"
#include "Engine/GameObject/Components/AudioComponent.h"

#include "Engine/Reflection/Reflector.h"

#include "AI/PollingStation/PollingStationComponent.h"

#include "BossCluster.h"

LevelBossStates::BossMortar::~BossMortar()
{
}

void LevelBossStates::BossMortar::Awake()
{
	myAudioComponent = myGameObject->AddComponent<Engine::AudioComponent>();
	if (myAudioComponent)
	{
		myAudioComponent->AddEvent("MortarExplosion", "event:/SFX/NPC/ProjectileExplode");
	}
	else
	{
		LOG_INFO(LogType::Game) << "MISSING AUDIOCOMPONENT";
	}
}

void LevelBossStates::BossMortar::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myExplosionVFXRef, "Explosion VFX");
	aReflector.Reflect(myClusterPrefab, "Cluster Prefab");
}

void LevelBossStates::BossMortar::Execute(Engine::eEngineOrder aOrder)
{
	if (myProgress > 0.0f)
	{
		myProgress -= Time::DeltaTime;

		if (myProgress <= 0.0f)
		{
			myProgress = 0.0f;
		}
	}
	const float ratio = 1.0f - (myProgress / myDuration);

	if (auto pollingStation = myGameObject->GetSingletonComponent<PollingStationComponent>())
	{
		Vec3f playerPosition = pollingStation->GetPlayerPos();

		Vec3f position = CU::Lerp(myStartPosition, playerPosition, ratio);
		position.y = myHeight * sinf(ratio * CU::PI) + myStartPosition.y;

		auto& transform = myGameObject->GetTransform();
		transform.SetPosition(position);
		transform.LookAt(position - (myLastFramePosition - position).GetNormalized());

		if (ratio >= 0.4f)
		{
			if (myClusterPrefab)
			{
				for (int i = 0; i < myClusterCount; i++)
				{
					Engine::GameObjectPrefab& mortarPrefab = myClusterPrefab->Get();
					auto scene = myGameObject->GetScene();
					GameObject& mortar = mortarPrefab.Instantiate(*scene);

					if (auto script = mortar.GetComponent<BossCluster>())
					{
						Vec3f targetPosition = playerPosition;
						targetPosition.x += Random::RandomFloat(-myOffsetSpread, myOffsetSpread);
						targetPosition.z += Random::RandomFloat(-myOffsetSpread, myOffsetSpread);

						RayCastHit hit;
						if (scene->RayCast(targetPosition, Vec3f(0, -1, 0), FLT_MAX, eLayer::DEFAULT | eLayer::GROUND, hit))
						{
							targetPosition = hit.Position;
						}

						float offset = Random::RandomFloat(1.0f - myOffsetLanding, 1.0f + myOffsetLanding);
						script->SetPositions(position, targetPosition, myHeight, myProgress * offset, myProgress * offset, ratio);
						script->SetClusterRadius(myClusterRadius);
						script->SetDamage(myDamage);

						if (myAudioComponent)
						{
							myAudioComponent->PlayEvent("MortarExplosion");
						}
					}
				}

				if (auto camera = myGameObject->GetScene()->GetMainCameraGameObject())
				{
					if (auto shake = camera->GetComponent<Engine::CameraShakeComponent>())
					{
						shake->AddPerlinShake("MortarExplosion");
					}
				}
			}

			if (myExplosionVFXRef->IsValid())
			{
				if (auto go = myGameObject->GetScene()->AddGameObject<GameObject>())
				{
					auto& transform = go->GetTransform();
					transform.SetPosition(myGameObject->GetTransform().GetPosition());

					if (auto vfx = go->AddComponent<Engine::VFXComponent>(myExplosionVFXRef->Get()))
					{
						vfx->Play();
						vfx->AutoDestroy();
					}
				}
			}

			myGameObject->Destroy();
			return;
		}

		myLastFramePosition = position;
	}
}

void LevelBossStates::BossMortar::SetPositions(const Vec3f& aStartPosition, const float aHeight, const float aTime)
{
	myLastFramePosition = aStartPosition;
	myStartPosition = aStartPosition;
	myHeight = aHeight;
	myProgress = myDuration = aTime;
}

void LevelBossStates::BossMortar::SetDamage(const float anAmount)
{
	myDamage = anAmount;
}

void LevelBossStates::BossMortar::SetClusterCount(const int aCount)
{
	myClusterCount = aCount;
}

void LevelBossStates::BossMortar::SetOffsetSpread(const float aSpread)
{
	myOffsetSpread = aSpread;
}

void LevelBossStates::BossMortar::SetOffsetLanding(const float aValue)
{
	myOffsetLanding = aValue;
}

void LevelBossStates::BossMortar::SetClusterRadius(const float aRadius)
{
	myClusterRadius = aRadius;
}
