#pragma once
class TimeStamp
{
public:
	TimeStamp(float aTime = 0.0f, float aTimeScale = 1.f)
		: myTime(aTime * aTimeScale), myTimeScale(aTimeScale) { }

	operator float() const { return myTime; }
	float GetSeconds() const { return myTime; }
	float GetMilliseconds() const { return myTime * 1000.f; }
	float GetTimeScale() const { return myTimeScale; }
	float GetUnscaledSeconds() const { return myTime / myTimeScale; }
private:
	float myTime;
	float myTimeScale;
};

