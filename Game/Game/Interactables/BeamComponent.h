#pragma once
#include "Engine/GameObject/Components/Component.h"

class BeamComponent : public Component
{
public:
	COMPONENT(BeamComponent, "Beam Component");

	void Awake() override;
	void Start() override;

	void Reflect(Engine::Reflector& aReflector) override;

	void Execute(Engine::eEngineOrder aOrder) override;

private:
	Transform* myPlayer = nullptr;
	Engine::ModelComponent* myModel = nullptr;

	Vec3f myStartPos;

	float myMaxDistance = 1500.0f; //Max distance when seen
	float myMinDistance = 3000.0f; //Min distance to start dissapearing
	float myTargetHeight = 100.0f;
};
