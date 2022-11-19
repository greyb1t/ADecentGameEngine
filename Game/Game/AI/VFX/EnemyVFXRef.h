#pragma once
#include "Engine/GameObject/Components/Component.h"

class EnemyVFXRef : public Component
{
public:
	COMPONENT_SINGLETON(EnemyVFXRef, "EnemyVFXRef")

	EnemyVFXRef() = default;
	EnemyVFXRef(GameObject * aGameObject);
	virtual ~EnemyVFXRef() = default;

	void Start() override;

	void Reflect(Engine::Reflector& aReflector) override;

	void Execute(Engine::eEngineOrder aOrder) override;
	void Render() override;

	VFXRef& GetAirPopProjectileTrail();
	VFXRef& GetAirTankProjectileTrail();
	VFXRef& GetAirTankProjectileExplode();
	VFXRef& GetMotarProjectileExplode();
	VFXRef& GetRunnerExplode();
	VFXRef& GetEnemyFlyDeathExplosion();
	VFXRef& GetMotarProjectileTrail();
	VFXRef& GetEnemyFlyPortal();
	VFXRef& GetEnemyGroundPortal();
	VFXRef& GetmyRunnerFuse();
	VFXRef& GetGroundTankLaserSight();
	VFXRef& GetGroundTankSlam();
	VFXRef& GetSlimePoison();
	VFXRef& GetGroundPopcornProjectile();
	VFXRef& GetLaserHit();
	VFXRef& GetEnemyDeathDecay();
private:
	VFXRef myAirPopProjectileTrail;

	VFXRef myAirTankProjectileTrail;
	VFXRef myAirTankProjectileExplode;

	VFXRef myMotarProjectileExplode;
	VFXRef myMotarProjectileTrail;

	VFXRef myRunnerExplode;
	VFXRef myRunnerFuse;

	VFXRef myEnemyFlyDeathExplosion;

	VFXRef myEnemyFlyPortal;
	VFXRef myEnemyGroundPortal;

	VFXRef myGroundTankLaserSight;
	VFXRef myGroundTankSlam;

	VFXRef myPoisonCloud;

	VFXRef myGroundPopcornProjectile;

	VFXRef myLaserHit;

	VFXRef myEnemyDeathDecay;
};
