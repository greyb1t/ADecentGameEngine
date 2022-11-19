#pragma once
#include "Engine\GameObject\Components\Component.h"
class FractureComponent :
    public Component
{
public:
	FractureComponent(GameObject* aGameObject, float aLifeTime = 10.f, float aDisappearThreshold = .92f, float aLifetimeRandomMagnitude = 0.2f);

	void Execute(Engine::eEngineOrder aOrder) override;
private:
	float myLifeTime = 10.f;
	float myDisappearThreshold = .92f;
	float myTime = 0.f;

	float myRandomizeModifier = 0.2f;

	CU::Vector3f myOriginalScale;
};