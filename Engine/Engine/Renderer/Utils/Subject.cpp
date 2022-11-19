#include "pch.h"
#include "Subject.h"
#include "Observer.h"

void Engine::Subject::AddObserver(const EventType aEventType, Observer* aObserver)
{
	myObservers[aEventType].push_back(aObserver);
}

void Engine::Subject::RemoveObserver(const EventType aEventType, Observer* aObserver)
{
	std::vector<Observer*>& messageObservers = myObservers[aEventType];

	while (true)
	{
		auto result = std::find(messageObservers.begin(), messageObservers.end(), aObserver);

		if (result != messageObservers.end())
		{
			messageObservers.erase(result);
		}
		else
		{
			break;
		}
	}
}

void Engine::Subject::NotifyObservers(const EventType aEventType, const std::any& aValue)
{
	auto findResultIt = myObservers.find(aEventType);

	if (findResultIt == myObservers.end())
	{
		return;
	}

	for (auto& observer : findResultIt->second)
	{
		observer->Receive(aEventType, aValue);
	}
}
