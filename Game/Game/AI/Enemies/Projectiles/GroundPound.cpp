#include "pch.h"
#include "GroundPound.h"

#include "Engine\Shortcuts.h"
#include "Engine/DebugManager/DebugDrawer.h"
#include "AI\Enemies\Base\EnemyBase.h"
#include "AI\PollingStation\PollingStationComponent.h"
#include "Components\HealthComponent.h"
#include "AI\Enemies\GroundTankLad\GroundTankLad.h"
#include "AI/VFX/EnemyVFXRef.h"
#include "Engine/GameObject/Components/VFXComponent.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"

GroundPound::GroundPound()
{
}

GroundPound::~GroundPound()
{
}

void GroundPound::Init(EnemyBase* anEnemy, float aDamage, float aChargeTime, float aCooldown)
{
	myEnemyRef = anEnemy;
	myChargeTime = aChargeTime;
	myDamage = aDamage;
	myCooldownTime = aCooldown;
}

void GroundPound::InitiateAttack()
{
	if (myFireState == eFireState::Ready)
	{
		myAttackPoint = myEnemyRef->GetTransform().GetPosition() + myEnemyRef->GetGameObject()->GetTransform().Forward() * -1.f * 100.f;

		myBeginningAttackHeight = myAttackPoint.y + 100.f;
		myDesiredAttackHeight = myAttackPoint.y;

		GDebugDrawer->DrawSphere3D(DebugDrawFlags::Always, myAttackPoint , 100.f, 0.1f, {1,0,0,1});

		myPoundTimer = myChargeTime;
		LOG_INFO(LogType::Animation) << "Charging";
		myFireState = eFireState::Charging;
	}

}

void GroundPound::Attack(BT_Blackboard* aBlackBoard)
{
	std::vector<OverlapHit> hits;
	Vec3f poundPosition = aBlackBoard->getVec3f("PoundPosition");
	poundPosition.y -= aBlackBoard->getFloat("PoundOffset");

	mySightLaserVFX = aBlackBoard->GetOwner()->GetScene()->AddGameObject<GameObject>();

	const auto& vfxRef = mySightLaserVFX->GetSingletonComponent<EnemyVFXRef>()->GetGroundTankSlam();
	myVFX = mySightLaserVFX->AddComponent<Engine::VFXComponent>(vfxRef->Get());
	myVFX->Play();
	myVFX->AutoDestroy();

	mySightLaserVFX->GetTransform().SetPosition(poundPosition);

	GDebugDrawer->DrawSphere3D(DebugDrawFlags::Always, poundPosition, aBlackBoard->getFloat("PoundColliderSize"), 1.f, { 0,1,0,1 });

	myEnemyRef->GetGameObject()->GetScene()->SphereCastAll(poundPosition, aBlackBoard->getFloat("PoundColliderSize"), eLayer::ALL, hits);

	for (int hitIndex = 0; hitIndex < hits.size(); hitIndex++)
	{
		if (hits[hitIndex].GameObject->GetName() == "Player")
		{
			hits[hitIndex].GameObject->GetComponent<HealthComponent>()->ApplyDamage(myDamage);
		}
	}
}

void GroundPound::EndAttack()
{
	myFireState = eFireState::Cooldown;
	myPoundTimer = myCooldownTime;
}

void GroundPound::Update(float aDeltaTime)
{
	std::vector<OverlapHit> hits;
	float value;
	float amountElapsed;
	CU::Vector3f enemyDir = myEnemyRef->GetTransform().Forward() * -1.f;

	//myEnemyRef->GetGameObject()->GetScene()->RayCastAll(myEnemyRef->GetTransform().GetPosition(), enemyDir, 100000000, eLayer::ALL, hits);
	//GDebugDrawer->DrawLine3D(DebugDrawFlags::Always, myEnemyRef->GetTransform().GetPosition(), enemyDir * 100000000.f, 0.1f, { 1,0,0,1 });

	switch (myFireState)
	{
	case GroundPound::eFireState::Charging:

		/*amountElapsed = myPoundTimer / myChargeTime;

		value = CU::Lerp(myDesiredAttackHeight, myBeginningAttackHeight, amountElapsed);

		myAttackPoint.y = value;

		GDebugDrawer->DrawSphere3D(DebugDrawFlags::Always, myAttackPoint, 100.f, 0.1f, { 1,0,0,1 });

		if (myPoundTimer <= 0)
		{
			myFireState = eFireState::Pounding;
		}*/
		break;
	case GroundPound::eFireState::Pounding:

		//myEnGroundPoundGameObject()->Pounding()->RayCast(myEnemyRef->GetTransform().GetPosition(), toPlayer, 100000000, eLayer::ENEMY, hit);
		GDebugDrawer->DrawSphere3D(DebugDrawFlags::Always, myAttackPoint, 100.f, 0.1f, { 0,1,0,1 });

		myEnemyRef->GetGameObject()->GetScene()->SphereCastAll(myAttackPoint, 100.f, eLayer::ALL, hits);

		for (int hitIndex = 0; hitIndex < hits.size(); hitIndex++)
		{
			if (hits[hitIndex].GameObject->GetName() == "Player")
			{
				hits[hitIndex].GameObject->GetComponent<HealthComponent>()->ApplyDamage(myDamage);
			}
		}

		if (myPoundTimer <= 0)
		{
			myPoundTimer = myCooldownTime;
			LOG_INFO(LogType::Animation) << "Cooldown";
			myFireState = eFireState::Cooldown;
		}
		break;
	case GroundPound::eFireState::Cooldown:

		if (myPoundTimer <= 0)
		{
			myEnemyRef->GetGameObject()->GetComponent<GroundTankLad>()->SetMyState(GroundTankLad::EnemyState::Neutral);
			LOG_INFO(LogType::Animation) << "Ready";
			myFireState = eFireState::Ready;
		}

		break;
	default:
		break;
	}


	myPoundTimer -= aDeltaTime;
}

const GroundPound::eFireState GroundPound::GetFireState() const
{
	return myFireState;
}
