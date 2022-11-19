#pragma once

class GameObject;

class UIEventManager
{
public:
	UIEventManager();

	static void AddEvent(const std::string& aName, const std::function<void(GameObject*)>& aCallbackFunction);
	static void AddEvent(const std::string& aName, const std::function<void(GameObject*, const float)>& aCallbackFunction);

	static void CallEvent(const std::string& aName, GameObject* aButtonObject, bool aIsInteractionKey = false);
	static void CallEvent(const std::string& aName, GameObject* aKnobObject, const float aSliderValue, bool aIsInteractionKey = false);
	static void OnCallEvent(bool aIsInteractionKey = false);

	static void Update();

	static bool ButtonExists(const std::string& aName);
	static bool SliderExists(const std::string& aName);

	static void SetObjective(const std::string& aObjective);
	static void SetObjectiveObserver(const std::function<void(const std::string&)>& aCallbackFunction);

	static void SetFadeOutObserver(const std::function<void(const std::function<void()>& /*aCallback*/, bool aShouldFadeBackIn)>& aStartFade);
	static void RemoveFadeOutObserver();
	static void TryToFadeOut(const std::function<void()>& aCallback, bool aShouldFadeBackIn = false);

	static void SetIsPaused(const bool aIsPaused);
	static bool GetIsPaused();

private:
	inline static UIEventManager* ourInstance = nullptr;

	inline static bool ourButtonReleased = true;
	inline static std::unordered_map<std::string, std::function<void(GameObject*)>> myCallbackButtonEvents;
	inline static std::unordered_map<std::string, std::function<void(GameObject*, const float)>> myCallbackSliderEvents;

	inline static std::function<void(const std::string&)> myObjectiveCallback = nullptr;

	inline static std::function<void(const std::function<void()>& /*aCallback*/, bool /*aShouldFadeBackIn*/)> myFadeOutCallback = nullptr;

	inline static bool myIsPaused = false;
};
