#include "pch.h"
#include "BossCluster.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"
#include "Engine/GameObject/Components/VFXComponent.h"
#include "Engine/GameObject/Components/CameraShakeComponent.h"
#include "Engine/GameObject/Components/AudioComponent.h"
#include "Engine/GameObject/GameObject.h"
#include "AI/PollingStation/PollingStationComponent.h"
#include "Components/HealthComponent.h"
#include "Engine/Reflection/Reflector.h"

#include "Engine/GameObject/GameObjectRef.h"
#include "Engine/GameObject/Prefab/GameObjectPrefab.h"

LevelBossStates::BossCluster::~BossCluster()
{
}

void LevelBossStates::BossCluster::Awake()
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

void LevelBossStates::BossCluster::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myExplosionVFXRef, "Explosion VFX");
	aReflector.Reflect(myTelegraphPrefab, "Telegraph prefab");
}

void LevelBossStates::BossCluster::Execute(Engine::eEngineOrder aOrder)
{
	if (myProgress > 0.0f)
	{
		myProgress -= Time::DeltaTime;

		if (myProgress <= 0.0f)
		{
			myProgress = 0.0f;
			Explode();
			myGameObject->Destroy();
			myTelegraphObject->Destroy();
			return;
		}
	}
	const float ratio = 1.0f - (myProgress / myDuration);

	Vec3f position = CU::Lerp(myStartPosition, myEndPosition, ratio);
	position.y = position.y + myHeight * sinf(ratio * CU::PI);

	myGameObject->GetTransform().SetPosition(position);
	auto& transform = myGameObject->GetTransform();
	transform.SetPosition(position);
	transform.LookAt(position - (myLastFramePosition - position).GetNormalized());

	if(!myTelegraphObject)
	{
		myTelegraphObject = &myTelegraphPrefab->Get().Instantiate(*GetGameObject()->GetScene());
		// +5 to avoid Z fighting with ground
		myTelegraphObject->GetTransform().SetPosition(myEndPosition + Vec3f(0.f, 5.f, 0.f));
	}
	else
	{
		const auto& children = myTelegraphObject->GetTransform().GetChildren();

		if(children.size())
		{
			children[0]->SetScale((ratio - myStartRatio) * (2.f - myStartRatio));
		}
	}
	myLastFramePosition = transform.GetPosition();
}

void LevelBossStates::BossCluster::SetPositions(const Vec3f& anStartPosition, const Vec3f& anEndPosition, const float aHeight, const float aDuration, const float aProgress, const float aStartRatio)
{
	myGameObject->GetTransform().SetPosition(anStartPosition);

	myLastFramePosition = anStartPosition;
	myStartPosition = anStartPosition;
	myEndPosition = anEndPosition;
	myHeight = aHeight;
	myDuration = aDuration;
	myProgress = aProgress;
	myStartRatio = aStartRatio;
}

void LevelBossStates::BossCluster::SetClusterRadius(const float aRadius)
{
	myClusterRadius = aRadius;
}

void LevelBossStates::BossCluster::SetDamage(const float anAmount)
{
	myDamage = anAmount;
}

void LevelBossStates::BossCluster::Explode()
{
	std::vector<OverlapHit> hits;
	if (auto pollingStation = myGameObject->GetSingletonComponent<PollingStationComponent>())
	{
		if (GameObject* gameObject = pollingStation->GetPlayer())
		{
			auto scene = myGameObject->GetScene();
			if (scene->SphereCastAll(myEndPosition, myClusterRadius, eLayer::PLAYER_DAMAGEABLE, hits))
			{
				for (auto& hit : hits)
				{
					if (hit.GameObject == gameObject)
					{
						if (auto health = gameObject->GetComponent<HealthComponent>())
						{
							health->ApplyDamage(myDamage);
						}
					}
				}
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
	if (myAudioComponent)
	{
		myAudioComponent->PlayEvent("MortarExplosion");
	}

	if (auto camera = myGameObject->GetScene()->GetMainCameraGameObject())
	{
		if (auto shake = camera->GetComponent<Engine::CameraShakeComponent>())
		{
			shake->AddPerlinShake("ClusterExplosion");
		}
	}
}
