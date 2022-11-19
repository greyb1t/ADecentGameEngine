#include "pch.h"
#include "PlayerEnterBossScriptedState.h"

#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "Player/Player.h"
#include "Player/StateMachine.h"
#include "Player/States/MovementState.h"

PlayerEnterBossScriptedState::PlayerEnterBossScriptedState(Player* aPlayer)
	: State(aPlayer)
{
}

void PlayerEnterBossScriptedState::Enter()
{
	LOG_INFO(LogType::Jonathan) << "Entered Boss Stage State";
	auto& controller = GetPlayer()->GetAnimatorComponent().GetController();
	controller.Trigger("TurnTrigger");
	GetStatus().upperBodyWeightGoal = 0;
	GetStatus().isSprinting = false;
	GetStatus().isMoving = false;

	controller.SetBool("IsInScriptedEvent", true);
}

void PlayerEnterBossScriptedState::Exit()
{
	LOG_INFO(LogType::Jonathan) << "Exited Boss Stage State";
	GetStatus().upperBodyWeightGoal = 1;
	GetStatus().isSprinting = false;
	GetStatus().isMoving = true;
	//GetPlayer()->GetStateMachine().SetState(GetPlayer()->GetMovementState());

	auto& controller = GetPlayer()->GetAnimatorComponent().GetController();
	controller.SetBool("IsInScriptedEvent", false);
}

void PlayerEnterBossScriptedState::Update()
{
	// When Finished
}
