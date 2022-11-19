#pragma once
#include <vector>


class Timeline
{
public:
	struct Key
	{
		float		time = 0;
		float		timeRandom = 0;

		unsigned	amount = 1;
		unsigned	amountRandom = 0;

		unsigned	repeatTimes = 0;
		float		repeatDelay = 0.1f;
		float		repeatDelayRandom = 0;
		bool		isLooping = false;
		float		timesPerSecond = 0;

		void		Repeat(float aTimeScale);
		void		Reset();

		bool		isActive = true;
		float		nextActivationTime = 0;
		unsigned	timesActivated = 0;

		static Key Single(float time, float timeRandom = 0.f, unsigned repeatTimes = 0, float repeatDelay = 0.f, float repeatDelayRandom = 0.f);
		static Key Burst(float time, unsigned amount, unsigned amountRandom = 0);
		static Key Burst(float time, unsigned amount, unsigned amountRandom, float timeRandom = 0.f, unsigned repeatTimes = 0, float repeatDelay = 0.f, float repeatDelayRandom = 0.f);
		static Key Create(float time, float timeRandom, unsigned amount, unsigned amountRandom, unsigned repeatTimes, float repeatDelay, float repeatDelayRandom, bool isLooping, float timesPerSecond);
	};

	void SetTotalTime(float aTotalTime);
	/**
	 * \brief 
	 * \return Returns amount of actions
	 */
	int Tick(float aTime);

	void Reset();

	void Add(const Key&);
	void AddSingle(float time, float timeRandom = 0.f, unsigned repeatTimes = 0, float repeatDelay = 0.f, float repeatDelayRandom = 0.f);
	void AddBurst(float time, unsigned amount, float timeRandom = 0.f, unsigned amountRandom = 0, unsigned repeatTimes = 0, float repeatDelay = 0.f, float repeatDelayRandom = 0.f);

	void Sort();

	float GetTime() const { return myTimer; }
	const std::vector<Key>& GetKeys() const;
	std::vector<Key>& Keys();
private:
	std::vector<Key> myKeys;
	float myTimer = 0;
	float myTimeScale = 1;
};