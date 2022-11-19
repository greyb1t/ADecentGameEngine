#pragma once
#include "AI/BehaviorTree/Base/BT_BehaviorTree.h"
#include "Engine\GameObject\GameObject.h"
#include "AI/BehaviorTree/Base/BT_Blackboard.h"
#include "AI/Enemies/Base/EnemyBase.h"
#include "AI/Enemies/Base/EnemyScalableStats.h"

//class AiHealthComponent;

class EnemyHealth;

namespace Engine
{
	class RigidBodyComponent;
	class CharacterControllerComponent;
	class AudioComponent;
}

class FlyPopcorn : public EnemyBase
{
public:
	COMPONENT(FlyPopcorn, "FlyPopcorn");

	FlyPopcorn() = default;
	FlyPopcorn(GameObject* aGameObject);
	virtual ~FlyPopcorn() = default;

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

	void WingForce();
private:
	//AiHealthComponent* myAIHealthComponent = nullptr;
	Engine::AnimationController* myAnimationController = nullptr;

	BT_BehaviorTree myBTree;
	BT_Blackboard myBlackboard;
	EnemyHealth* myEnemyHealth = nullptr;
	Engine::CharacterControllerComponent* cct = nullptr;

	Engine::AudioComponent* myAudioComponent = nullptr;

	float myShootingRange = 1500.f;

	float myColliderHeight = 150.f;
	float myColliderRadius = 150.f;
	float myColliderOffSetY = 0.f;

	float myGroundCheckDistance = 1000.f;

	GameObjectPrefabRef myEye;
	GameObject* myEyeGameObject = nullptr;
	int myEyeBoneIndex = 0;

	int myShootBoneIndex = 0;

	bool myActivateWingForce = false;
	float myMovementSpeed = 300.f;
	//float mySpeed = 0.f;
};

