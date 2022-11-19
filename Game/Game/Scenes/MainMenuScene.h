#pragma once

#include "Engine/Scene/FolderScene.h"

class MainMenuScene : public Engine::FolderScene
{
public:
	Result OnRuntimeInit() override;

	MainMenuScene* Clone() const override;

	void Update(TimeStamp ts) override;

private:
	GameObject* myGameObject = nullptr;
};
