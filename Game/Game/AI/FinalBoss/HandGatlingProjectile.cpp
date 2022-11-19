#include "pch.h"
#include "HandGatlingProjectile.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/ResourceManagement/Resources/AnimationCurveResource.h"
#include "Engine/AnimationCurve/Curve.h"
#include "Components/HealthComponent.h"
#include "AI/PollingStation/PollingStationComponent.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/VFXComponent.h"
#include "Constants.h"

void FB::HandGatlingProjectile::Launch(const Vec3f& aFrom, const Vec3f& aDirection, const float aDistance)
{
	myDestroyMyselfTimer = TickTimer::FromSeconds(mySecondsUntilSelfDestroy);
	myDirection = aDirection.GetNormalized();

	GetTransform().SetPosition(aFrom);
	GetTransform().LookAt(GetTransform().GetPosition() + myDirection);

	auto r2 = GetTransform().GetRotation();
	r2 = r2 * Quatf(Vec3f(0.f, Math::PI, 0.f));
	GetTransform().SetRotation(r2);

	DamagePlayer(aFrom, myDirection, aDistance);
	CreateImpactVFXOnGround(aFrom, myDirection, aDistance);

	GDebugDrawer->DrawLine3D(DebugDrawFlags::Gameplay, aFrom, aFrom + myDirection * aDistance, 5.f, Vec4f(1, 1, 0, 1));
}

void FB::HandGatlingProjectile::SetDamage(const float aDamage)
{
	myDamage = aDamage;
}

void FB::HandGatlingProjectile::SetSecondsUntilSelfDestroy(const float aSeconds)
{
	mySecondsUntilSelfDestroy = aSeconds;
}

void FB::HandGatlingProjectile::OnLostTarget()
{

}

void FB::HandGatlingProjectile::OnDamaged(const float aDamage)
{

}

bool FB::HandGatlingProjectile::OnDeath(float aDamage)
{
	myGameObject->Destroy();

	// what does return do?
	return true;
}

void FB::HandGatlingProjectile::OnCollidePlayer(GameObject* aPlayerGameObject)
{
	myGameObject->Destroy();
}

void FB::HandGatlingProjectile::Start()
{
}

void FB::HandGatlingProjectile::Execute(Engine::eEngineOrder aOrder)
{
	/*
	auto pos = GetTransform().GetPosition();
	pos += myDirection * mySpeed * Time::DeltaTime;
	GetTransform().SetPosition(pos);
	*/

	myDestroyMyselfTimer.Tick(Time::DeltaTime);

	if (myDestroyMyselfTimer.JustFinished())
	{
		// Self destroy
		myGameObject->Destroy();
	}
}

void FB::HandGatlingProjectile::Reflect(Engine::Reflector& aReflector)
{
	Component::Reflect(aReflector);

	aReflector.Reflect(mySecondsUntilSelfDestroy, "Seconds Until Self Destroy");

	aReflector.Reflect(myDamage, "Damage");

	aReflector.Reflect(myImpactVFX, "Impact VFX");
}

void FB::HandGatlingProjectile::DamagePlayer(const Vec3f& aFrom, const Vec3f& aDirection, const float aDistance)
{
	RayCastHit hit;
	auto scene = myGameObject->GetScene();
	if (scene->RayCast(aFrom, aDirection, aDistance, eLayer::GROUND | eLayer::DEFAULT | eLayer::PLAYER_DAMAGEABLE, hit))
	{
		auto scale = GetTransform().GetScale();
		scale.z = hit.Distance / 100.f;
		GetTransform().SetScale(scale);

		if (auto pollingStation = myGameObject->GetSingletonComponent<PollingStationComponent>())
		{
			if (auto gameObject = pollingStation->GetPlayer())
			{
				if (hit.GameObject == gameObject)
				{
					if (auto health = gameObject->GetComponent<HealthComponent>())
					{
						health->ApplyDamage(myDamage);
						LOG_INFO(LogType::Game) << "Gatling laser damaged player";
					}
				}
			}
		}
	}
}

void FB::HandGatlingProjectile::CreateImpactVFXOnGround(const Vec3f& aFrom, const Vec3f& aDirection, const float aDistance)
{
	RayCastHit hit;
	auto scene = myGameObject->GetScene();
	if (scene->RayCast(aFrom, aDirection, Constants::RaycastDistance, eLayer::GROUND | eLayer::DEFAULT, hit, false))
	{
		if (myImpactVFX && myImpactVFX->IsValid())
		{
			auto g = myGameObject->GetScene()->AddGameObject<GameObject>();
			g->GetTransform().SetPosition(hit.Position);
			auto myVFX = g->AddComponent<Engine::VFXComponent>(myImpactVFX->Get());
			myVFX->Play();
			myVFX->AutoDestroy();
		}
		else
		{
			LOG_ERROR(LogType::Game) << "Missing impact VFX";
		}
	}
}
