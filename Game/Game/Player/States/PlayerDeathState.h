#pragma once
#include "Player/State.h"

class PlayerDeathState :
    public State
{
public:
    PlayerDeathState(Player* aPlayer);
    void Enter() override;
    void Update() override;
    void Exit() override;
private:
    float myInvisibilityTimer = 0;
    bool myIsVisible = true;
};

