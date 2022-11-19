#include "pch.h"
#include "EnemyVFXRef.h"

#include "Engine/Shortcuts.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/Scene/FolderScene.h"
#include "Engine/Reflection/Reflector.h"
#include "Engine/GameObject/Components/PointLightComponent.h"

EnemyVFXRef::EnemyVFXRef(GameObject* aGameObject) :
	Component(aGameObject)
{
}

void EnemyVFXRef::Start()
{
	Component::Start();
}

void EnemyVFXRef::Reflect(Engine::Reflector& aReflector)
{
	Component::Reflect(aReflector);

	aReflector.Header("AirPop");
	aReflector.Reflect(myAirPopProjectileTrail, "Air Pop Projectile Trail");
	aReflector.Header("AirTank");
	aReflector.Reflect(myAirTankProjectileTrail, "Air Tank Projectile Trail");
	aReflector.Reflect(myAirTankProjectileExplode, "Air Tank Projectile Explode");
	aReflector.Header("Mortar");
	aReflector.Reflect(myMotarProjectileExplode, "Motar Projectile Explode");
	aReflector.Reflect(myMotarProjectileTrail, "Motar Projectile Trail");
	aReflector.Header("Runner");
	aReflector.Reflect(myRunnerExplode, "Runner Explode");
	aReflector.Reflect(myRunnerFuse, "Runner Fuse");
	aReflector.Header("Ground Tank");
	aReflector.Reflect(myGroundTankLaserSight, "Ground Tank Laser Sight");
	aReflector.Reflect(myGroundTankSlam, "Ground Tank Slam");
	aReflector.Reflect(myLaserHit, "Ground Tank Laser Hit");
	aReflector.Header("Slime");
	aReflector.Reflect(myPoisonCloud, "Slime Poison Cloud");	
	aReflector.Header("Ground Popcorn");
	aReflector.Reflect(myGroundPopcornProjectile, "Ground Popcorn Projectile");
	aReflector.Header("General");
	aReflector.Reflect(myEnemyFlyDeathExplosion, "Enemy Fly Death Explosion");
	aReflector.Reflect(myEnemyFlyPortal, "Enemy Fly Portal");
	aReflector.Reflect(myEnemyGroundPortal, "Enemy Ground Portal");
	aReflector.Reflect(myEnemyDeathDecay, "Enemy Death Decay");


}

void EnemyVFXRef::Execute(Engine::eEngineOrder aOrder)
{
	Component::Execute(aOrder);
}

void EnemyVFXRef::Render()
{
	Component::Render();
}

VFXRef& EnemyVFXRef::GetAirPopProjectileTrail()
{
	return myAirPopProjectileTrail;
}

VFXRef& EnemyVFXRef::GetAirTankProjectileTrail()
{
	return myAirTankProjectileTrail;
}

VFXRef& EnemyVFXRef::GetAirTankProjectileExplode()
{
	return myAirTankProjectileExplode;
}

VFXRef& EnemyVFXRef::GetMotarProjectileExplode()
{
	return myMotarProjectileExplode;
}

VFXRef& EnemyVFXRef::GetRunnerExplode()
{
	return myRunnerExplode;
}

VFXRef& EnemyVFXRef::GetEnemyFlyDeathExplosion()
{
	return myEnemyFlyDeathExplosion;
}

VFXRef& EnemyVFXRef::GetMotarProjectileTrail()
{
	return myMotarProjectileTrail;
}

VFXRef& EnemyVFXRef::GetEnemyFlyPortal()
{
	return myEnemyFlyPortal;
}

VFXRef& EnemyVFXRef::GetEnemyGroundPortal()
{
	return myEnemyGroundPortal;
}

VFXRef& EnemyVFXRef::GetmyRunnerFuse()
{
	return myRunnerFuse;
}

VFXRef& EnemyVFXRef::GetGroundTankLaserSight()
{
	return myGroundTankLaserSight;
}

VFXRef& EnemyVFXRef::GetGroundTankSlam()
{
	return myGroundTankSlam;
}

VFXRef& EnemyVFXRef::GetSlimePoison()
{
	return myPoisonCloud;
}

VFXRef& EnemyVFXRef::GetGroundPopcornProjectile()
{
	return myGroundPopcornProjectile;
}

VFXRef& EnemyVFXRef::GetLaserHit()
{
	return myLaserHit;
}

VFXRef& EnemyVFXRef::GetEnemyDeathDecay()
{
	return myEnemyDeathDecay;
}
