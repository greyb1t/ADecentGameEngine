#pragma once

#include "Engine/GameObject/Components/Component.h"
#include "Engine/UIEventManager/UIEventManager.h"

namespace Engine
{
	class AudioComponent : public Component
	{
	public:
		COMPONENT_COPYABLE(AudioComponent, "AudioComponent");

		struct AudioEvent
		{
			std::string myEventName = "";
			std::string myEventPath = "";
		};
		AudioComponent() = default;
		AudioComponent(GameObject* aGameObject);

		virtual ~AudioComponent();

		void Start() override;
		void Execute(eEngineOrder aOrder) override;
		void StopIdle();
		void PlayIdle();
		void Reflect(Reflector& aReflector) override;
		
		void ReleaseAudio();

		void AddEvent(const std::string& aEventName, const std::string& aEventPath);

		void ChangeParameter(const std::string& aParameterName, float aValue);
		void ChangeParameter(int aUUID, const std::string& aEventPath, const std::string& aParameterName, float aValue);

		void SetIdleSoundTimes(float aMin, float aMax);
		void SetIdleSoundPath(const std::string& aPath);
		bool IsInstanceLoaded(const std::string& aPath);

		void PlayEvent(const std::string& aEventName);
		void PauseAll();
		void PauseEvent(const std::string& aEventPath);
		void StopAll();
		void StopEvent(const std::string& aEvent);

		const std::string& GetEventPath(const std::string& aEvent);
	private:
		void PlayIdleSound();
	private:
		float myVolume = 1.0f;
		float myParameter = 0.0f;

		float myIdleSoundTimer = 0.0f;
		float myIdleSoundTime = -1.0f;
		float myIdleSoundTimeMin = -1.0f;
		float myIdleSoundTimeMax = -1.0f;
		std::string myIdleSoundPath = "NONE";
		bool myShouldPlay = true;
		bool myIdleLoop = false;
		//FIRST KEY SECOND VALUE
		std::vector<AudioEvent> myEvents;
		std::vector<std::string> myEventNames;
	};
}