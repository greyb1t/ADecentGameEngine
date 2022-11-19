#pragma once

#include "EventType.h"

namespace Engine
{
	class Observer
	{
	public:
		virtual void Receive(const EventType aEventType, const std::any& aValue) = 0;
	};
}