#include "pch.h"
#include "AudioOptions.h"

#include "Engine/GameObject/Components/2DComponents/SliderComponent.h"

void AudioOptions::Start()
{
	InitButtonCallbacks();
	InitSliderStartValues();
}

void AudioOptions::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myMasterVolumeRef, "Master Volume Slider");
	aReflector.Reflect(mySFXVolumeRef, "SFX Volume Slider");
	aReflector.Reflect(myMusicVolumeRef, "Music Volume Slider");
	aReflector.Reflect(myAmbienceVolumeRef, "Ambience Volume Slider");
}

void AudioOptions::InitButtonCallbacks()
{
	UI::AddEvent("MasterVolume", [this](GameObject* aKnobObject, const float aSliderValue) { SetVolume(aSliderValue, eVolumeType::MAIN); });
	UI::AddEvent("SFXVolume", [this](GameObject* aKnobObject, const float aSliderValue) { SetVolume(aSliderValue, eVolumeType::SFX); });
	UI::AddEvent("MusicVolume", [this](GameObject* aKnobObject, const float aSliderValue) { SetVolume(aSliderValue, eVolumeType::MUSIC); });
	UI::AddEvent("AmbienceVolume", [this](GameObject* aKnobObject, const float aSliderValue) { SetVolume(aSliderValue, eVolumeType::AMB); });
}

void AudioOptions::InitSliderStartValues()
{
	if (myMasterVolumeRef && myMasterVolumeRef.Get())
	{
		myMasterSlider = myMasterVolumeRef.Get()->GetComponent<Engine::SliderComponent>();
		myMasterSlider->SetValue(GetSettings().myMasterVolume);
	}

	if (mySFXVolumeRef && mySFXVolumeRef.Get())
	{
		mySFXSlider = mySFXVolumeRef.Get()->GetComponent<Engine::SliderComponent>();
		mySFXSlider->SetValue(GetSettings().mySFXVolume);
	}

	if (myMusicVolumeRef && myMusicVolumeRef.Get())
	{
		myMusicSlider = myMusicVolumeRef.Get()->GetComponent<Engine::SliderComponent>();
		myMusicSlider->SetValue(GetSettings().myMusicVolume);
	}

	if (myAmbienceVolumeRef && myAmbienceVolumeRef.Get())
	{
		myAmbienceSlider = myAmbienceVolumeRef.Get()->GetComponent<Engine::SliderComponent>();
		myAmbienceSlider->SetValue(GetSettings().myAmbienceVolume);
	}
}

void AudioOptions::SetVolume(const float aVolumeValue, const eVolumeType aType)
{
	switch (aType)
	{
	case eVolumeType::MAIN:
		GetSettings().myMasterVolume = aVolumeValue;
		break;
	case eVolumeType::SFX:
		GetSettings().mySFXVolume = aVolumeValue;
		break;
	case eVolumeType::MUSIC:
		GetSettings().myMusicVolume = aVolumeValue;
		break;
	case eVolumeType::AMB:
		GetSettings().myAmbienceVolume = aVolumeValue;
		break;
	default:
		break;
	}

	static auto am = AudioManager::GetInstance();
	am->SetVolume(aVolumeValue, aType);
}