#pragma once
#include "Engine/GameObject/Components/Component.h"


class VFXSpawner : public Component
{
	COMPONENT(VFXSpawner, "VFX Spawner");
public:
	VFXSpawner() = default;
	VFXSpawner(GameObject* aGameObject);

	void Reflect(Engine::Reflector& aReflector) override;

	void Awake() override;

	void SetVfx(const VFXRef& anVfx);

	void Stop();

private:
	VFXRef myVFX;
	Engine::VFXComponent* myVFXComponent = nullptr;
};