#include "pch.h"
#include "LaserTest.h"

#include "AI/BehaviorTree/Base/BT_Blackboard.h"
#include "AI/PollingStation/PollingStationComponent.h"
#include "AI/VFX/EnemyVFXRef.h"
#include "Components/HealthComponent.h"
#include "Engine/Shortcuts.h"
#include "Engine/GameObject/Prefab/GameObjectPrefab.h"
#include "Engine/GameObject/Components/ModelComponent.h"
#include "Engine/GameObject/Components/VFXComponent.h"
#include "AI\Enemies\GroundTankLad\GroundTankLad.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"
#include "Engine/GameObject/Components/AudioComponent.h"

LaserTest::LaserTest(GameObject* aGameObject, const GameObjectPrefabRef& aPrefab, const GameObjectPrefabRef& aPrefabSight, BT_Blackboard* aBlackboard,
	float aBeamUpTime, float aChargeUpTime) :
	Component(aGameObject),
	myLaserPrefab(aPrefab),
	myLaserSightPrefab(aPrefabSight),
	myBlackBoard(aBlackboard),
	myBeamUpTime(aBeamUpTime),
	myChargeUpTime(aChargeUpTime)
{
}

void LaserTest::Start()
{
	Component::Start();

	Engine::GameObjectPrefab& laserPrefab = myLaserPrefab->Get();
	myLaser = &laserPrefab.Instantiate(*myGameObject->GetScene());
	myLaser->GetTransform().SetPosition(myGameObject->GetTransform().GetPosition());

	/*Engine::GameObjectPrefab& laserHitPrefab = myLaserHitPrefab->Get();
	myLaserHit = &laserHitPrefab.Instantiate(*myGameObject->GetScene());
	myLaserHit->GetTransform().SetPosition(myGameObject->GetTransform().GetPosition());
	myLaserHit->SetActive(false);*/

	Engine::GameObjectPrefab& laserSightPrefab = myLaserSightPrefab->Get();
	mySightLaser = &laserSightPrefab.Instantiate(*myGameObject->GetScene());
	mySightLaser->GetTransform().SetPosition(myGameObject->GetTransform().GetPosition());

	myLaserHit = myGameObject->GetScene()->AddGameObject<GameObject>();
	myLaserHit->SetName("LaserHit");
	//myLaserHit->GetTransform().SetPosition(myLaser->GetTransform().GetPosition());

	const auto& vfxRef = myGameObject->GetSingletonComponent<EnemyVFXRef>()->GetLaserHit();
	myVFX = myLaserHit->AddComponent<Engine::VFXComponent>(vfxRef->Get());
	myVFX->Play();
	//myVFX->AutoDestroy();
	myLaserHit->SetActive(false);
}

void LaserTest::Execute(Engine::eEngineOrder aOrder)
{
	Component::Execute(aOrder);
	Update();
}

void LaserTest::Update()
{
	if (myBlackBoard->hasBool("IsDead"))
	{
		if (myBlackBoard->getBool("IsDead"))
			return;
	}

	if (!myLaserActive)
	{
		myLaser->GetTransform().SetScale(Vec3f(myLaser->GetTransform().GetScale().x, 0.f, 0.f));
		mySightLaser->GetTransform().SetScale(Vec3f(mySightLaser->GetTransform().GetScale().x, 0.f, 0.f));

		return;
	}

	{
		auto playerPos = myGameObject->GetSingletonComponent<PollingStationComponent>()->GetPlayerPos();

		auto length = (myGameObject->GetSingletonComponent<PollingStationComponent>()->GetPlayerPos() -
			myTargetPos).Length();

		myTargetPos = Math::Lerp(myTargetPos, playerPos, Time::DeltaTime * 4.f);
	}

	switch (myGTLaserState)
	{
	case GTLaserState::ChargeUp:
	{
		ChargeUp();
	}
	break;
	case GTLaserState::Beam:
	{
		Beam();
	}
	break;
	}

}

void LaserTest::Reflect(Engine::Reflector& aReflector)
{
	Component::Reflect(aReflector);
	aReflector.Reflect(myLaserActive, "Laser Activated");
	aReflector.Reflect(myLaserPrefab, "Laser Prefab");
}

void LaserTest::Render()
{
	Component::Render();
}

void LaserTest::ActivateLaser()
{
	myLaserActive = true;
	myHasHitOnce = false;
	myTargetPos = myGameObject->GetSingletonComponent<PollingStationComponent>()->GetPlayerPos();
	myBlackBoard->setVec3f("EyeColor", { 0,0,0 });
}

void LaserTest::ChargeUp()
{
	float laserLength = -1.f;

	myChargeUpTimer += Time::DeltaTime;
	if (myChargeUpTimer > myChargeUpTime)
	{
		myGTLaserState = GTLaserState::Beam;
		myChargeUpTimer = 0.f;
		myChargeUpTest = 0.1f;
		myBlackBoard->GetAnimationController()->Trigger("ActivateShoot");
		//mySightLaserVFX->SetActive(false);
		//mySightLaserVFX->SetActive(false);
		myBlackBoard->GetAudioComponent()->PlayEvent("LaserAttack");
		myLaserHit->SetActive(true);
		myLaserHit->GetComponent<Engine::VFXComponent>()->Play();
		//mySightLaserVFX->SetActive(false);
	}

	Vec3f newColor = myBlackBoard->getVec3f("EyeColor");

	newColor.y = 18.f * (myChargeUpTimer / myChargeUpTime);
	newColor.z = 127.f * (myChargeUpTimer / myChargeUpTime);

	myBlackBoard->setVec3f("EyeColor", newColor);


	Vec3f rayPos = myLaser->GetTransform().GetPosition();

	constexpr float rayRange = 10000.f;

	//Vec3f rayDirection = myGameObject->GetSingletonComponent<PollingStationComponent>()->GetPlayerPos() - myLaser->GetTransform().GetPosition();
	Vec3f rayDirection = myTargetPos - myLaser->GetTransform().GetPosition();
	rayDirection.Normalize();

	//mySightLaserVFX->GetTransform().SetPosition(myLaser->GetTransform().GetPosition() + rayDirection * 50.f);

	RayCastHit hit;
	if (myGameObject->GetScene()->RayCast(
		rayPos,
		rayDirection,
		rayRange,
		eLayer::DEFAULT | eLayer::PLAYER_DAMAGEABLE | eLayer::PLAYER_SHIELD,
		hit))
	{
		if (hit.GameObject)
		{
			laserLength = (mySightLaser->GetTransform().GetPosition() - hit.Position).Length();
		}

		/*	GDebugDrawer->DrawLine3D(
				DebugDrawFlags::AI,
				rayPos,
				hit.Position,
				5.f,
				{ 0.f,  1.f, 0.f, 1.f });*/
	}

	myLaser->GetTransform().SetScale(Vec3f(myLaser->GetTransform().GetScale().x, 0.f, 0.f));
	mySightLaser->GetTransform().SetScale(Vec3f(1.f, 1.f, laserLength / 100.f));

	auto& materialInstance = mySightLaser->GetComponent<Engine::ModelComponent>()->GetMeshMaterialInstanceByIndex(0);
	materialInstance.SetFloat4("myVar", Vec4f(100.f, 210.f, 1.f, 10.f));

	const Vec3f pos = myGameObject->GetTransform().GetPosition();
	const auto playerPos = myGameObject->GetSingletonComponent<PollingStationComponent>()->GetPlayerPos();

	myChargeUpTest += Time::DeltaTime * 50.f;

	//GDebugDrawer->DrawSphere3D(
	//	DebugDrawFlags::AI,
	//	pos,
	//	myChargeUpTest,
	//	0.f,
	//	{ 1.f,  0.f, 0.f, 1.f });

	//GDebugDrawer->DrawLine3D(
	//	DebugDrawFlags::AI,
	//	pos,
	//	playerPos, 0.f,
	//	{ 1.f,  0.f, 0.f, 1.f });

	mySightLaser->GetTransform().LookAt(myTargetPos);
	auto r2 = mySightLaser->GetTransform().GetRotation();
	r2 = r2 * Quatf(Vec3f(0.f, Math::PI, 0.f));
	mySightLaser->GetTransform().SetRotation(r2);
}

void LaserTest::Beam()
{
	float laserLength = -1.f;
	//if (!myHasHitOnce)
	//{
	Vec3f rayPos = myLaser->GetTransform().GetPosition();

	constexpr float rayRange = 10000.f;


	Vec3f rayDirection = myTargetPos - myLaser->GetTransform().GetPosition();
	rayDirection.Normalize();
	RayCastHit hit;
	if (myGameObject->GetScene()->RayCast(
		rayPos,
		rayDirection,
		rayRange,
		eLayer::DEFAULT | eLayer::PLAYER_DAMAGEABLE | eLayer::PLAYER_SHIELD,
		hit))
	{
		if (hit.GameObject)
		{
			if (hit.GameObject->GetLayer() & eLayer::PLAYER_DAMAGEABLE)
			{
				if (!myHasHitOnce)
				{
					if (!myHasHitOnce)
					{
						hit.GameObject->GetComponent<HealthComponent>()->ApplyDamage(myBlackBoard->getFloat("Damage"));
						myHasHitOnce = true;
					}

				}
			}
			laserLength = (myLaser->GetTransform().GetPosition() - hit.Position).Length();

		}

		/*	GDebugDrawer->DrawLine3D(
				DebugDrawFlags::AI,
				rayPos,
				hit.Position,
				5.f,
				{ 0.f,  1.f, 0.f, 1.f });*/
	}
	//}


	myBeamUpTimer += Time::DeltaTime;
	if (myBeamUpTimer > myBeamUpTime)
	{
		myBlackBoard->GetAudioComponent()->PlayEvent("LaserCharge");
		myGTLaserState = GTLaserState::ChargeUp;
		myBeamUpTimer = 0.f;
		myLaserActive = false;
		myLaserHit->SetActive(false);
		//mySightLaserVFX->SetActive(true);
	}

	myLaser->GetTransform().SetScale(Vec3f(/*myLaser->GetTransform().GetScale().x*/laserLength / 2000.f, 1.f, 1.f));
	mySightLaser->GetTransform().SetScale(Vec3f(mySightLaser->GetTransform().GetScale().x, 0.f, 0.f));
	myLaserHit->GetTransform().SetPosition(hit.Position);

	const auto playerPos = myGameObject->GetSingletonComponent<PollingStationComponent>()->GetPlayerPos();

	myLaser->GetTransform().LookAt(myTargetPos);
	auto r2 = myLaser->GetTransform().GetRotation();
	r2 = r2 * Quatf(Vec3f(0.f, Math::PI * 0.5f, 0.f));
	myLaser->GetTransform().SetRotation(r2);
}

GameObject* LaserTest::GetLaserGo()
{
	return myLaser;
}

GameObject* LaserTest::GetLaserSightGo()
{
	return mySightLaser;
}

void LaserTest::StopVFX()
{
	myLaser->Destroy();
	myLaserHit->Destroy();
	mySightLaser->Destroy();
}

