#pragma once
#include "Engine/Scene/FolderScene.h"

class SimonTestScene : public Engine::FolderScene
{
public:
	Result OnRuntimeInit() override;

	SimonTestScene* Clone() const override;

	void Update(TimeStamp ts) override;

private:
};
