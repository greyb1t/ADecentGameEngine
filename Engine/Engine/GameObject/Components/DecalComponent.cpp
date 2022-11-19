#include "pch.h"
#include "DecalComponent.h"
#include "Engine\Reflection\Reflector.h"
#include "Engine\GameObject\GameObject.h"

void Engine::DecalComponent::Start()
{
}

Engine::DecalComponent::DecalComponent(GameObject* aGameObject)
	: Component(aGameObject)
{
}

void Engine::DecalComponent::Execute(eEngineOrder aOrder)
{
}

void Engine::DecalComponent::Render()
{
	const auto& t = myGameObject->GetTransform();

	myGameObject->GetScene()->GetRendererScene().RenderDecal(*this);

	if (GetEngine().IsEditorEnabled())
	{
		GDebugDrawer->DrawCube3D(
			DebugDrawFlags::Always,
			t.GetPosition(),
			t.GetRotation().EulerAngles(),
			mySize * t.GetScale());
	}
}

void Engine::DecalComponent::Reflect(Reflector& aReflector)
{
	Component::Reflect(aReflector);

	aReflector.Reflect(mySize, "Size");

	myMaterialInstance.Reflect(aReflector, 0);
}

Engine::DecalMaterialInstance& Engine::DecalComponent::GetMaterialInstance()
{
	return myMaterialInstance;
}
