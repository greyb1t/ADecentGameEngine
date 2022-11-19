#pragma once
#include "Engine\GameObject\Components\Component.h"
#include "Engine\ResourceManagement\ResourceRef.h"

namespace Engine
{
	class Collider;
	class VFXComponent;
}

class HealthComponent;

class RespawnComponent : public Component
{
	COMPONENT(RespawnComponent, "Respawn Component");
public:
	RespawnComponent() = default;
	RespawnComponent(GameObject* aGameObject);
	~RespawnComponent();

	void Start() override;
	void Reflect(Engine::Reflector& aReflector) override;

	void Execute(Engine::eEngineOrder aOrder) override;

private:
	float myDamagePercent = 0.15f;

	GameObject* myPlayer = nullptr;

	Engine::Collider* myTrigger = nullptr;

	Vec3f myLastPosition;
	float myTimer = 0.0f;
	float myTime = 5.0f;

	HealthComponent* myHealthComponent = nullptr;

	bool myShouldTeleportPlayer = false;
};

