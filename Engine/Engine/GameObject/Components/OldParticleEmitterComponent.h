#pragma once
#include "Component.h"
//class VFXParticleEmitter;

class OldParticleEmitterComponent :
    public Component
{
public:
    OldParticleEmitterComponent(GameObject* aGameObject);

    void Start() override;
    void Execute(Engine::eEngineOrder aOrder) override;

//    VFXParticleEmitter& GetEmitter();
//private:
//    VFXParticleEmitter* myEmitter;
};

