#pragma once

#include "Options.h"

#include "Engine/AudioManager.h"

namespace Engine
{
	class SliderComponent;
}

class AudioOptions : public Options
{
	COMPONENT(AudioOptions, "AudioOptions");
public:
	AudioOptions() = default;
	~AudioOptions() = default;

	void Start() override;
	void Reflect(Engine::Reflector& aReflector) override;

private:
	void InitButtonCallbacks();
	void InitSliderStartValues();

	void SetVolume(const float aVolumeValue, const eVolumeType aType);

	Engine::GameObjectRef myMasterVolumeRef;
	Engine::GameObjectRef mySFXVolumeRef;
	Engine::GameObjectRef myMusicVolumeRef;
	Engine::GameObjectRef myAmbienceVolumeRef;

	Engine::SliderComponent* myMasterSlider = nullptr;
	Engine::SliderComponent* mySFXSlider = nullptr;
	Engine::SliderComponent* myMusicSlider = nullptr;
	Engine::SliderComponent* myAmbienceSlider = nullptr;

};

