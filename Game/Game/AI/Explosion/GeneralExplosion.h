#pragma once
#include "Engine\GameObject\GameObject.h"
#include "Engine/VFX/Description/VFXDescription.h"

class GeneralExplosion : public Component
{
public:
	COMPONENT(GeneralExplosion, "GeneralExplosion");

	GeneralExplosion() = default;
	GeneralExplosion(GameObject* aGameObject);
	//GeneralExplosion(GameObject* aGameObject, std::string aPath);
	GeneralExplosion(GameObject* aGameObject, VFX::VFXDescription& aDescription);
	virtual ~GeneralExplosion() = default;

	void Start() override;

	void Execute(Engine::eEngineOrder aOrder) override;

	void Update();

	void Reflect(Engine::Reflector& aReflector) override;

	void Render() override;

private:
	Engine::VFXComponent* myVFX = nullptr;
	VFX::VFXDescription* myVFXDes = nullptr;
	std::string myVFXPath = "";
	float myLifeTimer = 0.f;
	float myLifeTime = 5.f;

};

