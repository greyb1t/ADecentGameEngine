#pragma once

class TimerManager
{
public:
	using Callback = std::function<void(void)>;

	void	Update();
	int		SetTimer(float aDuration, const Callback& aCallback);
	float	CheckTimer(int anID);
	void	StopTimer(int anID);

private:
	struct Timer
	{
		std::function<void(void)> myCallback;
		float endTime = 0.f;
		int ID = 0;
	};

	std::vector<Timer> myTimers;
	float myCurrentTime = 0.f;
	int myNextID = 0;
};

