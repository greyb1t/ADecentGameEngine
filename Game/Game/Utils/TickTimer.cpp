#include "pch.h"
#include "TickTimer.h"

TickTimer TickTimer::FromSeconds(const float aSeconds)
{
	TickTimer timer;
	timer.myTimerSeconds = aSeconds;
	timer.myStartTimerDuration = aSeconds;

	return timer;
}

void TickTimer::Tick(const float aDeltaTime)
{
	myJustFinished = false;

	if (myFinished)
	{
		return;
	}

	myTimerSeconds -= aDeltaTime;
	myTimerSeconds = std::max(myTimerSeconds, 0.f);

	if (myTimerSeconds <= 0.f)
	{
		myJustFinished = true;
		myFinished = true;
	}
}

bool TickTimer::JustFinished() const
{
	return myJustFinished;
}

bool TickTimer::IsFinished() const
{
	return myFinished;
}

float TickTimer::Percent() const
{
	return Math::InverseLerp(myStartTimerDuration, 0.f, myTimerSeconds);
}