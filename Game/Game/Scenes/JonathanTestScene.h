#pragma once
#include "Engine/Scene/FolderScene.h"

class JonathanTestScene : public Engine::FolderScene
{
public:
	Result OnRuntimeInit() override;

	JonathanTestScene* Clone() const override;

	void Update(TimeStamp ts) override;

private:
	GameObject* myGameObject = nullptr;
};
