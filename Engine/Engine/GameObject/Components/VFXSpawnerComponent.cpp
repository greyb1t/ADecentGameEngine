#include "pch.h"
#include "VFXSpawnerComponent.h"
#include "Engine\Reflection\Reflector.h"
#include "Engine\GameObject\GameObject.h"
#include <Engine\GameObject\Components\VFXComponent.h>
#include "Engine/ResourceManagement/Resources/VFXResource.h"

#include "Engine\GameObject\Components\AnimatorComponent.h"

VFXSpawnerComponent::VFXSpawnerComponent(GameObject* aGameObject)
	: Component(aGameObject)
{
}

void VFXSpawnerComponent::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myVFXRef, "Blood Drain Particle");
}

void VFXSpawnerComponent::Start()
{
	myDripVFX = myGameObject->AddComponent<Engine::VFXComponent>(myVFXRef->Get());
	myDripVFX->Play();


	auto parent = GetTransform().GetParent();
	if (parent)
	{
		auto animComp = parent->GetGameObject()->GetComponent<Engine::AnimatorComponent>();

		if (animComp)
		{
			myController = &animComp->GetController();
		}
	}
}

void VFXSpawnerComponent::Execute(Engine::eEngineOrder aOrder)
{
	Vec3f position =  myController->GetBoneTransformWorld("BloodOrb").GetTranslation();

	GetTransform().SetPosition(position);
}

void VFXSpawnerComponent::SetVFX(const VFXRef& aVFXRef)
{
	myVFXRef = aVFXRef;
}
