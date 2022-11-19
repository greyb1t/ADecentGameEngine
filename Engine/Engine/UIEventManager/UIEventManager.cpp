#include "pch.h"
#include "UIEventManager.h"
#include "Engine/GameObject/GameObject.h"

UIEventManager::UIEventManager()
{

}

void UIEventManager::AddEvent(const std::string& aName, const std::function<void(GameObject*)>& aCallbackFunction)
{
	myCallbackButtonEvents[aName] = aCallbackFunction;
}

void UIEventManager::AddEvent(const std::string& aName, const std::function<void(GameObject*, const float)>& aCallbackFunction)
{
	myCallbackSliderEvents[aName] = aCallbackFunction;
}

void UIEventManager::CallEvent(const std::string& aName, GameObject* aButtonObject, bool aIsInteractionKey)
{
	if (!ButtonExists(aName))
	{
		LOG_ERROR(LogType::Components) << "Callback event with name " << aName << " does not exist.";
		return;
	}

	if (myCallbackButtonEvents[aName])
	{
		if (aIsInteractionKey)
		{
			if (ourButtonReleased)
			{
				myCallbackButtonEvents[aName](aButtonObject);
			}
		}
		else 
		{
			myCallbackButtonEvents[aName](aButtonObject);
		}
	}
}

void UIEventManager::CallEvent(const std::string& aName, GameObject* aKnobObject, const float aSliderValue, bool aIsInteractionKey)
{
	if (!SliderExists(aName))
	{
		LOG_ERROR(LogType::Components) << "Callback event with name " << aName << " does not exist.";
		return;
	}

	if (myCallbackSliderEvents[aName])
	{
		if (aIsInteractionKey)
		{
			if (ourButtonReleased)
			{
				myCallbackSliderEvents[aName](aKnobObject, aSliderValue);
			}
		}
		else
		{
			myCallbackSliderEvents[aName](aKnobObject, aSliderValue);
		}
	}
}

void UIEventManager::OnCallEvent(bool aIsInteractionKey)
{
	if (aIsInteractionKey)
	{
		ourButtonReleased = false;
	}
}

void UIEventManager::Update()
{
	if (!ourButtonReleased)
	{
		ourButtonReleased = true;
	}
}

bool UIEventManager::ButtonExists(const std::string& aName)
{
	auto iter = myCallbackButtonEvents.find(aName);
	if (iter != myCallbackButtonEvents.end())
	{
		return true;
	}
	return false;
}

bool UIEventManager::SliderExists(const std::string& aName)
{
	auto iter = myCallbackSliderEvents.find(aName);
	if (iter != myCallbackSliderEvents.end())
	{
		return true;
	}
	return false;
}

void UIEventManager::SetObjective(const std::string& aObjective)
{
	if (myObjectiveCallback)
	{
		myObjectiveCallback(aObjective);
	}
	else
	{
		LOG_ERROR(LogType::Game) << "SetObjective() COULDN'T FIND THE HUD OBJECTIVE ELEMENT";
	}
}

void UIEventManager::SetObjectiveObserver(const std::function<void(const std::string&)>& aCallbackFunction)
{
	myObjectiveCallback = aCallbackFunction;
}

void UIEventManager::SetFadeOutObserver(const std::function<void(const std::function<void()>& aCallback, bool aShouldFadeBackIn)>& aStartFade)
{
	myFadeOutCallback = aStartFade;
}

void UIEventManager::RemoveFadeOutObserver()
{
	myFadeOutCallback = nullptr;
}

void UIEventManager::TryToFadeOut(const std::function<void()>& aCallback, bool aShouldFadeBackIn)
{
	if (myFadeOutCallback)
	{
		myFadeOutCallback(aCallback, aShouldFadeBackIn);
	}
	else if(aCallback)
	{
		aCallback();
	}
}

void UIEventManager::SetIsPaused(const bool aIsPaused)
{
	myIsPaused = aIsPaused;
}

bool UIEventManager::GetIsPaused()
{
	return myIsPaused;
}
