#pragma once

#include "EventType.h"

namespace Engine
{
	class Observer;

	class Subject
	{
	public:
		void AddObserver(const EventType aEventType, Observer* aObserver);

		void RemoveObserver(const EventType aEventType, Observer* aObserver);

		void NotifyObservers(const EventType aEventType, const std::any& aValue = {});

	private:
		std::unordered_map<EventType, std::vector<Observer*>> myObservers;
	};
}