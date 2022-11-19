#pragma once
#include "Component.h"
#include "Engine/VFX/Description/PhysicsEmitterDescription.h"
#include "Engine/VFX/Emitter/Emitter.h"
class PhysicsObjectEmitter;

class PhysicsObjectEmitterComponent :
    public Component, public VFX::Emitter
{
public:
    COMPONENT_COPYABLE(PhysicsObjectEmitterComponent, "Physics Object Emitter Component");

    PhysicsObjectEmitterComponent() = default;
    PhysicsObjectEmitterComponent(GameObject* aGameObject, const VFX::PhysicsEmitterDescription& aDescription);

    void Start() override;
    void Execute(Engine::eEngineOrder aOrder) override;
protected:
    void Emit() override;
private:
    VFX::PhysicsEmitterDescription myDescription;
};

