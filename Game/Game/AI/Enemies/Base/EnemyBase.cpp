#include "pch.h"
#include "EnemyBase.h"

#include "AI/AIDirector/AIDirector.h"
#include "Components\HealthComponent.h"

#include "AI/Enemies/Projectiles/ExplodeProjectile.h"
#include "AI/Enemies/Projectiles/GroundPopcornProjectile.h"
#include "AI/Enemies/Projectiles/MortarProjectile.h"
#include "AI/Enemies/Projectiles/PopcornProjectile.h"
#include "AI/Enemies/Projectiles/Projectile.h"
#include "AI/Health/EnemyHealth.h"
#include "AI/PollingStation/PollingStationComponent.h"
#include "Components/MoneyDropComponent.h"

#include "Engine/Shortcuts.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/AudioComponent.h"
#include "Engine/GameObject/Components/CharacterControllerComponent.h"
#include "Engine/GameObject/Components/ModelComponent.h"
#include "Engine/Reflection/Reflector.h"
#include "Engine/TimeSystem/CallbackTimer.h"

class MoneyDropComponent;

EnemyBase::EnemyBase(GameObject* aGameObject) :
	Component(aGameObject)
{
}

EnemyBase::~EnemyBase()
{
}

void EnemyBase::Start()
{
	Component::Start();
}

void EnemyBase::Execute(Engine::eEngineOrder aOrder)
{
	Component::Execute(aOrder);// Mandatory to be able to work correctly, do not remove!

	Update();
}

void EnemyBase::Update()
{
	//UpdateDebuffTimers(); // Optimize stuff
	UpdateSmoothRotation();

	if (myKnockedBack)
	{
		Knockback(myKnockBackDirection);
	}

	CheckIfToFarFromPlayer();
}

void EnemyBase::Reflect(Engine::Reflector& aReflector)
{
	Component::Reflect(aReflector);
}

void EnemyBase::Render()
{
	Component::Render();

	const auto& ownerTransform = GetGameObject()->GetTransform();
	const auto& ownerPos = GetGameObject()->GetTransform().GetPosition();
	auto forward = ownerTransform.Forward();
	forward.Normalize();

	GDebugDrawer->DrawLine3D(
		DebugDrawFlags::AI,
		ownerPos,
		ownerPos + forward * 300.f,
		0.f,
		{ 0.f, 0.f, 1.f, 1.f });


}

void EnemyBase::OnDamage()
{

}

void EnemyBase::SetDeSpawnFunction(std::function<void()> aDeSpawnFunction)
{
	myDeSpawnFunction = aDeSpawnFunction;
}

void EnemyBase::SetTargetRotationRadians(float aRadians, float aRotationSmoothness)
{
	myRotationSmoothness = aRotationSmoothness;
	myTargetRotationRadians = aRadians;
}

void EnemyBase::UpdateSmoothRotation()
{
	myCurrentRotationRadians = Math::LerpAngleRadians(
		myCurrentRotationRadians,
		myTargetRotationRadians,
		Time::DeltaTime * myRotationSmoothness);

	GetGameObject()->GetTransform().SetRotation(CU::Vector3f(0.0f, myCurrentRotationRadians, 0.0f));
}

void EnemyBase::RotateTowardsPlayer(float aRotationSmoothness)
{
	auto targetPos = myGameObject->GetSingletonComponent<PollingStationComponent>()->GetPlayerPos();
	auto moveDir = targetPos - GetGameObject()->GetTransform().GetPosition();
	moveDir.Normalize();

	float angle = atan2(moveDir.z, moveDir.x);
	SetTargetRotationRadians(angle + 90.f * Math::Deg2Rad, aRotationSmoothness);
}

bool EnemyBase::CheckIfCanShoot(float aShootRange)
{
	bool returnValue = false;


	auto bulletStartPos = myGameObject->GetTransform().GetPosition();

	auto bulletEndPos = myGameObject->GetSingletonComponent<PollingStationComponent>()->GetPlayerPos();
	const auto rayCastDir = (bulletEndPos - bulletStartPos).GetNormalized();

	RayCastHit hit;
	if (myGameObject->GetScene()->RayCast(
		bulletStartPos,
		rayCastDir,
		1000000.f,
		eLayer::PLAYER | eLayer::DEFAULT,
		hit))

	{
		if (hit.GameObject != nullptr)
		{
			if (hit.GameObject->GetLayer() & eLayer::DEFAULT)
			{
				int aw = 5;
			}
			else
			{
				returnValue = true;
			}
		}
	}

	return returnValue;
}

void EnemyBase::Shoot(BT_Blackboard::Ptr aBlackBoard) const
{
	Vec3f spawnPosition = myGameObject->GetTransform().GetPosition();
	Vec3f playerPosition = myGameObject->GetSingletonComponent<PollingStationComponent>()->GetPlayerPos();
	auto* audioComp = myGameObject->GetComponent<Engine::AudioComponent>();	
	auto& scalableValues = myGameObject->GetComponent<EnemyScalableStats>()->GetScalableValues();

	switch (myEnemyShootType)
	{
	case eEnemyShootType::RapidPopcorn:
	{
		auto& data = aBlackBoard->GetRapidShootData();
		data.mySpawnPosition = aBlackBoard->getVec3f("ShootPosition");
		/*	spawnPosition.y += 300.f;
			data.mySpawnPosition = spawnPosition;*/
		playerPosition.y -= 30.f;
		const Vec3f moveDir = (playerPosition - spawnPosition).GetNormalized();

		GameObject* gameObject = myGameObject->GetScene()->AddGameObject<GameObject>();
		gameObject->GetTransform().SetPosition(data.mySpawnPosition);
		Projectile* projectile = gameObject->AddComponent<PopcornProjectile>();
		projectile->Init(data.mySpeed, moveDir, scalableValues.myDamage, myGameObject->GetUUID());

		aBlackBoard->GetAudioComponent()->PlayEvent("Projectile");
		break;
	}
	case eEnemyShootType::GroundPopcorn:
	{
		auto& data = aBlackBoard->GetGroundPopcornShootData();
		spawnPosition.y += 300.f;
		data.mySpawnPosition = aBlackBoard->getVec3f("ShootPosition");
		playerPosition.y += 150.f;
		const Vec3f moveDir = (playerPosition - spawnPosition).GetNormalized();

		GameObject* gameObject = myGameObject->GetScene()->AddGameObject<GameObject>();
		gameObject->GetTransform().SetPosition(data.mySpawnPosition);
		GroundPopcornProjectile* projectile = gameObject->AddComponent<GroundPopcornProjectile>();
		projectile->Init(data.mySpeed, moveDir, scalableValues.myDamage, myGameObject->GetUUID());
		projectile->SetRadius(data.myProjectileRadius);

		aBlackBoard->GetAudioComponent()->PlayEvent("Projectile");
		break;
	}
	case eEnemyShootType::BurstTank:
	{
		auto& data = aBlackBoard->GetBurstShootData();
		data.myDamage = scalableValues.myDamage;
		data.mySpawnPosition = aBlackBoard->getVec3f("ShootPosition");
		//spawnPosition.y += 300.f;
		//data.mySpawnPosition = spawnPosition;
		const Vec3f moveDir = (playerPosition - spawnPosition).GetNormalized();

		BurstShoot(data, aBlackBoard);
		
		break;
	}
	case eEnemyShootType::Mortar:
	{
		auto& data = aBlackBoard->GetMortarShootData();
		data.mySpawnPosition = aBlackBoard->getVec3f("ShootPosition");
		data.mySpawnPosition.y += 100.f;
		data.myDamage = scalableValues.myDamage;
		//spawnPosition.y += 300.f;
		//data.mySpawnPosition = spawnPosition;
		const Vec3f moveDir = (playerPosition - spawnPosition).GetNormalized();

		Mortar(data);

		aBlackBoard->GetAudioComponent()->PlayEvent("Mortar");
		break;
	}
	default:
		break;
	}
}

void EnemyBase::BurstShoot(Enemy::eBurstShootData aBurstShootData, BT_Blackboard::Ptr aBlackBoard) const
{
	auto scene = myGameObject->GetScene();

	int amountOfShoots = 3;
	float bulletSpawnTime = 0.f;
	float timeBetweenBullets = 0.5f;
	Engine::UUID UUID = myGameObject->GetUUID();

	float myDamage = aBurstShootData.myDamage;
	float mySpeed = aBurstShootData.mySpeed;
	float myRadius = aBurstShootData.myProjectileRadius;
	float myExplosionRadius = aBurstShootData.myProjectileExplosionRadius;
	Vec3f mySpawnPos = aBurstShootData.mySpawnPosition;

	auto* poll = myGameObject->GetSingletonComponent<PollingStationComponent>();
	auto* audio = aBlackBoard->GetAudioComponent();
	Weak<GameObject> weakGO = GetGameObject()->GetWeak();

	for (int i = 0; i < amountOfShoots; i++)
	{
		auto cb = DBG_NEW Engine::CallbackTimer(
			[scene, myDamage, mySpawnPos, mySpeed, poll, myRadius, myExplosionRadius, UUID, weakGO, audio, aBlackBoard]()
			{
				if (weakGO.expired()) 
				{
					return;
				}
				Vec3f moveDirection = (poll->GetPlayerPos() - mySpawnPos).GetNormalized();

				GameObject* gameObject = scene->AddGameObject<GameObject>();
				gameObject->GetTransform().SetPosition(mySpawnPos);
				ExplodeProjectile* projectile = gameObject->AddComponent<ExplodeProjectile>(myRadius, myExplosionRadius);
				projectile->Init(mySpeed, moveDirection, myDamage, UUID);
					audio->PlayEvent("Orb");
			},
			bulletSpawnTime);
		bulletSpawnTime += timeBetweenBullets;
	}
}

void EnemyBase::Mortar(Enemy::eMortarShootData aMortarShootData) const
{
	auto scene = myGameObject->GetScene();
	auto* poll = myGameObject->GetSingletonComponent<PollingStationComponent>();
	Engine::UUID UUID = myGameObject->GetUUID();

	Vec3f moveDirection = (poll->GetPlayerPos() - aMortarShootData.mySpawnPosition).GetNormalized();

	GameObject* gameObject = scene->AddGameObject<GameObject>();
	gameObject->GetTransform().SetPosition(aMortarShootData.mySpawnPosition);

	MortarProjectile* projectile = gameObject->AddComponent<MortarProjectile>(
		aMortarShootData.myProjectileRadius,
		aMortarShootData.myExplosionRadius);
	gameObject->SetLayers(eLayer::ENEMY_DAMAGEABLE, eLayer::NONE, eLayer::NONE);

	projectile->Init(aMortarShootData.mySpeed, moveDirection, aMortarShootData.myDamage, UUID);
}

void EnemyBase::SetEnemyShootType(eEnemyShootType aEnemyShootType)
{
	myEnemyShootType = aEnemyShootType;
}

void EnemyBase::CheckIfToFarFromPlayer()
{
	if (myGameObject->GetSingletonComponent<AIDirector>()->ShouldDespawnAll())
	{
		if (const auto moneyDrop = myGameObject->GetComponent<MoneyDropComponent>())
			moneyDrop->ZeroMoneyDrop();

		if (myDeSpawnFunction)
			myDeSpawnFunction();

		myGameObject->GetSingletonComponent<PollingStationComponent>()->EnemyCounterDecrease();

		return;
	}

	const Vec3f ownerPos = myGameObject->GetTransform().GetPosition();
	const Vec3f playerPos = myGameObject->GetSingletonComponent<PollingStationComponent>()->GetPlayerPos();

	const float distance = (ownerPos - playerPos).Length();

	const auto boundSphere = myGameObject->GetComponent<Engine::ModelComponent>()->CalculateWorldBoundingSphere();
	
	if (const bool isInside = myGameObject->GetScene()->GetMainCamera().GetViewFrustum().IsSphereInside(boundSphere))
		return;

	if (distance < myGameObject->GetSingletonComponent<AIDirector>()->GetEnemyDespawnDistacne())
		return;


	//myGameObject->GetSingletonComponent<AIDirector>()->RespawnEnemy(*myGameObject);

	//myGameObject->GetSingletonComponent<PollingStationComponent>()->EnemyCounterDecrease();
	//myGameObject->Destroy();


	if (const auto moneyDrop = myGameObject->GetComponent<MoneyDropComponent>())
		moneyDrop->ZeroMoneyDrop();

	//myGameObject->GetComponent<EnemyHealth>()->Kill();
	if (myDeSpawnFunction)
		myDeSpawnFunction();

	myGameObject->GetSingletonComponent<PollingStationComponent>()->EnemyCounterDecrease();
}

void EnemyBase::Knockback(Vec3f aDirection)
{
	auto controller = GetGameObject()->GetComponent<Engine::CharacterControllerComponent>();
	if (!controller)
		return;

	if (controller->IsGrounded() && !myIsKnocked)
	{
		controller->Jump(aDirection.y);
		myIsKnocked = true;
	}
	else if (!controller->IsGrounded() && myIsKnocked)
	{
		controller->Move(aDirection, Time::DeltaTime);
	}
	else
	{
		myKnockedBack = false;
		//FinnishJump();
	}
}

void EnemyBase::SetUpStun(std::function<void(float)> aStunFunction)
{
	myStunFunction = aStunFunction;
	auto health = GetGameObject()->GetComponent<EnemyHealth>()->GetHealthComponent();

	health->ObserveStun([&](float aTime)
		{
			myStunFunction(aTime);
		});
}

bool EnemyBase::SetStunValues(float aStunTime)
{
	if (myStunTimer < aStunTime)
	{
		myStunTimer = aStunTime;
		myStunned = true;
		return true;
	}
	return false;
}

void EnemyBase::SetUpSlow(std::function<void(float, float)> aSlowFunction)
{
	mySlowFunction = aSlowFunction;
	auto health = GetGameObject()->GetComponent<EnemyHealth>()->GetHealthComponent();

	health->ObserveSlow([&](float aSlowPercentage, float aSlowTime)
		{
			mySlowFunction(aSlowPercentage, aSlowTime);
		});
}

bool EnemyBase::SetSlowValues(float aSlowPercentage, float aSlowTime)
{
	return false;
}

void EnemyBase::SetUpKnockBack(std::function<void(Vec3f)> aKnockBackFunction)
{
	myKnockBackFunction = aKnockBackFunction;
	auto health = GetGameObject()->GetComponent<EnemyHealth>()->GetHealthComponent();

	health->ObserveKnockback([&](Vec3f aDirection)
		{
			myKnockBackFunction(aDirection);
		});
}

bool EnemyBase::SetKnockBackValues(Vec3f aDirection)
{
	myKnockBackDirection = aDirection;
	myKnockedBack = true;

	return true;
}

void EnemyBase::UpdateDebuffTimers()
{
	float deltaTime = Time::DeltaTime;

	if (myStunned)
	{
		myStunTimer -= deltaTime;
		if (myStunTimer <= 0)
		{
			myStunned = false;
			if (GetGameObject()->GetComponent<Engine::AnimatorComponent>())
			{
				GetGameObject()->GetComponent<Engine::AnimatorComponent>()->SetActive(true);
			}
		}
	}

	if (mySlowed)
	{
		if (GetGameObject()->GetComponent<EnemyHealth>()->GetHealthComponent()->UpdateSlowTimers())
		{
			mySlowed = false;
		}
	}
}

EnemyDeathType EnemyBase::GetEnemyDeathType()
{
	return myEnemyDeathType;
}

void EnemyBase::SetEnemyDeathType(EnemyDeathType aEnemyDeathType)
{
	myEnemyDeathType = aEnemyDeathType;
}

void EnemyBase::SpawnDeathSound()
{
}
