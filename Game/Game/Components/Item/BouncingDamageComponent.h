#pragma once
#include "Engine/GameObject/Components/Component.h"
class BouncingDamageComponent : public Component
{
	COMPONENT(BouncingDamageComponent, "Bouncing Damage");

public:
	~BouncingDamageComponent() = default;

	void Init(Transform* aTarget, int someTargets, float aRadius, float aDamage, float aDelay, float aVFXDuration);
	//void Init(const BouncingDamageComponent& aComp, Transform* aTarget);
	void Reflect(Engine::Reflector& aReflector) override;
	void Execute(Engine::eEngineOrder aOrder) override;
	void Chain();

	//void OnTargetDeath(float);

private:
	void SpawnLightningEffects(Transform* aNewTarget);

private:
	VFXRef myImpactEffect;
	GameObjectPrefabRef myLightningVFX;

	CU::Vector3f myStartPosition;
	Weak<GameObject> myCurrentGameObject;

	GameObject* myVFXObject = nullptr;

	std::vector<Transform*> myHitTargets;
	float myChainTimer = -1.f;

	int myRemainingTargets = 0;
	float myRadius = 0.f;
	float myDamage = 0.f;
	float myDelay = 0.f;
	float myLightningDuration = 0.f;
};

