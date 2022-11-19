#pragma once
#include "Player/State.h"

class PlayerEnterBossScriptedState :
    public State
{
public:
	PlayerEnterBossScriptedState(Player* aPlayer);
	void Enter() override;
	void Exit() override;
	void Update() override;
private:

};

