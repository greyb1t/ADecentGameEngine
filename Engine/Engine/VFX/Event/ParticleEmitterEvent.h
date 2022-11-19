#pragma once
#include "VFXEvent.h"

class Emitter;

class ParticleEmitterEvent :
    public VFXEvent
{
public:
    void Activate() override;
    //void AddEmission(TimeAction aEvent);
private:
    //std::vector<TimeAction> myTimeActions;
};

