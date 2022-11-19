#pragma once
#include "Engine/GameObject/Components/Component.h"
#include "Engine\AnimationCurve\Curve.h"

namespace Engine
{
	class SpriteComponent;
}

class HUDDamageVignette : public Component
{
public:
	COMPONENT(HUDDamageVignette, "HUD Damage Vignette");

	void Start() override;
	void Reflect(Engine::Reflector& aReflector) override;
	void Execute(Engine::eEngineOrder aOrder) override;

private:
	void OnDamageObserve();

	HealthComponent* myHealthComponent = nullptr;
	Engine::SpriteComponent* myVignette = nullptr;
	Engine::SpriteComponent* myStayVignette = nullptr;

	AnimationCurveRef mySizeCurveRef;
	Engine::Curve mySizeCurve;

	AnimationCurveRef myStayCurveRef;
	Engine::Curve myStayCurve;

	float myResizeTime = 0.3f;
	float myResizeTimer = 0.0f;

	float myVignetteMaxScale = 1.5f;
	float myVignetteMinScale = 1.0f;
};
