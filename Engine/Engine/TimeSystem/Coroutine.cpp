#include "pch.h"
#include "Coroutine.h"
#include "../Engine.h"
#include "TimeSystem.h"

Engine::Coroutine::Coroutine(const std::function<float(float)>& aFunction, float aWaitTime, bool aDeleteOnSceneTransition)
	: myFunction(aFunction), myTime(aWaitTime), myStepTime(aWaitTime)
{
	myDeleteOnSceneTransition = aDeleteOnSceneTransition;
	GetEngine().GetTimeSystem().AddCoroutine(this);
}

Engine::Coroutine::~Coroutine()
{
	GetEngine().GetTimeSystem().RemoveCoroutine(this);
}

void Engine::Coroutine::Update(float aDeltaTime)
{
	myTime -= aDeltaTime;
	if (myTime <= 0)
	{
		myStepTime += abs(myTime);
		myStepTime = myFunction(myStepTime);
		myTime = 0;
		myTime += myStepTime;
	}
}

bool Engine::Coroutine::DeleteOnSceneTransition() const
{
	return myDeleteOnSceneTransition;
}
