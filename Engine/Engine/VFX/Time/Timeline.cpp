#include "pch.h"
#include "Timeline.h"

void Timeline::Key::Repeat(float aTimeScale)
{
	if (isLooping)
	{
		nextActivationTime += aTimeScale / timesPerSecond;
		return;
	}

	nextActivationTime += repeatDelay + Random::RandomFloat(-repeatDelayRandom, repeatDelayRandom);
}

void Timeline::Key::Reset()
{
	isActive = true;
	timesActivated = 0;

	nextActivationTime = time + Random::RandomFloat(-timeRandom, timeRandom);
}

Timeline::Key Timeline::Key::Single(float time, float timeRandom, unsigned repeatTimes, float repeatDelay,
	float repeatDelayRandom)
{
	return Key{ time, timeRandom, 1, 0, repeatTimes, repeatDelay, repeatDelayRandom };
}

Timeline::Key Timeline::Key::Burst(float time, unsigned amount, unsigned amountRandom)
{
	return Key{ time, 0.f, amount, amountRandom };
}

Timeline::Key Timeline::Key::Burst(float time, unsigned amount, unsigned amountRandom, float timeRandom,
                                   unsigned repeatTimes, float repeatDelay, float repeatDelayRandom)
{
	return Key{ time, timeRandom, amount, amountRandom, repeatTimes, repeatDelay, repeatDelayRandom };
}

Timeline::Key Timeline::Key::Create(float time, float timeRandom, unsigned amount, unsigned amountRandom,
	unsigned repeatTimes, float repeatDelay, float repeatDelayRandom, bool isLooping, float timesPerSecond)
{
	return Key{  time, timeRandom, amount, amountRandom, repeatTimes, repeatDelay, repeatDelayRandom, isLooping, timesPerSecond };
}

void Timeline::SetTotalTime(float aTotalTime)
{
	myTimeScale = 1 / aTotalTime;
}

int Timeline::Tick(float aTime)
{
	myTimer += aTime;
	int counter = 0;
	for (int i = 0; i < myKeys.size(); i++)
	{
		auto& key = myKeys[i];
		if (key.isActive)
		{
			while (key.nextActivationTime < myTimer)
			{
				counter += key.amount;
				if (key.timesActivated < key.repeatTimes || key.isLooping)
				{
					++key.timesActivated;
					key.Repeat(myTimeScale);
				} else
				{
					key.isActive = false;
					break;
				}
			}
		}
	}
	return counter;
}

void Timeline::Reset()
{
	myTimer = 0;
	for (auto& key : myKeys)
	{
		key.Reset();
	}
}

void Timeline::Add(const Key& aKey)
{
	myKeys.emplace_back(aKey);
}

void Timeline::AddSingle(float time, float timeRandom, unsigned repeatTimes, float repeatDelay,
                         float repeatDelayRandom)
{
	myKeys.emplace_back(Key{ time, timeRandom, 1,0, repeatTimes, repeatDelay, repeatDelayRandom,  });
}

void Timeline::AddBurst(float time, unsigned amount, float timeRandom, unsigned amountRandom, unsigned repeatTimes,
	float repeatDelay, float repeatDelayRandom)
{
	myKeys.emplace_back(Key{ time, timeRandom, amount,amountRandom, repeatTimes, repeatDelay, repeatDelayRandom });
}

void Timeline::Sort()
{
	std::sort(myKeys.begin(), myKeys.end(), [](Key& a, Key& b)
	{
		return a.time < b.time;
	});
}

const std::vector<Timeline::Key>& Timeline::GetKeys() const
{
	return myKeys;
}

std::vector<Timeline::Key>& Timeline::Keys() 
{
	return myKeys;
}
