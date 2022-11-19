#pragma once
#include "Player/State.h"

class ShootState :
    public State
{
public:
    enum class eCombatState
    {
	    COMBAT_IDLE,
        COMBAT_READY,
        PRIMARY_SHOOTING,
        SECONDARY_CHARGING,
    };

    ShootState(Player* aPlayer);

    void Enter() override;
    void Exit() override;
    virtual void Update() override;
    eCombatState GetState();
private:
    void ResetCombatTimer();
    void UpdateCombatTimer();

    void UpdatePrimary();
    void UpdateSecondary();
    void UpdateMobility();
    void UpdateUtility();

    void ActivateSecondary();
    void ShootSecondary();
    void InterruptSecondary();
    void EndSecondary();

    eCombatState myCombatState = eCombatState::COMBAT_IDLE;
    float myCombatReadyTimer = 0;
};

