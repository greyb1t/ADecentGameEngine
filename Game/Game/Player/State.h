#pragma once
#include "PlayerStats.h"
#include "PlayerStatus.h"
#include "PlayerSettings.h"
#include "Engine/GameObject/Transform.h"
class Player;

class State
{
public:
	virtual ~State() = default;
	State() = default;
	State(Player* aPlayer);
	void SetPlayer(Player* aPlayer);
	void AddTransition(std::function<State* ()> aTransition);
	State* GetTransition();

	virtual void Enter();
	virtual void Exit();

	virtual void Update();


	// Helper functions
protected:
	GameObject&		GetGameObject() const;
	Transform&		GetTransform() const;

	PlayerStats&	GetStats() const;
	PlayerStatus&	GetStatus() const;
	PlayerSettings&	GetSettings() const;
	Player*			GetPlayer() const;
private:
	Player* myPlayer = nullptr;
	std::vector<std::function<State*()>> myTransitions;
};