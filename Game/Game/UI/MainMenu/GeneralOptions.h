#pragma once

#include "Options.h"

namespace Engine
{
	class SliderComponent;
}

class GeneralOptions : public Options
{
	COMPONENT(GeneralOptions, "GeneralOptions");
public:
	GeneralOptions() = default;
	~GeneralOptions() = default;

	void Start() override;
	void Reflect(Engine::Reflector& aReflector) override;

private:
	void InitButtonCallbacks();
	void InitSliderStartValues();

	void MouseSensitivity(const float aSliderValue);

	Engine::GameObjectRef mySensitivityRef;

	Engine::SliderComponent* mySensitivitySlider = nullptr;
	
	
};