#pragma once
#include "Engine\GameObject\Components\Component.h"
#include "Engine/GameObject/GameObject.h"

class MoveComponent : public Component
{
public:
	COMPONENT(MoveComponent, "Move Component");

	MoveComponent() = default;
	MoveComponent(GameObject* aGameObject, const float& aDamage, const float& aSpeed, const float& aDelay, const float& aLifeTime, const float& aAcceleration, const float& aRadius);

	void Start() override;

	void Init(const Vec3f& aPos);

	void Execute(Engine::eEngineOrder aOrder) override;

	void Reflect(Engine::Reflector& aReflector) override;

protected:

private:
	GameObject* myFrom;

	Vec3f myMovePart;
	Vec3f myTargetPos;

	float myProgress = 0;
	float myDamage = 0;
	float mySpeed = 0;
	float myDelay = 0;
	float myDelayTimer = 0;
	float myLifeTime = 0;
	float myRadius = 0;
	float mySmoothing = 0;
	float myAcceleration = 0;
};