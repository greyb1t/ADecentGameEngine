#pragma once
#include "AI/BehaviorTree/Base/BT_BehaviorTree.h"
#include "Engine\GameObject\GameObject.h"
#include "AI/BehaviorTree/Base/BT_Blackboard.h"
#include "AI/Enemies/Base/EnemyBase.h"
#include "AI/Enemies/Base\EnemyScalableStats.h"
#include "AI\Enemies\Projectiles\LaserBeam.h"


namespace Engine
{
	class RigidBodyComponent;
	class CharacterControllerComponent;
	class AudioComponent;
}

class GlassCannon : public EnemyBase
{
public:
	COMPONENT(GlassCannon, "GlassCannon");

	GlassCannon() = default;
	GlassCannon(GameObject* aGameObject);
	virtual ~GlassCannon() = default;

	void Start() override;

	void Execute(Engine::eEngineOrder aOrder) override;

	void Update(float aDeltaTime);

	void Reflect(Engine::Reflector& aReflector) override;

	BT_Blackboard& GetBlackboard();

	void InitAnimationCallbacks();

	void IgniteLaser();

	LaserBeam::eFireState CheckLaserStatus();

	void Render() override;
	void DebugDraw();
	void InitAudio();
private:

	float myLaserChargeTime = 3.f;
	float myLaserFireTime = 1.5f;
	float myLaserCoolDown = 4.f;

	//AiHealthComponent* myAIHealthComponent = nullptr;
	Engine::AnimationController* myAnimationController = nullptr;
	Engine::AudioComponent* myAudioComponent = nullptr;
	LaserBeam myLaserBeam;
	BT_BehaviorTree myBTree;
	BT_Blackboard myBlackboard;

	Engine::CharacterControllerComponent* cct = nullptr;
	EnemyScalableValues myEnemyScalableStats;
};

