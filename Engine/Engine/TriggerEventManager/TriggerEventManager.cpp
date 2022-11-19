#include "pch.h"
#include "TriggerEventManager.h"
#include "TriggerEventListener.h"
#include "Engine/GameObject/GameObject.h"
#include <Engine/GameObject/Components/ScriptsComponent.h>

TriggerEventManager TriggerEventManager::ourTriggerEventManager;

TriggerEventManager::TriggerEventManager()
{

}

void TriggerEventManager::Subscribe(TriggerEventListener* aObserver, const std::string& aEvent)
{
	std::vector<TriggerEventListener*>* listeners = DBG_NEW std::vector<TriggerEventListener*>;
	mySubscribers.insert(std::pair(aEvent, *listeners));
	mySubscribers.at(aEvent).push_back(aObserver);
}

void TriggerEventManager::Unsubscribe(TriggerEventListener* aObserver, const std::string& aEvent)
{
	auto f = mySubscribers.find(aEvent);

	if (f != mySubscribers.end())
	{
		auto& observers = f->second;
		auto result = std::find(observers.begin(), observers.end(), aObserver);
		if (result != observers.end())
		{
			observers.erase(result);
		}
	}
}

void TriggerEventManager::SendLetter(const std::string aEvent)
{
	for (auto& [event, observers] : mySubscribers)
	{
		if (event == aEvent)
		{
			for (auto observer : observers)
			{
				observer->OnEvent();
			}
		}
	}
}
void TriggerEventManager::SendLetter(const std::string aEvent, GameObject* aGO)
{
	for (auto& [event, observers] : mySubscribers)
	{
		if (event == aEvent)
		{
			for (auto observer : observers)
			{
				observer->OnEvent();
			}
		}
	}
	if (aGO)
	{
		aGO->GetScene()->GetComponentSystem().GetComponentHandler<Engine::ScriptsComponent>().EachActiveComponent<Engine::ScriptsComponent>([&](Engine::ScriptsComponent* c)
			{
				auto graphInstance = c->GetGraphInstance();

				if (graphInstance != nullptr)
				{
					graphInstance->ExecuteNode("On Trigger Event", aEvent, aGO->GetUUID());
				}
			});
	}
}

TriggerEventManager& TriggerEventManager::GetInstance()
{
	return ourTriggerEventManager;
}
