#pragma once

#include "Engine/Scene/FolderScene.h"

class ScriptedEventsTestScene : public Engine::FolderScene
{
public:
	Result OnRuntimeInit() override;

	ScriptedEventsTestScene* Clone() const override;

	void Update(TimeStamp ts) override;

private:
	GameObject* myGameObject = nullptr;
};
