#pragma once

#include "MenuComponent.h"

class MainMenu : public MenuComponent
{
	COMPONENT(MainMenu, "Main Menu");
public:
	MainMenu() = default;
	~MainMenu() = default;

	void Start() override;
	void Reflect(Engine::Reflector& aReflector) override;

private:
	void Play(GameObject* aButtonObject);
	void LevelSelect(GameObject* aButtonObject);
	void HowToPlay(GameObject* aButtonObject);
	void Options(GameObject* aButtonObject);
	void Credits(GameObject* aButtonObject);
	void ExitGame(GameObject* aButtonObject);

private:

};

