#include "pch.h"
#include "LaserBeam.h"

#include "Engine\Shortcuts.h"
#include "Engine/DebugManager/DebugDrawer.h"
#include "AI\Enemies\Base\EnemyBase.h"
#include "AI\PollingStation\PollingStationComponent.h"
#include "Engine/GameObject/Components/AudioComponent.h"
#include "AI/Enemies/GroundTankLad/GroundTankLad.h"
#include "Components\HealthComponent.h"

LaserBeam::LaserBeam()
{
}

LaserBeam::~LaserBeam()
{
}

void LaserBeam::Init(EnemyBase* anEnemyRef, BT_Blackboard* BT_Blackboard, float aDamage, float aChargeTime, float aFireTime, float aCooldown)
{
	myEnemyRef = anEnemyRef;
	myBlackboard = BT_Blackboard;
	myChargeTime = aChargeTime;
	myDamage = aDamage;
	myFireTime = aFireTime;
	myCooldownTime = aCooldown;
}

void LaserBeam::Ignite()
{
	if (myFireState == eFireState::Ready)
	{
		myLaserTimer = myChargeTime;
		LOG_INFO(LogType::Animation) << "Charging";
		if (myFireState != eFireState::Charging)
		{
			myBlackboard->GetAudioComponent()->PlayEvent("LaserCharging");
		}
		myFireState = eFireState::Charging;
	}

}

void LaserBeam::Update(float aDeltaTime)
{
	RayCastHit hit;
	std::vector<RayCastHit> hits;
	CU::Vector3f playerPos;
	//if (/*myEnemyRef->GetGameObject()->GetComponent<GroundTankLad>()*/ 1 == 2)
	//{
	//	int temp = myEnemyRef->GetGameObject()->GetComponent<GroundTankLad>()->GetBlackboard().getInt("DelayValue");
	//	playerPos = myEnemyRef->GetGameObject()->GetSingletonComponent<PollingStationComponent>()->GetDelayedPlayerPos(myEnemyRef->GetGameObject()->GetComponent<GroundTankLad>()->GetBlackboard().getInt("DelayValue"));
	//}
	//else

	playerPos = myEnemyRef->GetGameObject()->GetSingletonComponent<PollingStationComponent>()->GetPlayerPos();
	playerPos += {0, 75.f, 0};


	CU::Vector3f enemyPos = myBlackboard->getVec3f("ShootPosition");
	CU::Vector3f toPlayer = (playerPos - enemyPos).GetNormalized();

	CU::Vector3f enemyDir = myEnemyRef->GetTransform().Forward() * -1.f;

	myEnemyRef->GetGameObject()->GetScene()->RayCastAll(enemyPos, toPlayer, 100000000, eLayer::ALL, hits);
	//GDebugDrawer->DrawLine3D(DebugDrawFlags::Always, enemyPos, enemyDir * 100000000.f, 0.1f, { 1,0,0,1 });

	switch (myFireState)
	{
	case LaserBeam::eFireState::Charging:

		if (hits.size() > 0)
		{
			GDebugDrawer->DrawLine3D(DebugDrawFlags::Always, enemyPos, hits[hits.size() - 1].Position, 0.1f, { 1,0,0,1 });
		}

		if (myLaserTimer <= 0)
		{
			myLaserTimer = myFireTime;
			LOG_INFO(LogType::Animation) << "Firing";
			myFireState = eFireState::Firing;
		}
		break;
	case LaserBeam::eFireState::Firing:

		myBlackboard->GetAnimationController()->Trigger("ActivateShoot");
		myBlackboard->GetAudioComponent()->PlayEvent("LaserAttack");

		if (hits.size() > 0)
		{
			GDebugDrawer->DrawLine3D(DebugDrawFlags::Always, enemyPos, hits[1].Position, 0.1f, { 1,0,0,1 });

			if (hits[hits.size() - 1].GameObject->GetTag() == eTag::PLAYER && myDamageTimer <= 0)
			{
				hits[hits.size() - 1].GameObject->GetComponent<HealthComponent>()->ApplyDamage(myDamage);
				myDamageTimer = 1;
			}
		}
		else
		{
			GDebugDrawer->DrawLine3D(DebugDrawFlags::Always, myEnemyRef->GetTransform().GetPosition(), enemyDir * 100000000.f, 0.1f, { 1,0,0,1 });
		}

		/*if (hits.size() >= 2)
		{
			if (hits[1].GameObject->GetTag() == eTag::PLAYER)
			{
				hits[1].GameObject->GetComponent<HealthComponent>()->ApplyDamage(myDamage / myFireTime * aDeltaTime);
			}
		}*/

		if (myLaserTimer <= 0)
		{
			myLaserTimer = myCooldownTime;
			LOG_INFO(LogType::Animation) << "Cooldown";
			myFireState = eFireState::Cooldown;
		}
		break;
	case LaserBeam::eFireState::Cooldown:

		if (myLaserTimer <= 0)
		{
			LOG_INFO(LogType::Animation) << "Ready";
			myFireState = eFireState::Ready;
		}

		break;
	default:
		break;
	}

	myDamageTimer -= aDeltaTime;
	myLaserTimer -= aDeltaTime;
}

const LaserBeam::eFireState LaserBeam::GetFireState() const
{
	return myFireState;
}
