#include "pch.h"
#include "EmittedShockwave.h"
#include "Engine/GameObject/GameObject.h"
#include "AI/PollingStation/PollingStationComponent.h"
#include "Components/HealthComponent.h"

void FB::EmittedShockwave::Start()
{
	if (!myModel)
	{
		LOG_ERROR(LogType::Game) << "Emitted shockwave missing model prefab";
		myGameObject->Destroy();
		return;
	}
}

void FB::EmittedShockwave::Execute(Engine::eEngineOrder aOrder)
{
	UpdateSize();
	CheckHitPlayer();
}

void FB::EmittedShockwave::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(mySpeed, "Speed");
	aReflector.Reflect(myKnockbackStrength, "Player Knockback Strength");
	aReflector.Reflect(mySphereThiccness, "Sphere Thiccness");
	aReflector.Reflect(myModel, "Shockwave Model");
	aReflector.Reflect(myShockwaveHeight, "Shockwave Height");
}

void FB::EmittedShockwave::SetDamage(const float aDamage)
{
	myDamage = aDamage;
}

void FB::EmittedShockwave::SetSpeed(const float aSpeed)
{
	mySpeed = aSpeed;
}

void FB::EmittedShockwave::SetKnockbarStrength(const float aKnockbackStrength)
{
	myKnockbackStrength = aKnockbackStrength;
}

void FB::EmittedShockwave::SphereThiccness(const float aSphereThiccness)
{
	mySphereThiccness = aSphereThiccness;
}

void FB::EmittedShockwave::SetShockwaveHeight(const float aHeight)
{
	myShockwaveHeight = aHeight;
}

void FB::EmittedShockwave::CheckHitPlayer()
{
	auto polling = myGameObject->GetSingletonComponent<PollingStationComponent>();

	std::vector<OverlapHit> hits;

	GDebugDrawer->DrawSphere3D(
		DebugDrawFlags::Gameplay,
		GetTransform().GetPosition(),
		myRadius,
		0.1f);

	const bool hitSomething = myGameObject->GetScene()->SphereCastAll(
		GetTransform().GetPosition(),
		myRadius, eLayer::PLAYER_DAMAGEABLE,
		hits);

	if (hitSomething)
	{
		for (const auto& hit : hits)
		{
			if (!hit.GameObject)
			{
				continue;
			}


			const bool isPlayer = hit.GameObject == polling->GetPlayer();

			// TODO: Check if player is above a specific Y level to know if it jumped over the
			// sphere

			if (isPlayer)
			{
				GameObject* player = polling->GetPlayer();

				const auto emitPos = GetTransform().GetPosition();

				const auto playerToCenterDist = (player->GetTransform().GetPosition() - emitPos).Length();

				const float innerCircleRadius = std::max(myRadius - mySphereThiccness, 0.f);
				const float outerCircleRadius = myRadius;

				const bool isWithinDonutArm =
					playerToCenterDist < outerCircleRadius&&
					playerToCenterDist > innerCircleRadius;

				LOG_INFO(LogType::Filip) << innerCircleRadius << ", " << outerCircleRadius << ", " << playerToCenterDist;

				const bool isPlayerAboveDonutArm =
					player->GetTransform().GetPosition().y > (GetTransform().GetPosition().y + myShockwaveHeight);

				if (isWithinDonutArm && !isPlayerAboveDonutArm)
				{
					if (!myHasDamagedPlayer)
					{
						myHasDamagedPlayer = true;

						// DEAL DAMAGE
						// 
						LOG_INFO(LogType::Filip) << "scream hit player";
						auto playerHealth = player->GetComponent<HealthComponent>();

						playerHealth->ApplyDamage(myDamage);

						/*
						Vec3f dirAndStrength = (player->GetTransform().GetPosition() - GetTransform().GetPosition()).GetNormalized();
						dirAndStrength *= myKnockbackStrength;

						playerHealth->ApplyKnockback(dirAndStrength);
						*/
					}
				}
			}
		}
	}
}

void FB::EmittedShockwave::UpdateSize()
{
	myRadius += mySpeed * Time::DeltaTime;

	if (GameObject* g = myModel.Get())
	{
		auto scale = g->GetTransform().GetScale();
		scale.x = myRadius / 100.f;
		scale.y = 4.f;
		scale.z = myRadius / 100.f;
		g->GetTransform().SetScale(scale);
	}
}
