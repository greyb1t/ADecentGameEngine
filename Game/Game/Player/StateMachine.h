#pragma once
class State;

class StateMachine
{
public:
	StateMachine();
	~StateMachine();

	void Init();
	void Update();

	void AddState(State* aState);
	void SetState(State* aState);
	void AddTransitionGlobal(const std::function<State* ()>& aTransition);
	State* GetTransitionGlobal();
private:
	State* myActiveState = nullptr;

	std::vector<State*> myStates;
	std::vector<std::function<State*()>> myTransitionsGlobal;
	
};

