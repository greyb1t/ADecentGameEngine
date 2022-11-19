#include "pch.h"
#include "LaunchpadComponent.h"

#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/RigidBodyComponent.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "Engine/GameObject/Components/AudioComponent.h"

#include "Game/Components/HealthComponent.h"
#include <Player/Player.h>

LaunchpadComponent::LaunchpadComponent(GameObject* aObject):
	Component(aObject)
{}

void LaunchpadComponent::Start()
{
	InitAudio();
	GetGameObject()->SetLayers(eLayer::NONE, eLayer::NONE, eLayer::PLAYER);

	Engine::RigidBodyComponent* rigidbody = GetGameObject()->GetComponent<Engine::RigidBodyComponent>();
	if(!rigidbody)
	{
		// LOG_WARNING(LogType::Game, "Missing component RigidbodyComponent from LaunchpadComponent");
		return;
	}

	rigidbody->ObserveTriggerEnter([&](GameObject* aObject){Collide(aObject);});
	auto audio = myGameObject->GetComponent<Engine::AudioComponent>();
	audio->PlayEvent("GeysirLoop");
}

void LaunchpadComponent::Execute(Engine::eEngineOrder)
{
	myCollisionTimer += Time::DeltaTime;
}

void LaunchpadComponent::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myStrength, "Strength");
}

void LaunchpadComponent::Collide(GameObject* aObject)
{
	auto audio = myGameObject->GetComponent<Engine::AudioComponent>();
	audio->PlayEvent("GeysirErupt");

	HealthComponent* health = aObject->GetComponent<HealthComponent>();
	if(!health)
	{
		return;
	}

	if(myCollisionTimer >= .25f)
	{
		Player* player = aObject->GetComponent<Player>();
		if(player)
		{
			aObject->GetComponent<Player>()->GetVelocity().SetY(0.f);
		}

		health->ApplyKnockback({0.f, myStrength, 0.f});
		myCollisionTimer = 0.f;

		if (auto* player = aObject->GetComponent<Player>()) 
		{
			player->GetAnimatorComponent().GetController().Trigger("Jump");
		}
	}
}
void LaunchpadComponent::InitAudio()
{
	auto audio = myGameObject->AddComponent<Engine::AudioComponent>();
	
	audio->AddEvent("GeysirErupt", "event:/SFX/ENV/GeysirErupt");
	audio->AddEvent("GeysirLoop", "event:/SFX/ENV/GeysirLoop");
}