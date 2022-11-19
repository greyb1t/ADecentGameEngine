#include "pch.h"
#include "BouncingDamageComponent.h"
#include "Engine/GameObject/GameObject.h"
#include "Game/Components/HealthComponent.h"
#include "Engine/GameObject/Components/ModelComponent.h"
#include "Engine/GameObject/Components/AudioComponent.h"
#include "Engine/GameObject/Components/VFXComponent.h"
#include "Engine/GameObject/Prefab/GameObjectPrefab.h"
#include "Engine/VFX/VFXUtilities.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"
#include "Engine/Utils/TimerManager.h"

constexpr float myHeightOffset = 70.f;

void BouncingDamageComponent::Init(Transform* aTarget, int someTargets, float aRadius, float aDamage, float aDelay, float aVFXDuration)
{
	myGameObject->SetName("Chain Lightning");
	myStartPosition = aTarget->GetPosition() + CU::Vector3f{ 0.f, myHeightOffset, 0.f };
	GetTransform().SetPosition(myStartPosition);

	myRemainingTargets = someTargets;

	myRadius = aRadius;
	myDamage = aDamage;
	myDelay = aDelay;
	myLightningDuration = aVFXDuration;

	auto audio = myGameObject->AddComponent<Engine::AudioComponent>();
	audio->AddEvent("Chain", "event:/SFX/PLR/Pickups/ChainLightning");

	Chain();
}

void BouncingDamageComponent::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myImpactEffect, "Impact Effect");
	aReflector.Reflect(myLightningVFX, "Lightning VFX");
}

void BouncingDamageComponent::Execute(Engine::eEngineOrder aOrder)
{
	if (auto shared = myCurrentGameObject.lock())
	{
		GetTransform().SetPosition(shared->GetTransform().GetPosition() + CU::Vector3f{ 0.f, myHeightOffset, 0.f });
	}

	if (myChainTimer > 0.f)
	{
		myChainTimer -= Time::DeltaTime;
		if (myChainTimer <= 0.f)
		{
			Chain();
		}
	}
}

void BouncingDamageComponent::Chain()
{
	std::vector<OverlapHit> hits;
	myGameObject->GetScene()->SphereCastAll(GetTransform().GetPosition(), myRadius, eLayer::ENEMY, hits);

	std::vector<Transform*> validTargets;
	validTargets.reserve(hits.size());
	for (auto& hit : hits)
	{
		if (std::find(myHitTargets.begin(), myHitTargets.end(), &hit.GameObject->GetTransform()) == myHitTargets.end())
		{
			validTargets.push_back(&hit.GameObject->GetTransform());
		}
	}

	if (validTargets.size())
	{
		const CU::Vector3f thisPosition = GetTransform().GetPosition();
		Transform* chosenTarget = validTargets[0];
		float shortestDistanceSqr = (thisPosition - chosenTarget->GetPosition()).LengthSqr();
		for (int i = 1; i < validTargets.size(); ++i)
		{
			const float distanceSqr = (thisPosition - validTargets[i]->GetPosition()).LengthSqr();
			if (distanceSqr < shortestDistanceSqr)
			{
				shortestDistanceSqr = distanceSqr;
				chosenTarget = validTargets[i];
			}
		}
		SpawnLightningEffects(chosenTarget);

		myHitTargets.push_back(chosenTarget);

		GetTransform().SetPosition(chosenTarget->GetPosition());
		myCurrentGameObject = chosenTarget->GetGameObjectShared();
		HealthComponent* health = chosenTarget->GetGameObject()->GetComponent<HealthComponent>();
		health->ApplyDamage(myDamage);
		--myRemainingTargets;

		myChainTimer = myDelay;
	}
	else
	{
		myRemainingTargets = 0;
	}

	if (myRemainingTargets < 1)
	{
		myGameObject->Destroy();
	}
}

void BouncingDamageComponent::SpawnLightningEffects(Transform* aNewTarget)
{
	CU::Vector3f randomOffset = CU::Vector3f{ 0.f, myHeightOffset * Random::RandomFloat(0.8f, 2.0f), 0.f };
	GameObject* object = myGameObject->GetScene()->AddGameObject<GameObject>();
	object->GetTransform().SetPosition(aNewTarget->GetPosition() + randomOffset);

	Engine::VFXComponent* comp = object->AddComponent<Engine::VFXComponent>(myImpactEffect->Get());
	comp->Play();
	comp->AutoDestroy();

	auto audio = myGameObject->GetComponent<Engine::AudioComponent>();
	audio->PlayEvent("Chain");

	Transform& mTransform = GetTransform();
	GameObject* newObject = &Engine::GameObjectPrefab::InstantiatePrefabInstance(myLightningVFX, *myGameObject->GetScene());

	Transform& newTransform = newObject->GetTransform();

	newTransform.SetPosition(mTransform.GetPosition() + CU::Vector3f{ 0.f, myHeightOffset, 0.f });
	newTransform.LookAt(aNewTarget->GetPosition() + CU::Vector3f{ 0.f, myHeightOffset, 0.f });
	float distance = (aNewTarget->GetPosition() - mTransform.GetPosition()).Length();
	newTransform.SetScale({ 1.f, 1.f, distance * 0.01f });
	newTransform.SetPosition(aNewTarget->GetPosition() + randomOffset);

	Weak<GameObject> weakGO = newObject->GetWeak();

	Main::GetTimerManager().SetTimer(myLightningDuration, [weakGO]
		{
			if (Shared<GameObject> shared = weakGO.lock())
			{
				shared->Destroy();
			}
		});
}
