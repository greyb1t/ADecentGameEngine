#pragma once

#include "Engine/Scene/FolderScene.h"

class FilipTestScene : public Engine::FolderScene
{
public:
	Result OnRuntimeInit() override;

	FilipTestScene* Clone() const override;

	void Update(TimeStamp ts) override;

private:
	GameObject* myGameObject = nullptr;
};
