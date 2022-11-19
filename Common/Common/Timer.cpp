#include <chrono>

#include "Timer.h"

Common::Timer::Timer()
{
	const auto now = Now();

	myPrevFrameTime = now;
	myStartTime = now;
}

void Common::Timer::Update()
{
	const auto now = Now();

	myDeltaTime = std::chrono::duration_cast<std::chrono::duration<float>>(now - myPrevFrameTime).count();
	myTotalTime = std::chrono::duration_cast<std::chrono::duration<double>>(now - myStartTime).count();

	myPrevFrameTime = now;
}

float Common::Timer::GetDeltaTime() const
{
	return myDeltaTime;
}

void Common::Timer::SetDeltaTime(const float aDeltaTime)
{
	myDeltaTime = aDeltaTime;
}

double Common::Timer::GetTotalTime() const
{
	return myTotalTime;
}

std::chrono::high_resolution_clock::time_point Common::Timer::Now() const
{
	return std::chrono::high_resolution_clock::now();
}
