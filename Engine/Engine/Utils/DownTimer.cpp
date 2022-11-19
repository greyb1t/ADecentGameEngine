#include "pch.h"
#include "DownTimer.h"
#include "Engine/Engine.h"

void DownTimer::Update(const float aDeltaTime)
{
	if (!myStarted)
	{
		return;
	}

	myCurrentTimeSeconds -= aDeltaTime;

	if (IsFinished())
	{
		myCurrentTimeSeconds = 0.f;
	}
}

float DownTimer::GetStartTimeSeconds() const
{
	return myStartTimeSeconds;
}

float DownTimer::GetCurrentTimeSeconds() const
{
	return myCurrentTimeSeconds;
}

float DownTimer::GetPercentage() const
{
	return 1.f - Math::InverseLerp(0.f, myStartTimeSeconds, myCurrentTimeSeconds);
}

void DownTimer::Init(const float aTimeSeconds)
{
	myCurrentTimeSeconds = aTimeSeconds;
	myStartTimeSeconds = aTimeSeconds;

	myStarted = false;
}

void DownTimer::Start()
{
	myStarted = true;
}

bool DownTimer::IsStarted() const
{
	return myStarted;
}

bool DownTimer::IsFinished() const
{
	return myCurrentTimeSeconds <= 0.f;
}