#include "pch.h"
#include "HandHomingProjectileLauncher.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/ResourceManagement/Resources/GameObjectPrefabResource.h"
#include "Engine/ResourceManagement/Resources/AnimationCurveResource.h"
#include "Engine/GameObject/Prefab/GameObjectPrefab.h"
#include "HandHomingProjectile.h"
#include "AI/PollingStation/PollingStationComponent.h"
#include "Engine/AnimationCurve/Curve.h"

void FB::HandHomingProjectileLauncher::Start()
{
	if (!myProjectilePrefab->IsValid())
	{
		LOG_ERROR(LogType::Game) << "HandHomingProjectileLauncher is missing prefab";
		myGameObject->Destroy();
		return;
	}

	if (!myImplodeScale->IsValid())
	{
		LOG_ERROR(LogType::Game) << "HandHomingProjectileLauncher is missing implode curve";
		myGameObject->Destroy();
		return;
	}

	myImplodeTimer = TickTimer::FromSeconds(1.f);

	// Set default scale
	const float scale = myImplodeScale->Get().Evaluate(0.f);
	GetTransform().SetScale(scale);
}

void FB::HandHomingProjectileLauncher::Execute(Engine::eEngineOrder aOrder)
{
	myImplodeTimer.Tick(Time::DeltaTime);

	const float scale = myImplodeScale->Get().Evaluate(myImplodeTimer.Percent());

	GetTransform().SetScale(scale);

	// if (myImplodeTimer.JustFinished())
	// {
	// 	Implode();
	// }
}

void FB::HandHomingProjectileLauncher::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myProjectilesCount, "Projectile Count");
	aReflector.Reflect(myProjectilePrefab, "Projectile Prefab");
	aReflector.Reflect(mySphereOffsetLength, "Sphere Offset Length");
	aReflector.Reflect(myImplodeScale, "Implode Scale Curve");
}

void FB::HandHomingProjectileLauncher::SetProjectileDamage(const float aDamage)
{
	myProjectileDamage = aDamage;
}

void FB::HandHomingProjectileLauncher::SetProjectilesCount(const int aProjectilesCount)
{
	myProjectilesCount = aProjectilesCount;
}

void FB::HandHomingProjectileLauncher::SetSphereOffsetLength(const float aOffsetLength)
{
	mySphereOffsetLength = aOffsetLength;
}

void FB::HandHomingProjectileLauncher::SetSecondsUntilSelfDestroy(const float aSeconds)
{
	mySecondsUntilSelfDestroy = aSeconds;
}

void FB::HandHomingProjectileLauncher::SetProjectileSpeed(const float aSpeed)
{
	myProjectileSpeed = aSpeed;
}

void FB::HandHomingProjectileLauncher::Implode()
{
	// Spawn the projectiles using a bezier curve in a circle
	for (int i = 0; i < myProjectilesCount; ++i)
	{
		const float percent = static_cast<float>(i) / static_cast<float>(myProjectilesCount);

		const float radius = Math::PI * 2.f * percent;

		const float x = cos(radius) * mySphereOffsetLength;
		const float y = sin(radius) * mySphereOffsetLength;


		if (myProjectilePrefab)
		{
			auto& g = myProjectilePrefab->Get().Instantiate(*myGameObject->GetScene());

			auto offset = Vec3f(x, y, 0.f) * GetTransform().GetRotation();

			if (auto homingProjectile = g.GetComponent<HandHomingProjectile>())
			{
				const Vec3f from = GetTransform().GetPosition();

				auto polling = myGameObject->GetSingletonComponent<PollingStationComponent>();
				auto player = polling->GetPlayer()->GetTransform().GetGameObjectShared();

				homingProjectile->LaunchTowards(from, player, offset, i);
				homingProjectile->SetDamage(myProjectileDamage);
				homingProjectile->SetProjectileSpeed(myProjectileSpeed);
				homingProjectile->SetSecondsUntilSelfDestroy(mySecondsUntilSelfDestroy);
			}
			else
			{
				LOG_ERROR(LogType::Game) << "Homing Laser prefab is missing HandHomingProjectile component";
			}
		}
	}

	myGameObject->Destroy();
}
