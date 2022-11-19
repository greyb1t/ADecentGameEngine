#pragma once
#include "Engine/GameObject/Components/Component.h"
#include "Engine\GameObject\GameObject.h"
#include "AI/BehaviorTree/Base/BT_Blackboard.h"

class BT_Blackboard;

enum class GTLaserState
{
	ChargeUp,
	Beam,

	Count
};

class LaserTest : public Component
{
public:
	COMPONENT(LaserTest, "LaserTest");

	LaserTest() = default;
	LaserTest(GameObject * aGameObject, const GameObjectPrefabRef& aPrefab, const GameObjectPrefabRef& aPrefabSight, BT_Blackboard* aBlackboard,
		float aBeamUpTime, float aChargeUpTime);
	virtual ~LaserTest() = default;

	void Start() override;

	void Execute(Engine::eEngineOrder aOrder) override;

	void Update();

	void Reflect(Engine::Reflector& aReflector) override;

	void Render() override;

	void ActivateLaser();
	void ChargeUp();
	void Beam();

	GameObject* GetLaserGo();
	GameObject* GetLaserSightGo();

	void StopVFX();

private:
	GameObjectPrefabRef myLaserPrefab;
	GameObjectPrefabRef myLaserSightPrefab;
	GameObject* myLaser = nullptr;
	GameObject* myLaserHit = nullptr;
	GameObject* mySightLaser = nullptr;
	GameObject* mySightLaserVFX = nullptr;
	BT_Blackboard* myBlackBoard = nullptr;
	GTLaserState myGTLaserState = GTLaserState::ChargeUp;


	Vec3f myTargetPos = {0.f, 0.f, 0.f};

	Engine::VFXComponent* myVFX = nullptr;

	float myChargeUpTimer = 0.f;
	float myChargeUpTime = 2.f;

	float myBeamUpTimer = 0.f;
	float myBeamUpTime = 1.f;

	float myChargeUpTest = 0.1f;

	bool myLaserActive = false;
	bool myHasHitOnce = false;
};

