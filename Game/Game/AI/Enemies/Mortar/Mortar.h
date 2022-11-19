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

class Mortar : public EnemyBase
{
public:
	COMPONENT(Mortar, "Mortar");

	Mortar() = default;
	Mortar(GameObject* aGameObject);
	virtual ~Mortar() = default;

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
	int myShootBoneIndex = 0;

	float myColliderHeight = 150.f;
	float myColliderRadius = 150.f;
	float myColliderOffSetY = 0.f;

	//float myGravityTimer = 0.f;
	//float myGravityTime = 2.f;

	EnemyScalableValues myEnemyScalableStats;
	bool myHasFinsihedSpawning = false;
};

