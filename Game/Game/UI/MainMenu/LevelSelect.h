#pragma once

#include "MenuComponent.h"

class LevelSelect : public MenuComponent
{
	COMPONENT(LevelSelect, "Level Select");
public:
	LevelSelect() = default;
	~LevelSelect() = default;

	void Start() override;
	void Reflect(Engine::Reflector& aReflector) override;

private:
	void PlayLevel(const std::string aPath);

	void EnableImagePreview(GameObject* aButtonObject);
	void DisableImagePreview(GameObject* aButtonObject);

private:
	Engine::GameObjectRef myVideoHandler;

};
