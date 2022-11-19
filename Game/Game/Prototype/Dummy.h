#pragma once
#include "Engine\GameObject\Components\Component.h"

class HealthComponent;

namespace Engine
{
	class RigidBodyComponent;
}

class Dummy :
    public Component
{
public:
    COMPONENT(Dummy, "Dummy");
    Dummy() = default;
    Dummy(GameObject*);

    void Reflect(Engine::Reflector& aReflector) override;

    void Start() override;
    void Execute(Engine::eEngineOrder aOrder) override;

    void OnDamage(float aDmg);
    void OnDeath(float aDmg);
private:
    Engine::RigidBodyComponent* myRb = nullptr;
    HealthComponent* myHealth = nullptr;
    bool myConstrainRotation = true;
};