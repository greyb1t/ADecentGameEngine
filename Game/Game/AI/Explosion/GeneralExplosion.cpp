#include "pch.h"
#include "GeneralExplosion.h"

#include "Engine/VFX/VFXUtilities.h"

#include "AI/AIDirector/AIDirector.h"
#include "Engine/GameObject/Components/VFXComponent.h"
#include "Engine/VFX/VFXUtilities.h"
#include "Engine/GameObject/GameObject.h"

//GeneralExplosion::GeneralExplosion(GameObject* aGameObject, std::string aPath) :
//	Component(aGameObject),
//	myVFXPath(aPath)
//{
//}

GeneralExplosion::GeneralExplosion(GameObject* aGameObject) :
	Component(aGameObject)
{
}

GeneralExplosion::GeneralExplosion(GameObject* aGameObject, VFX::VFXDescription& aDescription) :
	Component(aGameObject),
	myVFXDes(&aDescription)
{
}

void GeneralExplosion::Start()
{
	Component::Start();

	myGameObject->SetName("GeneralExplodeVFX");

	myVFX = myGameObject->AddComponent<Engine::VFXComponent>(*myVFXDes);
	myVFX->Play();
	myVFX->AutoDestroy();
}

void GeneralExplosion::Execute(Engine::eEngineOrder aOrder)
{
	Component::Execute(aOrder);
	Update();
}

void GeneralExplosion::Update()
{
	myLifeTimer += Time::DeltaTime;
	if (myLifeTimer >= myLifeTime)
	{
		myGameObject->Destroy();
		myVFX->Stop();
	}
}

void GeneralExplosion::Reflect(Engine::Reflector& aReflector)
{
	Component::Reflect(aReflector);
}

void GeneralExplosion::Render()
{
	Component::Render();
}
