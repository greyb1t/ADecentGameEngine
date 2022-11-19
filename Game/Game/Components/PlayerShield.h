#pragma once
#include "Engine/GameObject/Components/Component.h"
#include "Engine\AnimationCurve\Curve.h"

namespace Engine
{
	class Collider;
	class AudioComponent;
}

class PlayerShield : public Component
{
public:
	COMPONENT(PlayerShield, "Player Shield Component");

	void Reflect(Engine::Reflector& aReflector) override;

	void Awake() override;

	void Start() override;

	void Execute(Engine::eEngineOrder aOrder) override;

	void Deactivate(bool aInstantly = false);

	bool IsPlayerInRadius();
	bool IsPlayerInMaxRadius();

	void SetDeactivateCallback(const std::function<void()>& aCallback);

	void KnockBackEnemy();
private:
	void ActiveUpdate();
	void UnActiveUpdate();

	void UpdateRadius(bool aShouldGrow);

	Engine::Collider* myCollider = nullptr;
	Transform* myPlayer = nullptr;

	Transform* myShieldModel = nullptr;

	float myDefaultRadius = 850.0f;
	float myRadius = 0.0f;

	bool myIsActive = true;

	float myLifeTime = 20.0f;
	float myLifeTimer = 0.0f;

	float myDeactivateDelay = 0.5f;
	float myDeactivateTimer = 0.0f;

	float myResizeTime = 0.5f;
	float myResizeTimer = 0.0f;

	float myModelStartScale = 9.0f;

	bool myHasPlayerClose = false;

	AnimationCurveRef mySizeCurveRef;
	Engine::Curve mySizeCurve;

	std::function<void()> myDeactivateCallback = nullptr;

	Engine::AudioComponent* myAudio = nullptr;

	HealthComponent* myPlayerHealth = nullptr;
};
