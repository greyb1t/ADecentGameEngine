#pragma once
#include "AI/BehaviorTree/Base/BT_BehaviorTree.h"
#include "Engine\GameObject\GameObject.h"
#include "AI/BehaviorTree/Base/BT_Blackboard.h"
#include "AI/Enemies/Base/EnemyBase.h"
#include "AI/Enemies/Base\EnemyScalableStats.h"


namespace Engine
{
	class RigidBodyComponent;
	class CharacterControllerComponent;
	class AudioComponent;
}

class Slime : public EnemyBase
{
public:
	enum class EnemyState
	{
		Neutral,
		Jumping,
		Landing
	};

	COMPONENT(Slime, "Slime");
	Slime() = default;
	Slime(GameObject* aGameObject);
	virtual ~Slime() = default;

	void Start() override;

	void Execute(Engine::eEngineOrder aOrder) override;

	void Update(float aDeltaTime);

	void FinnishJump();

	void SummonPoisonCloud(float aSizeModifier = 1.f);

	void Reflect(Engine::Reflector& aReflector) override;

	BT_Blackboard& GetBlackboard();

	void InitAnimationCallbacks();

	void SetMyState(EnemyState aState);

	void Render() override;
	void DebugDraw();
	void InitAudio();
	void OnDeath();
	void OnDamage() override;
	void SpawnDeathSound() override;

	void SetRotation();

private:
	EnemyState myState = EnemyState::Neutral;

	CU::Vector3f myStartPos;
	CU::Vector3f myJumpDirection;
	CU::Vector3f myLastPosition = {0,0,0};

	float myColliderHeight = 150.f;
	float myColliderRadius = 150.f;
	float myColliderOffSetY = 0.f;

	bool myIsJumping = false;

	float myTimer;
	float myJumpRecharge;
	float myJumpSpeed = 200.f;
	float myJumpForce = 300.f;
	float myGravity = 982.f;

	float myLandingTime = 1.f;

	float myPoisonDPS = 0;
	float myPoisonCloudDuration = 0;
	float myPoisonCloudRadius = 0;

	bool myHasFinishedSpawning = false;
	bool myDeathFart = false;

	//AiHealthComponent* myAIHealthComponent = nullptr;
	Engine::AnimationController* myAnimationController = nullptr;
	Engine::AudioComponent* myAudioComponent = nullptr;
	BT_BehaviorTree myBTree;
	BT_Blackboard myBlackboard;
	Engine::VFXComponent* mySlamVFX = nullptr;
	Engine::VFXComponent* myPoisonVFX = nullptr;

	Engine::CharacterControllerComponent* cct = nullptr;
	EnemyScalableValues myEnemyScalableStats;
};

