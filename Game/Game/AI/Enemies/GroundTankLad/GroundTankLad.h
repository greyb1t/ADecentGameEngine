#pragma once
#include "AI/BehaviorTree/Base/BT_BehaviorTree.h"
#include "Engine\GameObject\GameObject.h"
#include "AI/BehaviorTree/Base/BT_Blackboard.h"
#include "AI/Enemies/Base/EnemyBase.h"
#include "AI/Enemies/Base\EnemyScalableStats.h"
#include "AI\Enemies\Projectiles\GroundPound.h"
#include "AI\Enemies\Projectiles\LaserBeam.h"


namespace Engine
{
	class RigidBodyComponent;
	class CharacterControllerComponent;
	class AudioComponent;
}

class GroundTankLad : public EnemyBase
{
public:
	enum class EnemyState
	{
		Neutral,
		Pounding,
		FiringLaser
	};

	COMPONENT(GroundTankLad, "GroundTankLad");
	GroundTankLad() = default;
	GroundTankLad(GameObject* aGameObject);
	virtual ~GroundTankLad() = default;

	void Start() override;

	void Execute(Engine::eEngineOrder aOrder) override;

	void Update(float aDeltaTime);

	void Reflect(Engine::Reflector& aReflector) override;

	BT_Blackboard& GetBlackboard();

	void InitAnimationCallbacks();

	void IgniteLaser();

	void StartPounding();

	const LaserBeam::eFireState CheckLaserStatus() const;

	const GroundPound::eFireState CheckPoundStatus() const;

	void SetMyState(EnemyState aState);

	void Render() override;
	void DebugDraw();
	void InitAudio();
	void OnDeath();
	void SpawnDeathSound() override;
private:
	EnemyState myState = EnemyState::Neutral;

	float myColliderHeight = 150.f;
	float myColliderRadius = 150.f;
	float myColliderOffSetY = 0.f;

	float myMeleeAttackRange = 1500.f;
	float myLaserAttackRange = 500.f;

	float myLaserChargeTime = 3.f;
	float myLaserFireTime = 1.5f;
	float myLaserCoolDown = 4.f;
	float myStartLaserCoolDown = 0.f;

	float myPoundChargeTime = 3.f;
	float myPoundCooldown = 4.f;
	float myPoundColliderSize = 500.f;
	float myPoundPositionOffsetY = 0.f;
	int myTargetDelay = 0;

	bool mySafeDespawn = false;

	Vec3f myEyeColor = { 0,0,0 };

	float myMovementSpeed = 300.f;

	bool myHasFinishedSpawning = false;

	//AiHealthComponent* myAIHealthComponent = nullptr;
	Engine::AnimationController* myAnimationController = nullptr;
	GameObjectPrefabRef myEye;
	GameObjectPrefabRef myEyeEffect;
	GameObject* myEyeGameObject = nullptr;
	GameObject* myEyeEffectGameObject = nullptr;
	int myShootBoneIndex = 0;
	int myPoundBoneIndex = 0;
	Engine::AudioComponent* myAudioComponent = nullptr;
	
	LaserBeam myLaserBeam;
	GroundPound myGroundPound;
	BT_BehaviorTree myBTree;
	BT_Blackboard myBlackboard;

	Engine::CharacterControllerComponent* cct = nullptr;
	EnemyScalableValues myEnemyScalableStats;
	GameObject* myLaserGo = nullptr;

};

