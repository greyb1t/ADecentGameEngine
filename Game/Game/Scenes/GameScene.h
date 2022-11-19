#pragma once
#include "Engine/Scene/FolderScene.h"

class GameScene : public Engine::FolderScene
{
public:
	Result OnRuntimeInit() override;
	GameScene* Clone() const override;
	void Update(TimeStamp ts) override;
};