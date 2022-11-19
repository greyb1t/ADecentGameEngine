#pragma once

#include "Engine/GameObject/Components/Component.h"
#include "Engine/GameObject/GameObject.h"

struct SineWave : public Engine::Reflectable
{
	float myLowestValue = 0.5f;
	float myHighestValue = 1.0f;
	
	float mySpeed = 1.25f;
	float myMultiplier = 1.0f;

	float myTotalTime = 0.0f;

	void Reflect(Engine::Reflector& aReflector) override;
};

class UI_Fade : public Component
{
	COMPONENT(UI_Fade, "UI Fade");
public:
	UI_Fade() = default;
	~UI_Fade() = default;

	void Awake() override;
	void Start() override;

	void Reflect(Engine::Reflector& aReflector) override;

	void Execute(Engine::eEngineOrder aOrder) override;

private:
	Engine::SpriteComponent* mySprite = nullptr;
	Engine::TextComponent* myText = nullptr;

	bool myDoScale = false;
	SineWave myScaleSine;

	bool myDoAlpha = false;
	SineWave myAlphaSine;

};
