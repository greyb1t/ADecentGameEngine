#include "pch.h"
#include "BulletEnemy.h"

#include "AI/BehaviorTree/Base/BT_Builder.h"
#include "AI/BehaviorTree/Base/BT_Sequence.h"
#include "AI/GeneralNodes/BTN_FlyTowardsPlayer.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "Engine/GameObject/Components/CharacterControllerComponent.h"
#include "Engine/Reflection/Reflector.h"

BulletEnemy::BulletEnemy(GameObject* aGameObject) :
	::EnemyBase(aGameObject)
{
}

BulletEnemy::~BulletEnemy()
{
}

void BulletEnemy::Start()
{
	EnemyBase::Start();

	
	cct = myGameObject->AddComponent<Engine::CharacterControllerComponent>();
	cct->Init(90.f, 30.f);
	cct->SetMovementSettings(.01f);
	cct->SetOffset(Vec3f(50.f, 50.f, 50.f));
	cct->SetGravity(false);

	myGameObject->SetLayers(eLayer::ENEMY, eLayer::DEFAULT, eLayer::DEFAULT);
	myGameObject->SetTag(eTag::ENEMY);

	myAnimationController = &GetGameObject()->GetComponent<Engine::AnimatorComponent>()->GetController();

	myBlackboard = BT_Blackboard(GetGameObject(), myAnimationController);
	//myBTree.getBlackboard()->SetOwner(GetGameObject());


	/*myAIHealthComponent = myGameObject->GetComponent<AiHealthComponent>();
	myAIHealthComponent->Init(
		myBlackboard.getFloat("MaxHealth"),
		&myBlackboard,
		myAIType,
		cct,
		GetGroupID(),
		[this]()
		{

		});*/

	myBTree = BT_Builder()
		.composite<BT_Sequence>()
		//.leaf<BTN_Print>("Då", true)
		.leaf<BTN_FlyTowardsPlayer>(&myBlackboard, this)
		.end()
		.build();

	InitAnimationCallbacks();
}

void BulletEnemy::Execute(Engine::eEngineOrder aOrder)
{
	EnemyBase::Execute(aOrder);// Mandatory to be able to work correctly, do not remove!

	Update(Time::DeltaTime); // Mandatory to be able to work correctly, do not remove!
}

void BulletEnemy::Update(float aDeltaTime)
{
	myBTree.update();
}

void BulletEnemy::Reflect(Engine::Reflector& aReflector)
{
	EnemyBase::Reflect(aReflector);
}

BT_Blackboard& BulletEnemy::GetBlackboard()
{
	return myBlackboard;
}

void BulletEnemy::InitAnimationCallbacks()
{
	//myBlackboard.GetAnimationController()->AddEventCallback(
	//	"RunFirstFrame",
	//	[this]()
	//	{
	//		myAudioComponent->PlayEvent("LabCoatRun"); // SoundImplementation LabCoatRun
	//	}
	//);


	/*myAnimationController->AddStateOnExitCallback(
		"Base Layer",
		"Aggro",
		[this]()
		{
			myBlackboard.setBool("AggroAnimOnExitCallback", true);
		}
	);*/
}

void BulletEnemy::Render()
{
	EnemyBase::Render();
	DebugDraw();
}

void BulletEnemy::DebugDraw()
{

	/*GDebugDrawer->DrawSphere3D(
		DebugDrawFlags::AI,
		aggroPos,
		myBlackboard.getFloat("AggroRange"),
		0.f,
		{ 1.f, 0.f, 0.f, 1.f });*/
}

void BulletEnemy::InitAudio()
{
	/*myAudioComponent = myGameObject->AddComponent<Engine::AudioComponent>();
	myAudioComponent->AddEvent("TEST", "event:/FOL/NPC/Scientists/ScientistAgroRunMale1");

	myBlackboard.SetAudioComponent(myAudioComponent);*/
}