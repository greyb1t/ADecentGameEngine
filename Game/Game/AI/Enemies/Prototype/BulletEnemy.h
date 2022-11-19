#pragma once
#include "AI/BehaviorTree/Base/BT_BehaviorTree.h"
#include "Engine\GameObject\GameObject.h"
#include "AI/BehaviorTree/Base/BT_Blackboard.h"
#include "AI/Enemies/Base/EnemyBase.h"

//class AiHealthComponent;

namespace Engine
{
	class RigidBodyComponent;
	class CharacterControllerComponent;
}

class BulletEnemy : public EnemyBase
{
public:
	COMPONENT(BulletEnemy, "BulletEnemy");

	BulletEnemy() = default;
	BulletEnemy(GameObject* aGameObject);
	~BulletEnemy();

	void Start() override;

	void Execute(Engine::eEngineOrder aOrder) override;

	void Update(float aDeltaTime);

	void Reflect(Engine::Reflector& aReflector) override;

	BT_Blackboard& GetBlackboard();

	void InitAnimationCallbacks();

	void Render() override;
	void DebugDraw();
	void InitAudio();
private:
	//AiHealthComponent* myAIHealthComponent = nullptr;
	Engine::AnimationController* myAnimationController = nullptr;

	BT_BehaviorTree myBTree;
	BT_Blackboard myBlackboard;

	Engine::CharacterControllerComponent* cct = nullptr;
};

