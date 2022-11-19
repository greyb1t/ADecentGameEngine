#pragma once

// TODO: Add action bursts with both insta burst and over time burst?
// TIMELINE

class TimeAction
{
public:
	TimeAction(float aTime = 0.f, float aRandomizer = 0.f, bool isLooping = false)
		: myActionTime(aTime), myActionTimeRandomizer(aRandomizer), myIsLooping(isLooping)
	{
		Reset();
	}

	void Reset();

	// This updates time
	bool TickAction(float aDeltaTime);

	bool IsFinished() const { return myIsFinished; }
	bool IsLooping() const { return myIsLooping; }

	void SetActionTime(float aTime, float aRandomizer);
	void SetLooping(bool aLooping) { myIsLooping = aLooping; }
private:
	bool myIsFinished = false;
	bool myIsLooping = false;
	float myActionTime = 0.f;
	float myActionTimeRandomizer = 0.f;

	float myActivationTime = 0;
	float myTimer = 0;
};

