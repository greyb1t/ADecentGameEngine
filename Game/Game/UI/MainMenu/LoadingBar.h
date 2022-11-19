#pragma once

#include "Engine/GameObject/Components/Component.h"

class LoadingBar : public Component
{
	COMPONENT(LoadingBar, "Loading Bar");
public:
	LoadingBar() = default;
	~LoadingBar() = default;

	void Awake() override;

	void Execute(Engine::eEngineOrder aOrder) override;

private:
	Engine::SpriteComponent* mySprite = nullptr;
	Engine::TextComponent* myText = nullptr;

};

