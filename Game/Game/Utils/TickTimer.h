#pragma once

class TickTimer
{
public:
	TickTimer() = default;

	static TickTimer FromSeconds(const float aSeconds);

	void Tick(const float aDeltaTime);

	// Returns true if finished this frame
	bool JustFinished() const;

	bool IsFinished() const;

	float Percent() const;

private:
	float myStartTimerDuration = 0.f;
	float myTimerSeconds = 0.f;
	bool myFinished = false;
	bool myJustFinished = false;
};