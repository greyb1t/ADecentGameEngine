#pragma once
#include <vector>
#include <functional>

#include "Timeline.h"
#include "Common/Random.h"
#include "Timeline.h"

template<class T>
class ObjectTimeline
{
public:
	struct Key : public Timeline::Key
	{
		T data;
	};

	void SetTotalTime(float aTotalTime);
	
	/**
	 * \brief Each key will call the subscription when activated
	 * \return Returns amount of actions
	 */
	int Tick(float aTime);

	void Reset();

	void Add(T data, const Timeline::Key&);
	void Clear();


	void AddSingle(T data, float time, float timeRandom = 0.f, unsigned repeatTimes = 0, float repeatDelay = 0.f, float repeatDelayRandom = 0.f);
	void AddBurst(T data, float time, unsigned amount, float timeRandom = 0.f, unsigned amountRandom = 0, unsigned repeatTimes = 0, float repeatDelay = 0.f, float repeatDelayRandom = 0.f);
	void Subscribe(std::function<void(T)> callback);

	float GetTime() const { return myTimer; }

	const std::vector<Key>& GetKeys() const;
private:
	std::vector<Key> myKeys;
	std::function<void(T)> mySubscription;
	float myTimer = 0;
	float myTimeScale = 1;
};

template <class T>
void ObjectTimeline<T>::SetTotalTime(float aTotalTime)
{
	myTimeScale = 1 / aTotalTime;
}

template <class T>
int ObjectTimeline<T>::Tick(float aTime)
{
	myTimer += aTime;
	int counter = 0;
	for (int i = 0; i < myKeys.size(); i++)
	{
		auto& key = myKeys[i];
		if (key.isActive && key.nextActivationTime < myTimer)
		{
			if (mySubscription)
				mySubscription(key.data);

			counter += key.amount;
			if (key.timesActivated < key.repeatTimes)
			{
				++key.timesActivated;
				key.Repeat(myTimeScale);
			}
			else
			{
				key.isActive = false;
			}
		}
	}
	return counter;
}

template <class T>
void ObjectTimeline<T>::Reset()
{
	myTimer = 0;
	for (auto& key : myKeys)
	{
		key.Reset();
	}
}

template <class T>
void ObjectTimeline<T>::Add(T data, const Timeline::Key& aKey)
{
	Key k;
	k.data = data;
	k.time = aKey.time;
	k.amount = aKey.amount;
	k.amountRandom = aKey.amountRandom;
	k.timeRandom = aKey.timeRandom;
	k.repeatTimes = aKey.repeatTimes;
	k.repeatDelay = aKey.repeatDelay;
	k.repeatDelayRandom = aKey.repeatDelayRandom;
	myKeys.emplace_back(k);
}

template <class T>
void ObjectTimeline<T>::Clear()
{
	myKeys.clear();
}

template <class T>
void ObjectTimeline<T>::AddSingle(T data, float time, float timeRandom, unsigned repeatTimes, float repeatDelay,
	float repeatDelayRandom)
{
	Key k;
	k.data = data;
	k.time = time;
	k.amount = 1;
	k.amountRandom = 0;
	k.timeRandom = timeRandom;
	k.repeatTimes = repeatTimes;
	k.repeatDelay = repeatDelay;
	k.repeatDelayRandom = repeatDelayRandom;

	myKeys.emplace_back(k);
}

template <class T>
void ObjectTimeline<T>::AddBurst(T data, float time, unsigned amount, float timeRandom, unsigned amountRandom,
	unsigned repeatTimes, float repeatDelay, float repeatDelayRandom)
{
	Key k;
	k.data = data;
	k.time = time;
	k.amount = amount;
	k.amountRandom = amountRandom;
	k.timeRandom = timeRandom;
	k.repeatTimes = repeatTimes;
	k.repeatDelay = repeatDelay;
	k.repeatDelayRandom = repeatDelayRandom;

	myKeys.emplace_back(k);
}

template <class T>
void ObjectTimeline<T>::Subscribe(std::function<void(T)> callback)
{
	mySubscription = callback;
}

template <class T>
const std::vector<typename ObjectTimeline<T>::Key>& ObjectTimeline<T>::GetKeys() const
{
	return myKeys;
}
