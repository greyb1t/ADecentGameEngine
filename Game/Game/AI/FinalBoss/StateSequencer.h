#pragma once

namespace FB
{
	class BaseFinalBossState;

	class StateSequencer
	{
	public:
		void Init();

		void Update();

		void AddState(BaseFinalBossState* aState);

		bool IsFinished() const;

		BaseFinalBossState* GetActiveState();
		int GetActiveStateIndex() const;

		void OnExit();

	private:
		void ChangeState(const int aStateIndex);

	private:
		bool myIsFinished = false;
		int myStateIndex = 0;
		std::vector<BaseFinalBossState*> myStates;
		BaseFinalBossState* myActiveState = nullptr;
	};
}