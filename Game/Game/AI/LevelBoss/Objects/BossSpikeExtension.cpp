#include "pch.h"
#include "BossSpikeExtension.h"

#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "Engine/GameObject/Components/Collider.h"
#include "Engine/GameObject/Components/RigidBodyComponent.h"
#include "Engine/GameObject/Components/CameraShakeComponent.h"
#include "Engine/GameObject/Components/AudioComponent.h"
#include "Components/HealthComponent.h"

void BossSpikeExtension::Awake()
{
	if (!(myGameObjectWithCollider.Get()))
	{
		LOG_ERROR(LogType::Game) << "Could not find a GameObjectRef on Boss Spike Extension. Removing Object";
		myGameObject->Destroy();
		return;
	}

	if (!(myAnimator = myGameObject->GetComponent<Engine::AnimatorComponent>()))
	{
		LOG_ERROR(LogType::Game) << "Could not find an animator component on Boss Spike Extension. Removing Object";
		myGameObject->Destroy();
		return;
	}

	if (!(myCollider = myGameObjectWithCollider.Get()->GetComponent<Engine::Collider>()))
	{
		LOG_ERROR(LogType::Game) << "Could not find a collider component on Boss Spike Extension. Removing Object";
		myGameObject->Destroy();
		return;
	}

	myAudioComponent = myGameObject->AddComponent<Engine::AudioComponent>();
	if (myAudioComponent)
	{
		myAudioComponent->AddEvent("SpikeAttack", "event:/SFX/NPC/LEVEL BOSS/SpikeAttack");
	}
	else
	{
		LOG_INFO(LogType::Game) << "MISSING AUDIOCOMPONENT";
	}
}

void BossSpikeExtension::Start()
{
	if (myGameObject->IsDestroyed())
		return;

	myAnimator->GetController().AddEventCallback("DisableCollision", 
		[this]() 
		{
			auto rb = myGameObject->GetComponent<Engine::RigidBodyComponent>();
			if (rb)
			{
				rb->Destroy();
			}
		});

	myAnimator->GetController().AddEventCallback("Despawn", [this]() {
		myGameObject->Destroy();
		});

	if (auto camera = myGameObject->GetScene()->GetMainCameraGameObject())
	{
		if (auto shake = camera->GetComponent<Engine::CameraShakeComponent>())
		{
			myAnimator->GetController().AddEventCallback("Screenshake_Spike", [shake]() {
				shake->AddPerlinShake("LevelBoss_SpikeExtension");
				});

			if (myAudioComponent)
			{
				myAudioComponent->PlayEvent("SpikeAttack");
			}
		}
	}

	auto rb = myGameObjectWithCollider.Get()->GetComponent<Engine::RigidBodyComponent>();
	rb->ObserveTriggerEnter(
		[this] (GameObject* anOther) 
		{
			if (myHasHitPlayer)
				return;

			if (anOther->GetTag() == eTag::PLAYER)
			{
				if (auto health = anOther->GetComponent<HealthComponent>())
				{
					const float damage = Random::RandomFloat(myMinDamage, myMaxDamage);
					health->ApplyDamage(damage);
					health->ApplyKnockback(Vec3f(0, 1, 0) * myForce);
				}

				auto rb = myGameObject->GetComponent<Engine::RigidBodyComponent>();
				if (rb)
				{
					rb->Destroy();
				}

				myHasHitPlayer = true;
			}
		});

	

	myGameObject->SetLayers(eLayer::NONE, eLayer::NONE, eLayer::PLAYER_DAMAGEABLE);
	myHasHitPlayer = false;
}

void BossSpikeExtension::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myGameObjectWithCollider, "GameObject With Collider");
}

void BossSpikeExtension::SetDamage(const float aMin, const float aMax)
{
	myMinDamage = aMin;
	myMaxDamage = aMax;
}

void BossSpikeExtension::SetForce(const float aForce)
{
	myForce = aForce;
}
