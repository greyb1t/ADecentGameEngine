#pragma once

#include "MenuComponent.h"

class AbilitiesHowToPlay : public MenuComponent
{
	COMPONENT(AbilitiesHowToPlay, "AbilitiesHowToPlay");
public:
	AbilitiesHowToPlay() = default;
	~AbilitiesHowToPlay() = default;

	void Start() override;
	void Reflect(Engine::Reflector& aReflector) override;

private:

};

