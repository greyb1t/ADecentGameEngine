#include "pch.h"
#include "StateMachine.h"

#include "State.h"

StateMachine::StateMachine()
{

}

StateMachine::~StateMachine()
{

}

void StateMachine::Init()
{

}

void StateMachine::Update()
{
	if (auto* state = GetTransitionGlobal())
	{
		SetState(state);
	}

	if (myActiveState)
	{
		if (auto* state = myActiveState->GetTransition())
		{
			SetState(state);
			if (!state)
				return;
		}

		myActiveState->Update();
	}
}

void StateMachine::AddState(State* aState)
{
	myStates.emplace_back(aState);
}

void StateMachine::SetState(State* aState)
{
	assert(aState && "Error, trying to set player state to nullptr");
	if (myActiveState)
	{
		myActiveState->Exit();
	}

	myActiveState = aState;
	myActiveState->Enter();
}

void StateMachine::AddTransitionGlobal(const std::function<State*()>& aTransition)
{
	myTransitionsGlobal.emplace_back(aTransition);
}

State* StateMachine::GetTransitionGlobal()
{
	for (unsigned int i = 0; i < myTransitionsGlobal.size(); i++)
	{
		State* transitionTo = myTransitionsGlobal[i]();
		if (transitionTo != nullptr)
		{
			return transitionTo;
		}
	}
	return nullptr;
}
