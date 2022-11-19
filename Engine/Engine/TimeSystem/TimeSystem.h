#pragma once

namespace Engine
{
	
class CallbackTimer;
class Coroutine;

class TimeSystem
{
public:
	TimeSystem();
	~TimeSystem();

	void Update(float aDeltaTime);

	void ClearSceneTimers();

private:
friend class CallbackTimer;
friend class Coroutine;
	void AddTimer(CallbackTimer* aTimer);
	void AddCoroutine(Coroutine* aCoroutine);

	void RemoveTimer(CallbackTimer* aTimer);
	void RemoveCoroutine(Coroutine* aCoroutine);
	
	std::vector<CallbackTimer*> myTimers;
	std::vector<Coroutine*> myCoroutines;
};


}