#pragma once
#include "VFXEvent.h"
class PhysicsObjectEmitterEvent :
    public VFXEvent
{
public:
    void Activate() override;
    //void AddEmission(TimeAction);
private:
	//std::vector<TimeAction> myTimeActions;
};

