#pragma once

#include <vector>
#include "TriggerEventListener.h"
#include "Engine/Reflection/Enum.h"

class Subscriber;

class TriggerEventEnum : public Engine::Enum
{

};

class TriggerEventManager
{
public:
	TriggerEventManager();

	void Subscribe(TriggerEventListener* aObserver, const std::string& aEvent);

	void Unsubscribe(TriggerEventListener* aObserver, const std::string& aEvent);

	void SendLetter(const std::string aEvent);

	void SendLetter(const std::string aEvent, GameObject* aGO);

	static TriggerEventManager& GetInstance();

private:
	std::map<std::string, std::vector<TriggerEventListener*>> mySubscribers;

	static TriggerEventManager ourTriggerEventManager;
};
