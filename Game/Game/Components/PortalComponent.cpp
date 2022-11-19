#include "pch.h"
#include "PortalComponent.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/Reflection/Reflector.h"
#include <Engine/GameObject/Components/TriggerComponent.h>
#include <Engine/GameObject/Components/RigidBodyComponent.h>
#include <Components/InteractableManager.h>

PortalComponent::~PortalComponent()
{
	//InteractableManager::Instance().Unregister(this);
}

void PortalComponent::Start()
{
	myRot = myGameObject->GetTransform().GetRotationLocal().EulerAngles();
	ObserveInteract([&]
		{
			myTimerStarted = true;
			// LOAD NEXT LEVEL

		});

	//InteractableManager::Instance().Register(this);
}
void PortalComponent::Execute(Engine::eEngineOrder aOrder)
{
	if (myTimerStarted)
	{
		myTimer += Time::DeltaTime;
	}
	if (myTimer > myNextLevelTime)
	{
		StartLoadLevel();
	}
	myRot = myGameObject->GetTransform().GetRotationLocal().EulerAngles();

	if (myRotSpeed.x < 0.1f && myRotSpeed.x > -0.1f)
	{
		myRotSpeed.x += Random::RandomFloat(-0.1f, 0.1f) * Time::DeltaTime;
	}
	if (myRotSpeed.y < 0.1f && myRotSpeed.y > -0.1f)
	{
		myRotSpeed.y += Random::RandomFloat(-0.1f, 0.1f) * Time::DeltaTime;
	}
	if (myRotSpeed.z < 0.1f && myRotSpeed.z > -0.1f)
	{
		myRotSpeed.z += Random::RandomFloat(-0.1f, 0.1f) * Time::DeltaTime;
	}
	myRot += myRotSpeed;

	myGameObject->GetTransform().SetRotationLocal(Quatf(myRot));
}
void PortalComponent::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myNextLevelSceneName, "Next Level Name");
	aReflector.Reflect(myNextLevelFolderName, "Next Level Folder");
}
void PortalComponent::SpawnPortal()
{
	myEvent = GeneralEvent::PortalSpawn;
}
void PortalComponent::SetEvent(GeneralEvent aEvent)
{
	myEvent = aEvent;
}
void PortalComponent::StartLoadLevel()
{
	// Engine::SceneManager::LoadJob job;
	// auto scene = MakeShared<Engine::FolderScene>();

	// LoadScene(myNextLevelSceneName, myNextLevelFolderName, scene);

	UIEventManager::TryToFadeOut([&]()
	{
		auto& s = GetEngine().GetSceneManager();

		const Engine::SceneHandle handle = s.LoadSceneAsync("Assets\\Scenes\\" + myNextLevelFolderName);
		s.SetNextScene(handle);
	});
}