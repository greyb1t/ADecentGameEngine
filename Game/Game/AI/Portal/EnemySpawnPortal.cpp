#include "pch.h"
#include "EnemySpawnPortal.h"

#include "AI/AIDirector/AIDirector.h"
#include "AI/PollingStation/PollingStationComponent.h"
#include "Engine/Shortcuts.h"
#include "AI/AIDirector/AIDirector.h"
#include "AI/PollingStation/PollingStationComponent.h"
#include "AI/VFX/EnemyVFXRef.h"
#include "Engine/Shortcuts.h"
#include "Engine/AnimationCurve/Curve.h"
#include "Engine/GameObject/Components/ModelComponent.h"
#include "Engine/GameObject/Components/VFXComponent.h"
#include "Engine/ResourceManagement/Resources/AnimationCurveResource.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"
#include "Engine/VFX/VFXUtilities.h"



EnemySpawnPortal::EnemySpawnPortal(GameObject* aGameObject) :
	Component(aGameObject)
{
}

void EnemySpawnPortal::Start()
{
	Component::Start();
	auto dir = myGameObject->GetTransform().GetPosition() - myGameObject->GetSingletonComponent<PollingStationComponent>()->GetPlayerPos();
	dir.Normalize();

	myParticleGO = myGameObject->GetScene()->AddGameObject<GameObject>();

	const auto& vfxRef = myGameObject->GetSingletonComponent<EnemyVFXRef>()->GetEnemyGroundPortal();
	myVFX = myParticleGO->AddComponent<Engine::VFXComponent>(vfxRef->Get());
	myVFX->Play();
	//myGameObject->GetTransform().SetScale(0.1f);

}

void EnemySpawnPortal::Execute(Engine::eEngineOrder aOrder)
{
	Component::Execute(aOrder);
	Update();
}

void EnemySpawnPortal::Update()
{
	/*float scale = myGameObject->GetTransform().GetScale().x;

	auto aiDir = myGameObject->GetSingletonComponent<AIDirector>();

	if (myGameObject->GetTransform().GetScale().x > aiDir->GetPortalMaxScale())
	{
		myScaleUp = false;
	}
	if (myGameObject->GetTransform().GetScale().x <= 0.f)
	{
		myGameObject->Destroy();
	}

	if (myScaleUp)
	{
		scale += aiDir->GetPortalScaleSpeed() * Time::DeltaTime;
	}
	else
	{
		scale -= aiDir->GetPortalScaleSpeed() * Time::DeltaTime;
	}


	myGameObject->GetTransform().SetScale(scale);*/

	auto playerPos = myGameObject->GetSingletonComponent<PollingStationComponent>()->GetPlayerPos();

	auto dir = myGameObject->GetTransform().GetPosition() - playerPos;
	dir.Normalize();
	myParticleGO->GetTransform().SetPosition(GetTransform().GetPosition() + dir * myParticleOffset);

	//myGameObject->GetTransform().LookAt(playerPos);

	myCurrentLifeTime += Time::DeltaTime;
	auto value = myCurve->Get().Evaluate(myCurrentLifeTime / myLifeTime);

	auto& materialInstance = myGameObject->GetComponent<Engine::ModelComponent>()->GetMeshMaterialInstanceByIndex(0);
	materialInstance.SetFloat4("POMVar", Vec4f(value, 0.f, 0.f, 0.f));

	if (myCurrentLifeTime / myLifeTime >= 0.99f)
	{
		myGameObject->Destroy();
		myParticleGO->Destroy();
		myVFX->Destroy();
	}
}

void EnemySpawnPortal::Reflect(Engine::Reflector& aReflector)
{
	Component::Reflect(aReflector);
	aReflector.Reflect(myParticlePath, "Particle Path");
	aReflector.Reflect(myParticleOffset, "Particle Offset");
	aReflector.Reflect(myCurve, "Portal Curve");
	aReflector.Reflect(myLifeTime, "LifeTime");
}

void EnemySpawnPortal::Render()
{
	Component::Render();
}

void EnemySpawnPortal::Init()
{

}

