#include "pch.h"
#include "GeneralEventManager.h"
#include "GeneralEventListener.h"
#include "Engine/GameObject/GameObject.h"
#include <Engine/GameObject/Components/ScriptsComponent.h>

GeneralEventManager GeneralEventManager::ourGeneralEventManager;

GeneralEventManager::GeneralEventManager()
{
	for (int i = 0; i < static_cast<int>(GeneralEvent::Count); ++i)
	{
		mySubscribers.push_back({ });
	}
}

void GeneralEventManager::Subscribe(GeneralEventListener* aObserver, GeneralEvent aEvent)
{
	mySubscribers[static_cast<int>(aEvent)].push_back(aObserver);
}

void GeneralEventManager::Unsubscribe(GeneralEventListener* aObserver, GeneralEvent aEvent)
{
	std::vector<GeneralEventListener*>& messageObservers = mySubscribers[static_cast<int>(aEvent)];

	auto result = std::find(messageObservers.begin(), messageObservers.end(), aObserver);

	if (result != messageObservers.end())
	{
		messageObservers.erase(result);
	}
}

void GeneralEventManager::SendLetter(const GeneralEvent& aEvent, GameObject* aGameObject)
{
	for (auto& subscriber : mySubscribers[static_cast<int>(aEvent)])
	{
		if (aGameObject->IsActive())
		{
			if (subscriber != nullptr)
				subscriber->OnEvent(aGameObject);
		}
	}
}

GeneralEventManager& GeneralEventManager::GetInstance()
{
	return ourGeneralEventManager;
}
