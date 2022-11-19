#pragma once

#include "Player/State.h"

namespace Engine 
{
    class VFXComponent;
}

class DashState :
    public State
{
    enum class eDashState
    {
	    DASH_IN,
        DASH_UPDATE,
        DASH_OUT,
    };

public:
    DashState(Player* aPlayer);
    void Enter() override;
    void Update() override;
    void Exit() override;
private:
    void EnterIn();
    void UpdateIn();

    void EnterUpdateDash();
    void UpdateDash();

    void EnterOut();
    void UpdateOut();

    void ExitDash();
    void UpdateMovement();
    void Move(float aDistance);
private:
    eDashState myState = eDashState::DASH_IN;
    Vec3f myDashDirection = 0;
    float myTimer = 0;
    float myDistanceDashed = 0;
    float myDashSpeed = 0;
    float myDashTotalTime = 0;

    Engine::VFXComponent* myDashVFX = nullptr;
    GameObject* myVFXObject = nullptr;
};

