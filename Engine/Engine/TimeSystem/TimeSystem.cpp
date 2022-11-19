#include "pch.h"
#include "TimeSystem.h"
#include "CallbackTimer.h"
#include "Coroutine.h"

Engine::TimeSystem::TimeSystem()
{
}

Engine::TimeSystem::~TimeSystem()
{
	//for (int i = 0; i < myTimers.size(); i++)
	//{
	//	delete myTimers[i];
	//	myTimers[i] = nullptr;
	//}
	//for (int i = 0; i < myCoroutines.size(); i++)
	//{
	//	delete myCoroutines[i];
	//	myCoroutines[i] = nullptr;
	//}
}

void Engine::TimeSystem::Update(float aDeltaTime)
{
	ZoneScopedN("TimeSystem::Update");

	for (int i = myTimers.size() - 1; i >= 0; i--)
	{
		if (!myTimers[i]->Update(aDeltaTime))
		{
			CallbackTimer* timer = myTimers[i];
			myTimers.erase(myTimers.begin() + i);

			delete timer;
			timer = nullptr;
		}
	}
	
	for (int i = 0; i < myCoroutines.size(); i++)
	{
		myCoroutines[i]->Update(aDeltaTime);
	}
}

void Engine::TimeSystem::ClearSceneTimers()
{
	for (int i = static_cast<int>(myTimers.size()) - 1; i >= 0; i--)
	{
		if (myTimers[i]->DeleteOnSceneTransition())
		{
			// The destructor of CallbackTimer removes itself from the myTimers list
			// That is why we only delete
			delete myTimers[i];
			// myTimers[i] = nullptr;
			// myTimers.erase(myTimers.begin() + i);
		}
	}
	
	for (int i = static_cast<int>(myCoroutines.size()) - 1; i >= 0; i--)
	{
		if (myCoroutines[i]->DeleteOnSceneTransition())
		{
			// The destructor of Coroutine removes itself from the list
			// That is why we only delete
			delete myCoroutines[i];
			// myCoroutines[i] = nullptr;
			// myCoroutines.erase(myCoroutines.begin() + i);
		}
	}
}

void Engine::TimeSystem::AddTimer(CallbackTimer* aTimer)
{
	myTimers.emplace_back(aTimer);
}

void Engine::TimeSystem::AddCoroutine(Coroutine* aCoroutine)
{
	myCoroutines.emplace_back(aCoroutine);
}

void Engine::TimeSystem::RemoveTimer(CallbackTimer* aTimer)
{
	for (int i = 0; i < myTimers.size(); i++)
	{
		if (myTimers[i] == aTimer)
		{
			myTimers.erase(myTimers.begin() + i);
			return;
		}
	}
}

void Engine::TimeSystem::RemoveCoroutine(Coroutine* aCoroutine)
{
	for (int i = 0; i < myCoroutines.size(); i++)
	{
		if (myCoroutines[i] == aCoroutine)
		{
			myCoroutines.erase(myCoroutines.begin() + i);
			return;
		}
	}
}
