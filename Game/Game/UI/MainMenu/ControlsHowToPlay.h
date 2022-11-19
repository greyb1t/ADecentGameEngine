#pragma once

#include "MenuComponent.h"

class ControlsHowToPlay : public MenuComponent
{
	COMPONENT(ControlsHowToPlay, "ControlsHowToPlay");
public:
	ControlsHowToPlay() = default;
	~ControlsHowToPlay() = default;

	void Start() override;
	void Reflect(Engine::Reflector& aReflector) override;

private:

};

