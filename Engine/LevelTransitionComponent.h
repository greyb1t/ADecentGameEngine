#pragma once
#include "Engine/GameObject/Components/Component.h"

class LevelTransitionComponent :
    public Component
{
public:
    Component* Clone() const override;
    void Execute(Engine::eEngineOrder aOrder) override;
    void Reflect(Engine::Reflector& aReflector) override;
};

