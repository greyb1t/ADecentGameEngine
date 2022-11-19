#include "pch.h"
#include "CallbackTimer.h"

#include "../Engine.h"
#include "TimeSystem.h"

Engine::CallbackTimer::CallbackTimer(const std::function<void(void)>& aFunction, float aTime, bool aDeleteOnSceneTransition)
{
	myFunction = aFunction;
	myTime = aTime;
	myDeleteOnSceneTransition = aDeleteOnSceneTransition;
	GetEngine().GetTimeSystem().AddTimer(this);
}

Engine::CallbackTimer::~CallbackTimer()
{
	GetEngine().GetTimeSystem().RemoveTimer(this);
}

bool Engine::CallbackTimer::DeleteOnSceneTransition() const
{
	return myDeleteOnSceneTransition;
}

bool Engine::CallbackTimer::Update(float aDeltaTime)
{
	myTime -= aDeltaTime;
	if (myTime <= 0)
	{
		myFunction();
		return false;
	}
	return true;
}
