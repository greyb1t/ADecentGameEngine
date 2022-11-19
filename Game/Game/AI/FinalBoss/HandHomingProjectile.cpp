#include "pch.h"
#include "HandHomingProjectile.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/ResourceManagement/Resources/AnimationCurveResource.h"
#include "Engine/AnimationCurve/Curve.h"
#include "Components/HealthComponent.h"
#include "Engine/GameObject/Components/RigidBodyComponent.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"
#include "Engine/GameObject/Components/VFXComponent.h"

void FB::HandHomingProjectile::LaunchTowards(const Vec3f& aFrom, Weak<GameObject> aTarget, const Vec3f& aOffset, const int aProjectileNumber)
{
	if (aTarget.expired())
	{
		LOG_ERROR(LogType::Game) << "Homing projectile target has expired";
		Destroy();
		return;
	}

	myTarget = aTarget;
	myState = State::Launching;
	myLaunchTimer = TickTimer::FromSeconds(myLaunchDuration);
	myDestroyMyselfTimer = TickTimer::FromSeconds(mySecondsUntilSelfDestroy);
	myProjectileNumber = aProjectileNumber;

	myLaunchDelayTimer = TickTimer::FromSeconds(myLaunchDelay * static_cast<float>(myProjectileNumber));

	const auto targetPos = aTarget.lock()->GetTransform().GetPosition();
	const auto targetDirection = (targetPos - aFrom).GetNormalized();

	const auto launchTargetPos = aFrom + targetDirection * myLaunchDistance;

	myLaunchBezier = {};
	myLaunchBezier.AddPoint(aFrom);
	// myLaunchBezier.AddPoint(aFrom + targetDirection * (launchTargetPos - aFrom).Length() * 0.5f + Vec3f(0.f, 1.f, 0.f) * myLaunchUpDistance);
	myLaunchBezier.AddPoint(aFrom + targetDirection * (launchTargetPos - aFrom).Length() * 0.2f + aOffset);
	myLaunchBezier.AddPoint(launchTargetPos/* + Vec3f(0.f, 1.f, 0.f) * 500.f*/ + aOffset * 0.5f);
}

void FB::HandHomingProjectile::SetSecondsUntilSelfDestroy(const float aSeconds)
{
	mySecondsUntilSelfDestroy = aSeconds;
}

void FB::HandHomingProjectile::SetProjectileSpeed(const float aSpeed)
{
	mySpeed = aSpeed;
}

void FB::HandHomingProjectile::SetDamage(const float aDamage)
{
	myDamage = aDamage;
}

void FB::HandHomingProjectile::OnLostTarget()
{

}

void FB::HandHomingProjectile::OnDamaged(const float aDamage)
{

}

bool FB::HandHomingProjectile::OnDeath(float aDamage)
{
	ExplodeDestroyMyself();

	// what does return do?
	return true;
}

void FB::HandHomingProjectile::OnCollidePlayer(GameObject* aPlayerGameObject)
{
	// damage the player with explosion?
	if (aPlayerGameObject->GetTag() == eTag::PLAYER)
	{
		if (auto health = aPlayerGameObject->GetComponent<HealthComponent>())
		{
			health->ApplyDamage(myDamage);
		}
	}

	ExplodeDestroyMyself();
}

void FB::HandHomingProjectile::ExplodeDestroyMyself()
{
	if (myExplosionVFX->IsValid())
	{
		auto g = myGameObject->GetScene()->AddGameObject<GameObject>();
		g->GetTransform().SetPosition(GetTransform().GetPosition());
		auto myVFX = g->AddComponent<Engine::VFXComponent>(myExplosionVFX->Get());
		myVFX->Play();
		myVFX->AutoDestroy();
	}
	else
	{
		LOG_ERROR(LogType::Game) << "Missing VFX in hand homing projectile";
	}

	myGameObject->Destroy();
}

void FB::HandHomingProjectile::Start()
{
	if (!myLaunchSpeedCurve || !myLaunchSpeedCurve->IsValid())
	{
		LOG_ERROR(LogType::Game) << "HandHomingProjectile with name: " << myGameObject->GetName() << " is missing LaunchSpeedCurve";
		Destroy();
		return;
	}

	//myGameObject->GetComponent();
	auto health = myGameObject->GetComponent<HealthComponent>();
	if (!health)
	{
		LOG_ERROR(LogType::Game) << "HandHomingProjectile with name:" << myGameObject->GetName() << " is missing HealthComponent";
		Destroy();
		return;
	}

	health->ObserveDamage([this](float aDamage) { OnDamaged(aDamage); });
	health->ObserveDeath([this](float aDamage) { OnDeath(aDamage); });

	auto rb = myGameObject->GetComponent<Engine::RigidBodyComponent>();
	rb->ObserveTriggerEnter([this](GameObject* g)
		{
			if (myHasCollided)
			{
				return;
			}

			myHasCollided = true;

			if (g->GetTag() == eTag::PLAYER)
			{
				OnCollidePlayer(g);
			}
			else
			{
				OnDeath(0.f);
			}
		});

	// Must do this bcuz TriggerEnter does not always work
	rb->ObserveTriggerStay([this](GameObject* g)
		{
			if (myHasCollided)
			{
				return;
			}

			myHasCollided = true;

			if (g->GetTag() == eTag::PLAYER)
			{
				OnCollidePlayer(g);
			}
			else
			{
				OnDeath(0.f);
			}
		});
}

void FB::HandHomingProjectile::Execute(Engine::eEngineOrder aOrder)
{
	switch (myState)
	{
	case State::Launching:
	{
		// filip, fortsätt med att försöka testa min homing projectile.
		// just nu laggar bara bossens högra hand oändligt

		const float speedCurveT = myLaunchSpeedCurve->Get().Evaluate(myLaunchTimer.Percent());

		myLaunchTimer.Tick(Time::DeltaTime * speedCurveT);

		if (myLaunchTimer.JustFinished() || speedCurveT <= 0.01f)
		{
			myState = State::MoveTowardsTarget;
		}

		const float t = myLaunchTimer.Percent();

		const Vec3f curvePos = myLaunchBezier.Evaluate(t);
		const Vec3f nextCurvePos = myLaunchBezier.Evaluate(t + 0.001f);
		const Vec3f direction = (nextCurvePos - curvePos).GetNormalized();
		GetTransform().SetPosition(curvePos);
		GetTransform().LookAt(GetTransform().GetPosition() + direction);
	} break;

	case State::MoveTowardsTarget:
	{
		myLaunchDelayTimer.Tick(Time::DeltaTime);

		if (myLaunchDelayTimer.IsFinished())
		{
			if (auto target = myTarget.lock())
			{
				GetTransform().LookAt(target->GetTransform().GetPosition());
				const auto direction = (target->GetTransform().GetPosition() - GetTransform().GetPosition()).GetNormalized();
				auto pos = GetTransform().GetPosition();
				pos += direction * mySpeed * Time::DeltaTime;
				GetTransform().SetPosition(pos);

				myDestroyMyselfTimer.Tick(Time::DeltaTime);

				if (myDestroyMyselfTimer.JustFinished())
				{
					// Self destroy
					myGameObject->Destroy();
				}
			}
			else
			{
				OnLostTarget();
			}
		}
	} break;

	default:
		break;
	}
}

void FB::HandHomingProjectile::Reflect(Engine::Reflector& aReflector)
{
	Component::Reflect(aReflector);

	aReflector.Reflect(myLaunchSpeedCurve, "Launch Speed Curve");
	aReflector.Reflect(myExplosionVFX, "Explosion VFX");

	aReflector.Reflect(myLaunchDuration, "Launch Duration");

	aReflector.Reflect(mySecondsUntilSelfDestroy, "Seconds Until Self Destroy");

	aReflector.Reflect(myLaunchDistance, "Launch Distance");
	aReflector.Reflect(myLaunchUpDistance, "Launch Up Distance");

	aReflector.Reflect(myLaunchDelay, "Launch Delay");

	aReflector.Reflect(mySpeed, "Speed");
}