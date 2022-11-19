#pragma once
#include "AI/BehaviorTree/Base/BT_BehaviorTree.h"
#include "Engine\GameObject\GameObject.h"
#include "AI/BehaviorTree/Base/BT_Blackboard.h"
#include "AI/Enemies/Base/EnemyBase.h"
#include "AI/Enemies/Base/EnemyScalableStats.h"
#include "AI/Health/EnemyHealth.h"

//class AiHealthComponent;

namespace Engine
{
	class RigidBodyComponent;
	class CharacterControllerComponent;
	class AudioComponent;
}

class BurstTank : public EnemyBase
{
public:
	COMPONENT(BurstTank, "BurstTank");

	BurstTank() = default;
	BurstTank(GameObject* aGameObject);
	virtual ~BurstTank() = default;

	void Start() override;

	void Execute(Engine::eEngineOrder aOrder) override;

	void Update(float aDeltaTime);

	void Reflect(Engine::Reflector& aReflector) override;

	BT_Blackboard& GetBlackboard();

	void InitAnimationCallbacks();

	void Render() override;
	void DebugDraw();
	void InitAudio();
	void OnDamage() override;
	void SpawnDeathSound() override;
private:
	//AiHealthComponent* myAIHealthComponent = nullptr;
	Engine::AnimationController* myAnimationController = nullptr;
	Engine::AudioComponent* myAudioComponent = nullptr;

	BT_BehaviorTree myBTree;
	BT_Blackboard myBlackboard;
	EnemyHealth* myEnemyHealth = nullptr;
	Engine::CharacterControllerComponent* cct = nullptr;

	float myShootingRange = 1500.f;

	float myColliderHeight = 150.f;
	float myColliderRadius = 150.f;
	float myColliderOffSetY = 0.f;

	float myMovementSpeed = 300.f;

	float myGroundCheckDistance = 1000.f;

	EnemyScalableValues myEnemyScalableStats;
	int myShootBoneIndex = 0;

	Vec3f test = { 0.f, 1000.f, 0.f };

};

