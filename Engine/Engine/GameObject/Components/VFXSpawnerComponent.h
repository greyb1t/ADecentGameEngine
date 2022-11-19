#pragma once
#include "Component.h"
#include "Engine/ResourceManagement/ResourceRef.h"

namespace Engine
{
	class VFXComponent;
	class AnimationController;
}

class VFXSpawnerComponent :
    public Component
{
public:
	COMPONENT(VFXSpawnerComponent, "VFX Spawner Component");

	VFXSpawnerComponent() = default;
	VFXSpawnerComponent(GameObject* aGameObject);

	void Reflect(Engine::Reflector& aReflector) override;

	void Start() override;

	void Execute(Engine::eEngineOrder aOrder) override;

	void SetVFX(const VFXRef& aVFXRef);

private:
	Engine::VFXComponent* myDripVFX = nullptr;
	VFXRef myVFXRef;

	Engine::AnimationController* myController = nullptr;
};

