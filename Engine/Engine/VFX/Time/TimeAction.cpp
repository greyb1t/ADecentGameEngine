#include "pch.h"
#include "TimeAction.h"

void TimeAction::Reset()
{
	myTimer = 0;
	myActivationTime = myActionTime + Random::RandomFloat(-myActionTimeRandomizer, myActionTimeRandomizer);
}

bool TimeAction::TickAction(float aDeltaTime)
{
	if (IsFinished())
		return false;

	myTimer += aDeltaTime;
	if (myTimer >= myActivationTime)
	{
		if (myIsLooping)
		{
			Reset();
		} else
		{
			myIsFinished = true;
		}
		return true;
	}
	return false;
}

void TimeAction::SetActionTime(float aTime, float aRandomizer)
{
	myActionTime = aTime;
	myActionTimeRandomizer = aRandomizer;

	Reset();
}
