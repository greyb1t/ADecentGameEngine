#pragma once
#include "CrossHairHandler.h"
#include "PlayerStats.h"
#include "PlayerStatus.h"
#include "SkillStats.h"
#include "StateMachine.h"
#include "VelocityHandler.h"
#include "Engine/GameObject/Components/Component.h"
#include "Prototype/Skills/AOESkill.h"
#include "Prototype/Skills/DashSkill.h"
#include "Prototype/Skills/PiercingSkill.h"
#include "Prototype/Skills/PistolSkill.h"
#include "Shooting/Steadiness.h"
#include "Player/PlayerVFXHolder.h"
#include "HealHandler.h"
#include "Player/MagicCircleHandler.h"
#include "Player/PlayerSettings.h"
#include "Player/SecondaryArm.h"

class PlayerDeathState;
class MovementState;
class DashState;
class PlayerEnterBossScriptedState;
class HUDHandler;
class HealthComponent;
class CameraController;
class LoadingScreen;

namespace Engine
{
	class AnimatorComponent;
	class ModelComponent;
	class AudioComponent;
	class CharacterControllerComponent;
}

class Player : public Component
{
public:
	COMPONENT(Player, "Player");

	Player() = default;
	Player(GameObject* aGameObject);
	~Player() override; 

	void Reflect(Engine::Reflector& aReflector) override;

	void Awake() override;
	void Start() override;

	void FinishRespawnSequence();

	void Execute(Engine::eEngineOrder aOrder) override;

	void Render() override;

	bool IsGrounded() const;
	bool IsStartedFallingThisFrame() const;
	bool GetStartedFallingAudio() const;

	void SetStartedFallingThisFrame(bool aValue);
	void SetStartedFallingAudio(bool aValue);

	void ResetAudio();

	StateMachine&		GetStateMachine();
	PlayerStats&		GetStats();
	PlayerStatus&		GetStatus();
	PlayerSettings&		GetSettings();
	SkillStats&			GetSkillStats();

	VelocityHandler&	GetVelocity();
	CrossHairHandler&	GetCrosshair();
	Steadiness&			GetSteadiness();
	PlayerVFXHolder&	GetVFXHolder();
	MagicCircleHandler&	GetMagicCircle();
	SecondaryArm&		GetLeftArm();

	const Vec3f&		GetInputDir() const;

	float				GetHeight() const;
	float				GetRadius() const;
	float				GetGroundedBufferHeight() const;
	float				GetGroundingAngle() const;

	bool				GetSpawnStateIsFinished();

	CameraController&	GetCameraController() const;
	Engine::CharacterControllerComponent& GetController() const;
	Engine::AudioComponent&		GetAudioComponent() const;
	Engine::ModelComponent&		GetModelComponent() const;
	Engine::AnimatorComponent&	GetAnimatorComponent() const;
	HUDHandler*			GetHUDHandler();

	GameObject& GetModelObject() const;

	PistolSkill&		GetPrimarySkill();
	PiercingSkill&		GetSecondarySkill();
	DashSkill&			GetMobilitySkill();
	AOESkill&			GetUtilitySkill();

	MovementState*		GetMovementState() const;
	DashState*			GetDashState() const;
	PlayerDeathState*	GetDeathState() const;
	PlayerEnterBossScriptedState*	GetEnterBossScriptedState() const;

	GameObjectPrefabRef GetExplosionPrefab();
	GameObjectPrefabRef GetHomingprojectilePrefab();

	const Vec3f GetHeadPosition() const;
private:
	void InitHealth();
	void InitController();
	void InitAnimator();
	void InitHUDHandler();
	void InitAudio();
	void InitSkills();
	void InitStates();

	void Respawn();

	void Update();

	bool IsSpawning();
	void UpdateInputDirection();
	void UpdateSkills();
	void PlayFootstep();
	void UpdateVelocity();
	void UpdateSteadiness();
	void UpdateInteraction();
	void UpdateAnimations();
	void UpdateAudioReset();

	void Dash();

	void CalculateGrounded();

	void OnDamage(float aDmg);
	void OnHeal(float aHealing);
	void OnDeath(float aDmg);
	void ShieldChange(float aShield, float aMaxShield);
	void BarrierChange(float aBarrier);

	void OnKnockback(const Vec3f& aKnock);

private:
	GameObjectPrefabRef myExplosionPrefab;
	GameObjectPrefabRef myHomingProjectilePrefab;

	VFXRef myLandingVFX;

	Engine::CharacterControllerComponent* myController = nullptr;
	CameraController*		myCameraController = nullptr;
	HUDHandler*				myHUDHandler = nullptr;

	HealthComponent*		myHealth = nullptr;
	Engine::AudioComponent* myAudioComponent = nullptr;
	Engine::ModelComponent* myModel = nullptr;
	Engine::AnimatorComponent* myAnimator = nullptr;

	GameObject*		myHead = nullptr;

	PlayerStats		myStats;
	PlayerStatus	myStatus;
	PlayerSettings	mySettings;
	SkillStats		mySkillStats;

	PistolSkill		myPrimary;
	PiercingSkill	mySecondary;
	DashSkill		myMobility;
	AOESkill		myUtility;

	Steadiness		mySteadiness;

	Vec3f			myInputDirection;

	StateMachine		myStateMachine;
	VelocityHandler		myVelocity;
	CrossHairHandler	myCrossHair;
	PlayerVFXHolder		myVFXHolder;
	HealHandler			myHealHandler;
	MagicCircleHandler	myMagicCircleHandler;
	SecondaryArm		mySecondaryArm;

	MovementState*	myMovementState = nullptr;
	DashState*		myDashState = nullptr;
	PlayerDeathState* myDeathState = nullptr;
	PlayerEnterBossScriptedState* myEnterBossScriptedState = nullptr;

	enum class eSpawnState : unsigned int
	{
		WaitingForStrike,
		InAnimation,
		Done
	} mySpawnState = eSpawnState::Done;

	float myCapsuleHeight = 180;
	float myCapsuleRadius = 25;
	float myGroundingAngle = 60;
	float myGroundCheckBuffer = 5;
	float myAudioTimer = 0;
	float myAudioTime = 120;

	bool myIsGroundedLastFrame = false;
	bool myStartedFallingThisFrame = false;
	bool myStartedFallingAudio = false;
	bool myIsGrounded = false;
	bool myIsHoming = false;

	// This is ugly, sorry, By Pontus
	// Alltså förlåt för detta men jag måste, blunda bara
	Engine::GameObjectRef myLoadingScreenObj;
	LoadingScreen* myLoadingScreen = nullptr;
	public:
		// Activates loading screen and pauses the game (timescale 0.0)
		void ActivateLoadingScreen();

};

