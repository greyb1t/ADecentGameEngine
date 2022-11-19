#include "pch.h"
#include "GeneralOptions.h"

#include "Engine/GameObject/Components/2DComponents/SliderComponent.h"

void GeneralOptions::Start()
{
	InitButtonCallbacks();
	InitSliderStartValues();
}

void GeneralOptions::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(mySensitivityRef, "Mouse Sensitivity Slider");
}

void GeneralOptions::InitButtonCallbacks()
{
	UI::AddEvent("MouseSensitivity", [this](GameObject* aKnobObject, const float aSliderValue) { MouseSensitivity(aSliderValue); });
}

void GeneralOptions::InitSliderStartValues()
{
	if (mySensitivityRef && mySensitivityRef.Get())
	{
		mySensitivitySlider = mySensitivityRef.Get()->GetComponent<Engine::SliderComponent>();
		mySensitivitySlider->SetValue(GetSettings().myMouseSensitivityMultiplier);
	}
}

void GeneralOptions::MouseSensitivity(const float aSliderValue)
{
	GetSettings().myMouseSensitivityMultiplier = aSliderValue;
}
