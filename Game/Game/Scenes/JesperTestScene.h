#pragma once
#include "Engine/Scene/FolderScene.h"

namespace NavMesh
{
	class NavMesh;
}

class JesperTestScene : public Engine::FolderScene
{
public:
	Result OnRuntimeInit() override;

	JesperTestScene* Clone() const override;

	void Update(TimeStamp ts) override;

private:
	GameObject* myGameObject = nullptr;
	GameObject* myGameObject2 = nullptr;
};
