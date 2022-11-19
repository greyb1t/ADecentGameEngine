#pragma once
#include "Engine/GameObject/Components/Component.h"
#include "Engine\GameObject\GameObject.h"

class EnemySpawnPortalFly : public Component
{
public:
	COMPONENT(EnemySpawnPortalFly, "EnemySpawnPortalFly");

	EnemySpawnPortalFly() = default;
	EnemySpawnPortalFly(GameObject* aGameObject);
	virtual ~EnemySpawnPortalFly() = default;

	void Start() override;

	void Execute(Engine::eEngineOrder aOrder) override;

	void Update();

	void Reflect(Engine::Reflector& aReflector) override;

	void Render() override;

	void Init();

private:
	Engine::VFXComponent* myVFX = nullptr;
	GameObject* myParticleGO = nullptr;
	float myParticleOffset = 100.f;
	//std::string myParticlePath = "";
	AnimationCurveRef myCurve;
	float myLifeTime = 9.f;
	float myCurrentLifeTime = 0.f;
};

