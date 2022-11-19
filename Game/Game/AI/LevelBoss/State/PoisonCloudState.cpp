#include "pch.h"
#include "PoisonCloudState.h"

#include "Components/HealthComponent.h"

#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/VFXComponent.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"

LevelBossStates::PoisonCloudState::PoisonCloudState(LevelBoss& aBoss, PoisonCloudVars& someVariables) 
	: EngageState(aBoss), myVars(someVariables)
{
	OnExitState();

	GetController().AddEventCallback("PoisonAttackFrame", [this]() { CreateCloud(); });

	myAnimState = AnimState::None;
}

void LevelBossStates::PoisonCloudState::OnEnterState()
{ 
	auto pollingStation = GetPollingStation();
	if (!pollingStation)
		return;

	myShootPosition = pollingStation->GetPlayerPos();
	GetController().Trigger("TriggerPoison");
	myProgress = myDuration;

	GetAudio().PlayEvent("PoisonCloud");
}

void LevelBossStates::PoisonCloudState::OnExitState()
{
	myVars.myCooldownDuration = Random::RandomFloat(myVars.myCooldownDurationMin, myVars.myCooldownDurationMax);
	myVars.myCooldownProgress = myVars.myCooldownDuration;
}

void LevelBossStates::PoisonCloudState::OnEnableAttack()
{
	if (!myBoss.IsAbilityUnlocked(LevelBossState::PoisonCloud) && !myBoss.IsAbilityUnlocked(LevelBossState::Mortar))
	{
		myAnimState = AnimState::Opening;
		myVars.myOverrideProgress = myVars.myOverrideDuration;
	}
}

void LevelBossStates::PoisonCloudState::OnDisableAttack()
{
	if (!myBoss.IsAbilityUnlocked(LevelBossState::PoisonCloud) && !myBoss.IsAbilityUnlocked(LevelBossState::Mortar))
	{
		myAnimState = AnimState::Closing;
		myVars.myOverrideProgress = myVars.myOverrideDuration;
	}
}

void LevelBossStates::PoisonCloudState::Update()
{
	myProgress -= Time::DeltaTime;
	if (myProgress <= 0.0f)
	{
		myBoss.ChangeState(LevelBossState::Idle);
	}
}

void LevelBossStates::PoisonCloudState::ForceUpdate()
{
	myVars.myCooldownProgress -= Time::DeltaTime;
	if (myVars.myCooldownProgress <= 0.0f)
	{
		myVars.myCooldownProgress = 0.0f;
	}

	myVars.myOverrideProgress -= Time::DeltaTime;
	if (myAnimState == AnimState::Opening)
	{
		if (myVars.myOverrideProgress <= 0.0f)
		{
			myVars.myOverrideProgress = 0.0f;
			myAnimState = AnimState::None;
		}
		float ratio = myVars.myOverrideProgress / myVars.myOverrideDuration;
		GetController().SetLayerWeight("Mortar + Poison Is Disabled - (Override)", ratio);
	}
	else if (myAnimState == AnimState::Closing)
	{
		if (myVars.myOverrideProgress <= 0.0f)
		{
			myVars.myOverrideProgress = 0.0f;
			myAnimState = AnimState::None;
		}
		float ratio = 1.0f - (myVars.myOverrideProgress / myVars.myOverrideDuration);
		GetController().SetLayerWeight("Mortar + Poison Is Disabled - (Override)", ratio);
	}

	if (myPoisonClouds.empty())
		return;

	GameObject* gameObject = GetPollingStation()->GetPlayer();
	auto scene = myBoss.GetGameObject()->GetScene();
	std::vector<OverlapHit> hits;

	myVars.myAttackProgress -= Time::DeltaTime;

	const int size = static_cast<int>(myPoisonClouds.size());
	for (int i = size - 1; i >= 0; i--)
	{
		Cloud& cloud = myPoisonClouds[i];
		
		if (myVars.myAttackProgress <= cloud.mySpawnTime)
		{
			if (!cloud.myVFX->IsPlaying())
			{
				cloud.myVFX->Play();
				cloud.myVFX->AutoDestroy();
			}

			cloud.myProgress -= Time::DeltaTime;
			cloud.myTickProgress -= Time::DeltaTime;

			//const float ratio = cloud.myProgress / cloud.myDuration;
			//const Vec4f color = Vec4f(0.f, 73.0f / 255.f, 164.0f / 255.f, 1.0f);
			//GDebugDrawer->DrawSphere3D(DebugDrawFlags::Gameplay, cloud.myPosition, myVars.mySize, 0.0f, color);
			//GDebugDrawer->DrawSphere3D(DebugDrawFlags::Gameplay, cloud.myPosition, myVars.mySize * ratio, 0.0f, color);

			if (cloud.myTickProgress <= 0.0f)
			{
				if (scene->SphereCastAll(cloud.myPosition, myVars.mySize, eLayer::PLAYER_DAMAGEABLE, hits))
				{
					for (auto& hit : hits)
					{
						if (hit.GameObject == gameObject)
						{
							if (auto health = gameObject->GetComponent<HealthComponent>())
							{
								const float damage = Random::RandomFloat(myVars.myMinDamage, myVars.myMaxDamage);
								health->ApplyDamage(damage * myBoss.GetDmgMultiplier());
							}
						}
					}
				}
				cloud.myTickProgress = cloud.myTickDuration;
			}

			if (cloud.myProgress <= 0.0f)
			{
				cloud.myVFX->Stop();
				myPoisonClouds.erase(myPoisonClouds.begin() + i);
			}
		}
	}
}

void LevelBossStates::PoisonCloudState::OnUpdate(const Vec3f& aPlayerPosition)
{
	TurnTowards(aPlayerPosition);
}

void LevelBossStates::PoisonCloudState::CreateCloud()
{
	Vec3f startPosition = GetTransform().GetPosition();
	startPosition.y = 0;
	Vec3f endPosition = myShootPosition;
	endPosition.y = 0;
	Vec3f targetDirection = (endPosition - startPosition).GetNormalized();
	Vec3f upOffset = Vec3f(0, 1, 0) * 2100.0f;
	upOffset.y += GetTransform().GetPosition().y;

	const float distance = myVars.myDistance;
	const float fieldSize = myVars.mySize;
	const float count = distance / (fieldSize * 2.0f);

	const int size = (int)count;
	int width = 0;

	auto scene = myBoss.GetGameObject()->GetScene();
	RayCastHit hit;

	for (int i = 0; i < size; i++)
	{
		const float value = static_cast<float>(i) / count;
		const float spawnTime = (1.0f - value) * myVars.myAttackDuration;
		const Vec3f position = CU::Lerp(startPosition, startPosition + targetDirection * distance, value);

		if (scene->RayCast(position + upOffset, Vec3f(0, -1, 0), FLT_MAX, eLayer::DEFAULT | eLayer::GROUND, hit))
		{
			Cloud cloud;
			cloud.myPosition = hit.Position;
			cloud.mySpawnTime = spawnTime;
			cloud.myProgress = myVars.mySpeed;
			cloud.myDuration = myVars.mySpeed;
			cloud.myTickProgress = myVars.myTickRate;
			cloud.myTickDuration = myVars.myTickRate;

			if (myVars.myPoisonCloudVFXRef && myVars.myPoisonCloudVFXRef->IsValid())
			{
				if (auto go = myBoss.GetGameObject()->GetScene()->AddGameObject<GameObject>())
				{
					go->GetTransform().SetPosition(hit.Position);
					cloud.myVFX = go->AddComponent<Engine::VFXComponent>(myVars.myPoisonCloudVFXRef->Get());
				}
			}

			myPoisonClouds.push_back(cloud);
		}

		if (myVars.myWidth > 0 && i > int(myVars.myWidth / 2))
		{
			if (i % myVars.myWidth == 0)
			{
				width++;
			}
		}

		for (int j = width; j > 0; j--)
		{
			const Vec3f right = position + targetDirection.Cross(Vec3f(0, 1, 0)) * ((float)j * fieldSize * 2.0f);
			const Vec3f left = position + targetDirection.Cross(Vec3f(0, -1, 0)) * ((float)j * fieldSize * 2.0f);

			if (scene->RayCast(right + upOffset, Vec3f(0, -1, 0), FLT_MAX, eLayer::DEFAULT | eLayer::GROUND, hit))
			{
				Cloud cloud;
				cloud.myPosition = hit.Position;
				cloud.mySpawnTime = spawnTime;
				cloud.myProgress = myVars.mySpeed;
				cloud.myDuration = myVars.mySpeed;
				cloud.myTickProgress = myVars.myTickRate;
				cloud.myTickDuration = myVars.myTickRate;

				if (myVars.myPoisonCloudVFXRef && myVars.myPoisonCloudVFXRef->IsValid())
				{
					if (auto go = myBoss.GetGameObject()->GetScene()->AddGameObject<GameObject>())
					{
						go->GetTransform().SetPosition(hit.Position);
						cloud.myVFX = go->AddComponent<Engine::VFXComponent>(myVars.myPoisonCloudVFXRef->Get());
					}
				}

				myPoisonClouds.push_back(cloud);
			}

			if (scene->RayCast(left + upOffset, Vec3f(0, -1, 0), FLT_MAX, eLayer::DEFAULT | eLayer::GROUND, hit))
			{
				Cloud cloud;
				cloud.myPosition = hit.Position;
				cloud.mySpawnTime = spawnTime;
				cloud.myProgress = myVars.mySpeed;
				cloud.myDuration = myVars.mySpeed;
				cloud.myTickProgress = myVars.myTickRate;
				cloud.myTickDuration = myVars.myTickRate;

				if (myVars.myPoisonCloudVFXRef && myVars.myPoisonCloudVFXRef->IsValid())
				{
					if (auto go = myBoss.GetGameObject()->GetScene()->AddGameObject<GameObject>())
					{
						go->GetTransform().SetPosition(hit.Position);
						cloud.myVFX = go->AddComponent<Engine::VFXComponent>(myVars.myPoisonCloudVFXRef->Get());
					}
				}

				myPoisonClouds.push_back(cloud);
			}
		}
	}

	myVars.myAttackProgress = myVars.myAttackDuration;

	myBoss.ChangeState(LevelBossState::Idle);
}
