#pragma once

class DownTimer
{
public:
	void Init(const float aTimeSeconds);

	void Start();

	bool IsStarted() const;
	bool IsFinished() const;

	void Update(const float aDeltaTime);

	float GetStartTimeSeconds() const;
	float GetCurrentTimeSeconds() const;

	// 0: Beginning
	// 1: End
	float GetPercentage() const;

private:
	float myStartTimeSeconds = 0.f;
	float myCurrentTimeSeconds = 0.f;
	bool myStarted = false;
};