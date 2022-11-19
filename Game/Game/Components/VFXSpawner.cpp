#include "pch.h"
#include "VFXSpawner.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"
#include "Engine/GameObject/Components/VFXComponent.h"
#include "Engine/GameObject/GameObject.h"


VFXSpawner::VFXSpawner(GameObject* aGameObject)
	: Component(aGameObject)
{
}

void VFXSpawner::Reflect(Engine::Reflector& aReflector)
{
	Component::Reflect(aReflector);

	aReflector.Reflect(myVFX, "VFX");
}

void VFXSpawner::Awake()
{
	if (myVFX->IsValid())
	{
		myVFXComponent = myGameObject->AddComponent<Engine::VFXComponent>(myVFX->Get());
		myVFXComponent->Play();
	}
	else
	{
		LOG_ERROR(LogType::Game) << "VFX Spawner VFX is invalid";
	}
}

void VFXSpawner::SetVfx(const VFXRef& anVfx)
{
	myVFX = anVfx;
}

void VFXSpawner::Stop()
{
	if (myVFXComponent)
	{
		myVFXComponent->Stop();
	}
}
