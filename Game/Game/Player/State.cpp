#include "pch.h"
#include "State.h"

#include "Player.h"

State::State(Player* aPlayer)
{
	SetPlayer(aPlayer);
}

void State::SetPlayer(Player* aPlayer)
{
	myPlayer = aPlayer;
}

void State::AddTransition(std::function<State*()> aTransition)
{
	myTransitions.emplace_back(aTransition);
}

State* State::GetTransition()
{
	for (unsigned int i = 0; i < myTransitions.size(); i++)
	{
		State* transitionTo = myTransitions[i]();
		if (transitionTo != nullptr)
		{
			return transitionTo;
		}
	}
	return nullptr;
}

void State::Enter()
{

}

void State::Exit()
{

}

void State::Update()
{

}

GameObject& State::GetGameObject() const
{
	return *myPlayer->GetGameObject();
}

Transform& State::GetTransform() const 
{
	return myPlayer->GetTransform();
}

PlayerStats& State::GetStats() const
{
	return myPlayer->GetStats();
}

PlayerStatus& State::GetStatus() const
{
	return myPlayer->GetStatus();
}

PlayerSettings& State::GetSettings() const
{
	return myPlayer->GetSettings();
}

Player* State::GetPlayer() const
{
	return myPlayer;
}