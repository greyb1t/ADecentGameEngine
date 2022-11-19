#pragma once

class Key
{
public:
	float		time = 0;
	float		timeRandom = 0;

	unsigned	amount = 1;
	unsigned	amountRandom = 0;

	unsigned	repeatTimes = 0;
	float		repeatDelay = 0.1f;
	float		repeatDelayRandom = 0;

	void		Repeat();
	void		Reset();

	bool		isActive = true;
	float		nextActivationTime = 0;
	unsigned	timesActivated = 0;
};

