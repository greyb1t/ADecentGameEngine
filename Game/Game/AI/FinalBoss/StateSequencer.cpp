#include "pch.h"
#include "StateSequencer.h"
#include "States/FinalBossState.h"

void FB::StateSequencer::Init()
{
	myIsFinished = false;
	myStateIndex = 0;

	ChangeState(0);
}

void FB::StateSequencer::Update()
{
	if (myIsFinished)
	{
		return;
	}

	if (myActiveState)
	{
		myActiveState->Update();

		if (myActiveState->IsFinished())
		{
			// Go to next state in the transition tree
			int nextStateInt = myStateIndex + 1;

			if (nextStateInt >= myStates.size())
			{
				myIsFinished = true;
				myActiveState = nullptr;
			}
			else
			{
				ChangeState(nextStateInt);
			}
		}
	}
}

bool FB::StateSequencer::IsFinished() const
{
	return myIsFinished;
}

FB::BaseFinalBossState* FB::StateSequencer::GetActiveState()
{
	return myStates[myStateIndex];
}

int FB::StateSequencer::GetActiveStateIndex() const
{
	return myStateIndex;
}

void FB::StateSequencer::OnExit()
{
	if (myActiveState)
	{
		myActiveState->OnExit();
	}
}

void FB::StateSequencer::ChangeState(const int aStateIndex)
{
	if (myActiveState)
	{
		myActiveState->OnExit();
	}

	myActiveState = myStates[aStateIndex];
	myStateIndex = aStateIndex;

	if (myActiveState)
	{
		myActiveState->OnEnter();
	}
}

void FB::StateSequencer::AddState(BaseFinalBossState* aState)
{
	myStates.push_back(aState);
}
