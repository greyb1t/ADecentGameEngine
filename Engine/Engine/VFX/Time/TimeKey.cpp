#include "pch.h"
#include "TimeKey.h"

void Key::Repeat()
{
	nextActivationTime += repeatDelay + Random::RandomFloat(-repeatDelayRandom, repeatDelayRandom);
}

void Key::Reset()
{
	isActive = true;
	timesActivated = 0;
	nextActivationTime = time + Random::RandomFloat(-timeRandom, timeRandom);
}
