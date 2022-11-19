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

class GTTest : public EnemyBase
{
public:
	COMPONENT(GTTest, "GTTest");

	GTTest() = default;
	GTTest(GameObject* aGameObject);
	virtual ~GTTest() = default;

	void Start() override;

	void Execute(Engine::eEngineOrder aOrder) override;

	void Update(float aDeltaTime);

	void Reflect(Engine::Reflector& aReflector) override;

	void OnDamage() override;

	BT_Blackboard& GetBlackboard();

	void InitAnimationCallbacks();

	void Render() override;
	void DebugDraw();
	void InitAudio();
private:
	//AiHealthComponent* myAIHealthComponent = nullptr;
	Engine::AnimationController* myAnimationController = nullptr;
	Engine::AudioComponent* myAudioComponent = nullptr;

	BT_BehaviorTree myBTree;
	BT_Blackboard myBlackboard;
	EnemyHealth* myEnemyHealth = nullptr;
	Engine::CharacterControllerComponent* cct = nullptr;

	float myColliderHeight = 150.f;
	float myColliderRadius = 150.f;
	float myColliderOffSetY = 0.f;

	float myShootingRange = 1500.f;
	float myMoveSpeed = 500.f;

	int myShootBoneIndex = 0;

	bool myHasFinsihedSpawning = false;

	//EnemyScalableValues myEnemyScalableStats;
};

