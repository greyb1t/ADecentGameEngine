#pragma once
#include "Engine/GameObject/Components/Component.h"
#include "Engine\GameObject\GameObject.h"

class EnemySpawnPortal : public Component
{
public:
	COMPONENT(EnemySpawnPortal, "EnemySpawnPortal");

	EnemySpawnPortal() = default;
	EnemySpawnPortal(GameObject* aGameObject);
	virtual ~EnemySpawnPortal() = default;

	void Start() override;

	void Execute(Engine::eEngineOrder aOrder) override;

	void Update();

	void Reflect(Engine::Reflector& aReflector) override;

	void Render() override;

	void Init();

private:
	//bool myScaleUp = true;

	Engine::VFXComponent* myVFX = nullptr;
	GameObject* myParticleGO = nullptr;
	float myParticleOffset = 100.f;
	std::string myParticlePath = "";
	AnimationCurveRef myCurve;
	float myLifeTime = 2.f;
	float myCurrentLifeTime = 0.f;
};

