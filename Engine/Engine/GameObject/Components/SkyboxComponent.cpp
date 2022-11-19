#include "pch.h"
#include "SkyboxComponent.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/Reflection/Reflector.h"

#include "ModelComponent.h"

Engine::SkyboxComponent::SkyboxComponent(GameObject* aGameObject)
	: Component(aGameObject)
{
}

void Engine::SkyboxComponent::Awake()
{
	myModelComponent = myGameObject->GetComponent<ModelComponent>();
}

void Engine::SkyboxComponent::Render()
{
	GetTransform().SetPosition(myGameObject->GetScene()->GetMainCameraGameObject()->GetTransform().GetPosition());
}

void Engine::SkyboxComponent::SetAdditiveColor(const Vec3f& aColor)
{
	if (!myModelComponent)
	{
		LOG_ERROR(LogType::Components) << "Missing modelcomponent on skybox gameobject";
		return;
	}

	auto& mesh = myModelComponent->GetMeshMaterialInstanceByIndex(0);
	mesh.SetAdditiveColor(aColor);
}

const Vec3f Engine::SkyboxComponent::GetAdditiveColor() const
{
	if (!myModelComponent)
	{
		LOG_ERROR(LogType::Components) << "Missing modelcomponent on skybox gameobject";
		return Vec3f(0,0,0);
	}

	auto& mesh = myModelComponent->GetMeshMaterialInstanceByIndex(0);
	return mesh.GetAdditiveColor();
}