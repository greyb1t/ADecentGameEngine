#pragma once

#include "Engine/Scene/FolderScene.h"

class MarkusTestScene : public Engine::FolderScene
{
public:
	Result OnRuntimeInit() override;

	MarkusTestScene* Clone() const override;

	void Update(TimeStamp ts) override;

private:
};
