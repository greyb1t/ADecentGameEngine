#pragma once
#include "Engine\GameObject\Components\Component.h"
#include <Physics/Types.h>
#include "Engine/GameObject/Tag.h"
#include "Player/DamageInfo.h"

struct ExplosionInfo {
	float range = 100;
	float knockback = 100;
	float explosionTime = 1;
	LayerMask targetMask;
	AnimationCurveRef sizeCurve;
};

class Explosion :
	public Component
{
public:
	COMPONENT(Explosion, "Explosion");
	Explosion() = default;
	Explosion(GameObject* aGameObject);
	~Explosion() override;

	void Init(ExplosionInfo aExplosion, DamageInfo aDamage);
	void Execute(Engine::eEngineOrder aOrder) override;
private:
	void UpdateExplosion();
	void ScanDamage(float aRange);
private:
	std::vector<int> myHits;
	ExplosionInfo myExplosion;
	DamageInfo myDamage;
	float myTimer = 0;
	Vec3f myInitialSize;
	Engine::ModelComponent* myModel = nullptr;
};

