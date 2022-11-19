#pragma once

#include <vector>
#include "GeneralEventListener.h"
#include "Engine/Reflection/Enum.h"

class Subscriber;

enum class GeneralEvent
{
	PortalSpawn,
	PortalTimerStart,
	Count
};
class GeneralEventEnum : public Engine::Enum
{
	std::string EnumToString(int aValue) override
	{
		switch (static_cast<GeneralEvent>(aValue))
		{
		case GeneralEvent::PortalSpawn:
		{
			return "StartGame";
		}
		break;
		case GeneralEvent::PortalTimerStart:
		{
			return "StartGame";
		}
		break;
		}

		return "undefined";
	}
	int GetCount() override { return static_cast<int>(GeneralEvent::Count); }

private:
};

class GeneralEventManager
{
public:
	GeneralEventManager();

	void Subscribe(GeneralEventListener* aObserver, GeneralEvent aEvent);

	void Unsubscribe(GeneralEventListener* aObserver, GeneralEvent aEvent);

	void SendLetter(const GeneralEvent& aEvent, GameObject* aGameObject);

	static GeneralEventManager& GetInstance();

private:
	std::vector<std::vector<GeneralEventListener*>> mySubscribers;

	static GeneralEventManager ourGeneralEventManager;
};
