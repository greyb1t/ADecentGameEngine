#include "pch.h"
#include "Player.h"

#include "VFXParticleAttraction.h"
#include "Engine/GameObject/GameObject.h"
#include "AI/PollingStation/PollingStationComponent.h"
#include "Components/HealthComponent.h"
#include "Camera/CameraController.h"
#include "Engine/GameObject/Components/AudioComponent.h"
#include "Engine/GameObject/Components/CharacterControllerComponent.h"
#include "Engine/GameObject/Components/ModelComponent.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "Engine/GameObject/Prefab/GameObjectPrefab.h"
#include "Engine/Reflection/Reflector.h"
#include "Engine/Navmesh/NavmeshComponent.h"
#include "Engine/Navmesh/NavMesh.h"
#include "HUD/HUDHandler.h"
#include "HUD/HUDHealth.h"
#include "States/DashState.h"
#include "States/PlayerDeathState.h"
#include "States/MovementState.h"
#include "States/PlayerEnterBossScriptedState.h"
#include "HUD\HUDHandler.h"
#include "HUD\HUDAbility.h"
#include "Items/ItemManager.h"
#include "Components/InteractableManager.h"
#include "Components/Interactable.h"
#include "Components/InventoryComponent.h"

#include "Game/GameManager/GameManager.h"
#include "Game/UI/MainMenu/LoadingScreen.h"
#include <Components\WalletComponent.h>
#include "Engine/ResourceManagement/Resources/VFXResource.h"
#include "Engine/GameObject/Components/VFXComponent.h"
#include "PlayerSpawnHandler.h"

Player::Player(GameObject* aGameObject)
	: Component(aGameObject)
{
}

Player::~Player()
{
	delete myDashState;
	myDashState = nullptr;
	delete myDeathState;
	myDeathState = nullptr;
	delete myMovementState;
	myMovementState = nullptr;

	if (Main::Exists())
	{
		Main::SetPlayer(nullptr);
	}

	//getengine.getviewport

}

void Player::Reflect(Engine::Reflector& aReflector)
{
	Component::Reflect(aReflector);

	aReflector.Reflect(myExplosionPrefab, "Explosion Prefab");

	aReflector.Header("----------------- Player Stats ----------------");
	aReflector.Header("Life");
	aReflector.Reflect(myStats.HealthRef(), "Health");
	aReflector.Reflect(myVFXHolder.heal, "Heal VFX");
	aReflector.Reflect(myHealHandler.HealTimeRef(), "Heal VFX Emit Time");

	aReflector.Reflect(myVFXHolder.death, "Death VFX");
	aReflector.Reflect(mySettings.death.invisibleDelay, "Invisibility Delay");

	aReflector.Header("Movement");
	aReflector.Reflect(myGroundingAngle, "Max Walk Angle");
	aReflector.Reflect(myStats.MovementSpeedRef(), "Movement Speed");
	aReflector.Reflect(myStats.SprintMultiplierRef(), "Sprint Multiplier");
	aReflector.Reflect(myStats.GetMovementLagRef(), "Movement Lag");
	aReflector.Reflect(myStats.JumpForceRef(), "Jump Force");
	aReflector.Reflect(myStats.JumpAmountRef(), "Jump Amount");
	aReflector.Reflect(myStats.AmountJumpedRef(), "Amount Jumped");
	aReflector.Reflect(myVFXHolder.airJump, "Air Jump VFX");

	aReflector.ReflectGroup([&]()
		{
			aReflector.Reflect(myVFXHolder.sprintDust.normalDust, "Normal Dust");
		}, "Sprint Dust");

	aReflector.Header("Knockback");
	aReflector.Reflect(mySettings.knockback.airStrength, "Air Strength");
	aReflector.Reflect(mySettings.knockback.groundStrength, "Ground Strength");
	aReflector.Reflect(mySettings.knockback.stopThreshold, "Stop Threshold");

	if (myHealth)
	{
		aReflector.Reflect(myHealth->GetDodgeChance(), "Dodge Chance(0,1)");
	}
	aReflector.ReflectGroup([&]()
		{
			aReflector.Reflect(GetSettings().animation.upperBodyWeightSpeed, "Upper Body lerp speed");
			aReflector.Reflect(mySettings.combat.combatReadyTime, "Combat Ready Time");
		}, "Animation");

	aReflector.ReflectGroup([&]()
		{
			aReflector.Reflect(mySettings.leftHand.offset, "Offset");
		}, "Left Hand");
	/*
	aReflector.ReflectGroup([&]()
		{
			aReflector.Reflect(mySettings.rightHand.offset, "Offset");
		}, "Right Hand");*/

	aReflector.Header("Combat");
	aReflector.Reflect(myStats.DamageRef(), "Damage");
	aReflector.Reflect(myStats.AttackSpeedRef(), "Attack Speed");
	aReflector.Reflect(myStats.CritChance(), "Crit Chance");
	aReflector.Reflect(myStats.CritDamageMultiplierRef(), "Crit Damage Multiplier");
	aReflector.Reflect(myStats.SteadinessRecoveryRateRef(), "Steadiness Recovery Rate");

	aReflector.ReflectGroup([&]()
		{
			aReflector.Reflect(myCrossHair.RotationRef(), "Rotation");
			aReflector.Reflect(myCrossHair.InnerRef(), "Inner Distance");
			aReflector.Reflect(myCrossHair.SteadinessOffsetRef(), "Steadiness Offset");


			aReflector.Header("Charge");
			aReflector.Reflect(myCrossHair.ChargeRotationSpeedRef(), "Charge Rotation Speed");
			aReflector.Reflect(mySettings.crossHair.secondary.rotationSpeedCurve, "Charge Rotation Speed Lerp");
			aReflector.Reflect(mySettings.crossHair.secondary.returnTime, "Return Time");
			aReflector.Reflect(mySettings.crossHair.secondary.returnLerp, "End Return Lerp");


			aReflector.Header("Hit Marker");
			aReflector.Reflect(mySettings.crossHair.hitmark.timeActive, "Time Active");
			aReflector.Reflect(mySettings.crossHair.hitmark.alphaCurve, "Alpha Curve");
			aReflector.Reflect(mySettings.crossHair.hitmark.sizeCurve, "Size Curve");
		}, "Crosshair");

	aReflector.ReflectGroup([&]()
		{
			aReflector.Reflect(myVFXHolder.magicCircle.mainRune, "MainRune");
			aReflector.Reflect(myVFXHolder.magicCircle.smallRune, "Small Rune");
			aReflector.Reflect(myMagicCircleHandler.OffsetRef(), "Offset");
			aReflector.Reflect(myMagicCircleHandler.OffsetSmallRef(), "Small Rune Offset");

			aReflector.Header("Main Rotation");
			aReflector.Reflect(mySettings.magicCircle.primaryRotation.rotationAmount, "Rotation Amount");
			aReflector.Reflect(mySettings.magicCircle.primaryRotation.rotationLerp, "Rotation Lerp");

			aReflector.Header("Main Recoil");
			aReflector.Reflect(mySettings.magicCircle.primaryRotation.recoilStrength, "Recoil Strength");
			aReflector.Reflect(mySettings.magicCircle.primaryRotation.recoilLerp, "Recoil Lerp");
		}, "Magic Circle");

	aReflector.Reflect(myIsHoming, "Homing Projectiles");

	aReflector.Header("Gravity");
	aReflector.Reflect(myStats.GravityRef(), "Gravity");
	aReflector.Reflect(myStats.TerminalVelocityRef(), "Terminal Velocity");

	aReflector.Reflect(myStats.IsFallDamageActiveRef(), "Fall Damage Activated");
	aReflector.Reflect(myStats.FallDamageMinRef(), "Min Fall Dmg");
	aReflector.Reflect(myStats.FallDamageMaxRef(), "Max Fall Dmg");
	aReflector.Reflect(myStats.FallSpeedMinRef(), "Min Speed Threshold Dmg");
	aReflector.Reflect(myStats.FallSpeedMaxRef(), "Max Speed Threshold Dmg");


	aReflector.Header("Collider");
	aReflector.Reflect(myCapsuleHeight, "Collider Height");
	aReflector.Reflect(myCapsuleRadius, "Collider Radius");
	aReflector.Header("Ground Extras");
	aReflector.Reflect(myGroundCheckBuffer, "Grounded Check Range");

	aReflector.ReflectGroup([&]()
		{
			aReflector.Header("----------------- Primary ----------------");
			aReflector.Reflect(mySkillStats.primary.damageMultiplier, "Damage Multiplier");
			aReflector.Reflect(mySkillStats.primary.attackSpeedMultiplier, "Attack Speed Multiplier");
			aReflector.Reflect(mySkillStats.primary.projectileSpeed, "Projectile Speed");
			aReflector.Reflect(mySkillStats.primary.projectileRange, "Projectile Range");

			aReflector.Reflect(mySkillStats.primary.steadinessSpread, "Spread");
			aReflector.Reflect(mySkillStats.primary.steadinessShootLoss, "Shoot Steadiness Loss");
			aReflector.Reflect(mySkillStats.primary.steadinessWeightPower, "Spread Weight Power");

			aReflector.Reflect(myVFXHolder.primary.projectile, "Projectile Prefab");
			aReflector.Reflect(myVFXHolder.primary.fire, "Fire VFX");
			aReflector.Reflect(myVFXHolder.primary.hit, "Hit VFX");
		}, "Primary");

	aReflector.ReflectGroup([&]()
		{
			aReflector.Header("----------------- Secondary ----------------");

			aReflector.Reflect(mySkillStats.secondary.cooldown, "Cooldown");
			aReflector.Reflect(mySkillStats.secondary.chargeTime, "Charge Time");

			aReflector.Reflect(mySkillStats.secondary.damageRadius, "Projectile Radius");
			aReflector.Reflect(mySkillStats.secondary.baseDamage, "Base Damage");
			aReflector.Reflect(mySkillStats.secondary.playerDamageMultiplier, "Player Damage Multiplier");
			aReflector.Reflect(mySkillStats.secondary.minDamageMultiplier, "Min Damage Multiplier");
			aReflector.Reflect(mySkillStats.secondary.maxDamageMultiplier, "Max Damage Multiplier");

			aReflector.Reflect(mySkillStats.secondary.explosionMinRange, "Min Explosion Range");
			aReflector.Reflect(mySkillStats.secondary.explosionMaxRange, "Max Explosion Range");
			aReflector.Reflect(mySkillStats.secondary.explosionDamageMultiplier, "Explosion Damage Multiplier");
			aReflector.Reflect(mySkillStats.secondary.explosionKnockback, "Explosion Knockback Strength");

			aReflector.Reflect(mySkillStats.secondary.minProjectileSpeed, "Min Projectile Speed");
			aReflector.Reflect(mySkillStats.secondary.maxProjectileSpeed, "Max Projectile Speed");

			aReflector.Reflect(mySkillStats.secondary.projectileRange, "Projectile Range");

			aReflector.Reflect(myVFXHolder.secondary.projectilePrefab, "Projectile Prefab");
			aReflector.Reflect(myVFXHolder.secondary.projectile, "Projectile VFX");
			aReflector.Reflect(myVFXHolder.secondary.chargeUp, "ChargeUp VFX");
			aReflector.Reflect(myVFXHolder.secondary.muzzleFlash, "Muzzle Flash VFX");
			aReflector.Reflect(myVFXHolder.secondary.hit, "Hit VFX");
			aReflector.Reflect(myVFXHolder.secondary.explosionPrefab, "Explosion Prefab");
			aReflector.Reflect(myVFXHolder.secondary.explosion, "Explosion VFX");
			aReflector.Reflect(myVFXHolder.secondary.explosionScaling, "Explosion Scaling");
			aReflector.Reflect(mySkillStats.secondary.explosionTime, "Explosion Time");
		}, "Secondary");

	aReflector.ReflectGroup([&]()
		{
			aReflector.Header("----------------- Mobility ----------------");
			aReflector.Reflect(myVFXHolder.dashVFX, "Dash VFX");
			aReflector.Reflect(myVFXHolder.dashOutVFX, "Dash Exit VFX");
			aReflector.Reflect(mySkillStats.mobility.cooldown, "Cooldown");
			aReflector.Reflect(mySkillStats.mobility.dashDistance, "Dash Distance");
			aReflector.Reflect(mySkillStats.mobility.dashTime, "Dash Time");
			aReflector.Reflect(mySkillStats.mobility.dashInTime, "Dash In Time");
			aReflector.Reflect(mySkillStats.mobility.dashOutTime, "Dash Out Time");
		}, "Mobility");

	aReflector.ReflectGroup([&]()
		{
			aReflector.Header("----------------- Utility ----------------");
			aReflector.Reflect(mySkillStats.utility.cooldown, "Cooldown");
			aReflector.Reflect(myStats.CooldownMultiplierRef(), "Cooldown Multiplier");
			aReflector.Reflect(mySkillStats.utility.baseDamage, "Base Damage");
			aReflector.Reflect(mySkillStats.utility.damageMultiplier, "Damage Multiplier");
			aReflector.Reflect(mySkillStats.utility.bounces, "Bounces");
			aReflector.Reflect(mySkillStats.utility.bounceRange, "Bounce Range");

			aReflector.Reflect(mySkillStats.utility.projectileSpeed, "Projectile Speed");
			aReflector.Reflect(mySkillStats.utility.bounceProjectileSpeed, "Bouncing Projectile Speed");
			aReflector.Reflect(mySkillStats.utility.projectileRange, "Bounce Range");

			aReflector.Reflect(myVFXHolder.utility.projectile, "Projectile VFX");
			aReflector.Reflect(myVFXHolder.utility.hit, "Hit VFX");
		}, "Utility");

	aReflector.Header("----------------- Reflect ----------------");
	aReflector.Reflect(myHomingProjectilePrefab, "Homing Projectile Prefab");
	aReflector.Reflect(myLoadingScreenObj, "Loading Screen Object");
	aReflector.Reflect(myLandingVFX, "Landing VFX");

}

void Player::Awake()
{
	auto* pollingStation = myGameObject->GetSingletonComponent<PollingStationComponent>();
	if (pollingStation)
		pollingStation->Setplayer(this);

	myGameObject->SetLayers(eLayer::PLAYER | eLayer::PLAYER_DAMAGEABLE, eLayer::GROUND | eLayer::DEFAULT);
	myGameObject->SetTag(eTag::PLAYER);
	InitAudio();
	myModel = GetGameObject()->FindComponent<Engine::ModelComponent>();
	InitAnimator();
	Respawn();
}

void Player::Start()
{
	InitHealth();
	InitController();

	InitHUDHandler();


	myCameraController = myGameObject->GetComponent<CameraController>();
	assert(myCameraController && "Player has no camera controller :(");

	myCrossHair = CrossHairHandler(this);
	mySteadiness = Steadiness(myStats.GetSteadinessRecoveryRate());

	myStats.ObserveChange([&](PlayerStats::eVariable aVariable)
		{
			if (aVariable == PlayerStats::eVariable::COOLDOWN_REDUCTION)
			{
				float secondaryCooldown = GetSkillStats().secondary.cooldown * GetStats().GetCooldownMultiplier();
				mySecondary.SetCooldown(secondaryCooldown);
				myHUDHandler->GetAbility(HUDHandler::AbilityType::SecondaryFire)->SetCooldownTime(secondaryCooldown);

				float mobilityCooldown = GetSkillStats().mobility.cooldown * GetStats().GetCooldownMultiplier();
				myMobility.SetCooldown(mobilityCooldown);
				myHUDHandler->GetAbility(HUDHandler::AbilityType::Mobility)->SetCooldownTime(mobilityCooldown);

				float utilityCooldown = GetSkillStats().utility.cooldown * GetStats().GetCooldownMultiplier();
				myUtility.SetCooldown(utilityCooldown);
				myHUDHandler->GetAbility(HUDHandler::AbilityType::Utiliy)->SetCooldownTime(utilityCooldown);
			}
		});

	InitSkills();
	InitStates();
	myHealHandler.Init(this);
	myMagicCircleHandler.Init(this);
	mySecondaryArm.Init(this);

	if (myLoadingScreenObj.IsValid())
	{
		myLoadingScreen = myLoadingScreenObj.Get()->GetComponent<LoadingScreen>();
	}

	if (mySpawnState == eSpawnState::WaitingForStrike)
	{
		GameObject* spawnerObject = myGameObject->GetScene()->AddGameObject<GameObject>();
		PlayerSpawnHandler* spawnHandler = spawnerObject->AddComponent<PlayerSpawnHandler>();
		spawnHandler->Init(this, myCameraController);

		myStatus.overrideHUD = true;
		myHUDHandler->SetCrosshairVisible(false);
	}

	Time::TimeScale = 1.0f;
	myAudioComponent->ReleaseAudio();
	myAudioComponent->PlayEvent("Spawn");
}

void Player::FinishRespawnSequence()
{
	myAnimator->GetController().AddStateOnExitCallback("BaseLayer", "SE_Intro", [this]
		{
			//GetAudioComponent().PlayEvent("EnterPortal");
			mySpawnState = eSpawnState::Done;
		});

	myAnimator->GetController().Trigger("Intro");
	if (myLandingVFX->IsValid())
	{
		GameObject* vfxObject = myGameObject->GetScene()->AddGameObject<GameObject>();
		vfxObject->GetTransform().SetPosition(GetTransform().GetPosition());
		Engine::VFXComponent* comp = vfxObject->AddComponent<Engine::VFXComponent>(myLandingVFX->Get());
		comp->AutoDestroy();
		comp->Play();
	}
	myHUDHandler->SetCrosshairVisible(true);
	myStatus.overrideHUD = false;
}

void Player::Respawn()
{
	if (Main::SetPlayer(myGameObject))
	{
		Main::ClearProgress();
		mySpawnState = eSpawnState::WaitingForStrike;
	}
}

bool Player::IsSpawning()
{
	return mySpawnState != eSpawnState::Done;
}


void Player::Execute(Engine::eEngineOrder aOrder)
{
	Update();
	UpdateAudioReset();
}

void Player::Render()
{
	// Set the player pos for the shaders
	myGameObject->GetScene()->GetRendererScene().SetPlayerPos(GetTransform().GetPosition());
}

bool Player::IsGrounded() const
{
	return myIsGrounded;
}

bool Player::IsStartedFallingThisFrame() const
{
	return myStartedFallingThisFrame;
}

bool Player::GetStartedFallingAudio() const
{
	return myStartedFallingAudio;
}

void Player::SetStartedFallingThisFrame(bool aValue)
{
	myStartedFallingThisFrame = aValue;
}

void Player::SetStartedFallingAudio(bool aValue)
{
	myStartedFallingAudio = aValue;
}

void Player::ResetAudio()
{
	myAudioComponent->ReleaseAudio();
}

StateMachine& Player::GetStateMachine()
{
	return myStateMachine;
}

PlayerStats& Player::GetStats()
{
	return myStats;
}

PlayerStatus& Player::GetStatus()
{
	return myStatus;
}

PlayerSettings& Player::GetSettings()
{
	return mySettings;
}

SkillStats& Player::GetSkillStats()
{
	return mySkillStats;
}

const Vec3f& Player::GetInputDir() const
{
	return myInputDirection;
}

float Player::GetHeight() const
{
	return myCapsuleHeight;
}

float Player::GetRadius() const
{
	return myCapsuleRadius;
}

float Player::GetGroundedBufferHeight() const
{
	return myGroundCheckBuffer;
}

float Player::GetGroundingAngle() const
{
	return myGroundingAngle;
}

bool Player::GetSpawnStateIsFinished()
{
	return mySpawnState == eSpawnState::Done;
}

VelocityHandler& Player::GetVelocity()
{
	return myVelocity;
}

CrossHairHandler& Player::GetCrosshair()
{
	return myCrossHair;
}

Steadiness& Player::GetSteadiness()
{
	return mySteadiness;
}

PlayerVFXHolder& Player::GetVFXHolder()
{
	return myVFXHolder;
}

MagicCircleHandler& Player::GetMagicCircle()
{
	return myMagicCircleHandler;
}

SecondaryArm& Player::GetLeftArm()
{
	return mySecondaryArm;
}

CameraController& Player::GetCameraController() const
{
	return *myCameraController;
}

Engine::CharacterControllerComponent& Player::GetController() const
{
	return *myController;
}

Engine::AudioComponent& Player::GetAudioComponent() const
{
	return *myAudioComponent;
}

Engine::ModelComponent& Player::GetModelComponent() const
{
	return *myModel;
}

Engine::AnimatorComponent& Player::GetAnimatorComponent() const
{
	return *myAnimator;
}

HUDHandler* Player::GetHUDHandler()
{
	return myHUDHandler;
}

GameObject& Player::GetModelObject() const
{
	return *myModel->GetGameObject();
}

PistolSkill& Player::GetPrimarySkill()
{
	return myPrimary;
}

PiercingSkill& Player::GetSecondarySkill()
{
	return mySecondary;
}

DashSkill& Player::GetMobilitySkill()
{
	return myMobility;
}

AOESkill& Player::GetUtilitySkill()
{
	return myUtility;
}

MovementState* Player::GetMovementState() const
{
	return myMovementState;
}

DashState* Player::GetDashState() const
{
	return myDashState;
}

PlayerDeathState* Player::GetDeathState() const
{
	return myDeathState;
}

PlayerEnterBossScriptedState* Player::GetEnterBossScriptedState() const
{
	return myEnterBossScriptedState;
}

GameObjectPrefabRef Player::GetExplosionPrefab()
{
	return myExplosionPrefab;
}

GameObjectPrefabRef Player::GetHomingprojectilePrefab()
{
	return myHomingProjectilePrefab;
}

const Vec3f Player::GetHeadPosition() const
{
	return myGameObject->GetTransform().GetPosition() + Vec3f(0, myCapsuleHeight * .5f - 10.0f, 0);
}

void Player::InitHealth()
{
	if (auto* health = myGameObject->GetComponent<HealthComponent>())
	{
		myHealth = health;
		myHealth->SetMaxHealth(myStats.GetHealth());
	}
	else
	{
		myHealth = myGameObject->AddComponent<HealthComponent>(myStats.GetHealth());
	}


	myHealth->ObserveDamage([&](float aDmg)
		{
			OnDamage(aDmg);
		});
	myHealth->ObserveHealing([&](float aHeal)
		{
			OnHeal(aHeal);
		});
	myHealth->ObserveDeath([&](float aDmg)
		{
			OnDeath(aDmg);
		});
	myHealth->ObserveKnockback([&](const Vec3f& aKnockback)
		{
			OnKnockback(aKnockback);
		});

	myHealth->ObserveShield([&](float aShield, float aMaxShield)
		{
			ShieldChange(aShield, aMaxShield);
		});
	myHealth->ObserveBarrier([&](float aBarrier)
		{
			BarrierChange(aBarrier);
		});
}

void Player::InitController()
{
	myController = myGameObject->AddComponent<Engine::CharacterControllerComponent>();
	myController->Init(myCapsuleHeight - myCapsuleRadius * 2.f, myCapsuleRadius);
	myController->SetGravity(0);
	myController->SetUpdatesPerSecond(-1);
	myController->SetMovementSettings(0.05f);
	myController->ObserveTriggerEnter([](GameObject* obj)
		{
			LOG_INFO(LogType::Viktor) << "Player trigger enter";
		});

}

void Player::InitAnimator()
{
	myAnimator = GetGameObject()->FindComponent<Engine::AnimatorComponent>();
	assert(myAnimator && "Player has no animator component :(");

	myAnimator->GetController().SetBool("IsDead", false);
	myAnimator->GetController().SetBool("IsDashing", false);

	myAnimator->GetController().AddEventCallback("Footstep_Player",
		[this]()
		{
			// CALLS FOOTSTEP EVENT IN IDLE ANIMATION FOR SOME REASON
			myAudioComponent->PlayEvent("Footstep");
		});
}

void Player::InitHUDHandler()
{
	myHUDHandler = myGameObject->GetComponent<HUDHandler>();
	if (myHUDHandler)
	{
		myHUDHandler->GetAbility(HUDHandler::AbilityType::SecondaryFire)->SetCooldownTime(mySkillStats.secondary.cooldown);
		myHUDHandler->GetAbility(HUDHandler::AbilityType::Mobility)->SetCooldownTime(mySkillStats.mobility.cooldown);
		myHUDHandler->GetAbility(HUDHandler::AbilityType::Utiliy)->SetCooldownTime(mySkillStats.utility.cooldown);

		myHUDHandler->GetHealth()->SetMaxHealth(myHealth->GetMaxHealth());
		myHUDHandler->GetHealth()->SetHealth(myHealth->GetHealth());
	}
}

void Player::InitAudio()
{
	myAudioComponent = myGameObject->AddComponent<Engine::AudioComponent>();

	myAudioComponent->AddEvent("Damage", /**/"event:/SFX/PLR/PlayerDamaged"/**/);
	myAudioComponent->AddEvent("Death", /**/"event:/SFX/PLR/PlayerDeath"/**/);
	myAudioComponent->AddEvent("Footstep", /**/"event:/FOL/PLR/Footstep"/**/);
	myAudioComponent->AddEvent("Sprint", /**/"event:/FOL/PLR/Footstep"/**/);
	myAudioComponent->AddEvent("PrimaryAttack", /**/"event:/SFX/PLR/PrimaryAttack"/**/);
	myAudioComponent->AddEvent("Jump", /**/"event:/FOL/PLR/Jump"/**/);
	myAudioComponent->AddEvent("ExtraJump", /**/"event:/SFX/PLR/Pickups/ExtraJump"/**/);
	myAudioComponent->AddEvent("EnterPortal", /**/"event:/SFX/INT/EnterPortal"/**/);
	//myAudioComponent->AddEvent("Fall", /**/"event:/FOL/PLR/Footstep"/**/);
	myAudioComponent->AddEvent("Land", /**/"event:/FOL/PLR/Land"/**/);
	myAudioComponent->AddEvent("Spawn", /**/"event:/SFX/PLR/PlayerSpawn"/**/);
	myAudioComponent->AddEvent("Dash", /**/"event:/FOL/PLR/Dash"/**/);
	myAudioComponent->AddEvent("Utility", /**/"event:/FOL/PLR/Footstep"/**/);
	myAudioComponent->AddEvent("Heal", /**/"event:/FOL/PLR/Footstep"/**/);
	myAudioComponent->AddEvent("Interact", /**/"event:/FOL/PLR/Footstep"/**/);
	myAudioComponent->AddEvent("SecondaryAttack", /**/"event:/SFX/PLR/SecondaryAttack"/**/);
	myAudioComponent->AddEvent("SecondaryCharge", /**/"event:/SFX/PLR/ChargeAttack"/**/);
	myAudioComponent->AddEvent("Knockback", "event:/SFX/PLR/Knockback");
	myAudioComponent->AddEvent("Ability2Sound", /**/"event:/FOL/PLR/Footstep"/**/);
}

void Player::InitSkills()
{
	myPrimary = PistolSkill(this);
	mySecondary = PiercingSkill(this);
	myMobility = DashSkill(this);
	myUtility = AOESkill(this);

	mySecondary.SetCooldown(GetSkillStats().secondary.cooldown);
	myMobility.SetCooldown(GetSkillStats().mobility.cooldown);
	myUtility.SetCooldown(GetSkillStats().utility.cooldown);

	myMobility.Observe([&]()
		{
			Dash();
		});
}

void Player::InitStates()
{
	myStateMachine.Init();

	myMovementState = DBG_NEW MovementState(this);
	myDashState = DBG_NEW DashState(this);
	myDeathState = DBG_NEW PlayerDeathState(this);
	myEnterBossScriptedState = new PlayerEnterBossScriptedState(this);
	myStateMachine.AddState(myMovementState);
	myStateMachine.AddState(myDeathState);
	myStateMachine.AddState(myDashState);

	myStateMachine.AddTransitionGlobal([&]()
		{
			return nullptr;
		});

	myStateMachine.SetState(myMovementState);
}

void Player::Update()
{
#ifndef _RETAIL
	auto input = GetEngine().GetInputManager();

	if (input.IsKeyDown(C::KeyCode::K))
	{
		myHealth->ApplyDamage(10.0f);
	}
	if (input.IsKeyDown(C::KeyCode::L))
	{
		myHealth->ApplyHealing(10.0f);
	}
	if (input.IsKeyDown(C::KeyCode::I))
	{
		myHealth->AddPermanentShield(10.0f);
	}
	if (input.IsKeyDown(C::KeyCode::O))
	{
		myHealth->AddBarrier(10.0f);
	}
	if (input.IsKeyDown(C::KeyCode::P))
	{
		//GameManager::GetInstance()->ChangeLevel();
	}
	if (input.IsKeyDown(C::KeyCode::N))
	{
		myCrossHair.HitMark();
	}
	if (input.IsKeyDown(C::KeyCode::H))
	{
		myGameObject->GetComponent<WalletComponent>()->IncreaseGold(100000);
	}
	if (input.IsKeyDown(C::KeyCode::G))
	{
		auto& items = Main::GetItemManager().GetAllItems();
		for (auto& item : items)
		{
			Main::GetPlayer()->GetComponent<InventoryComponent>()->AddItem(*item);
		}
	}
#endif // !_RETAIL

	VFXParticleAttraction::SetAttractionPoint(GetTransform().GetPosition());

	if (IsSpawning())
	{
		CalculateGrounded();
		UpdateSteadiness();
		UpdateVelocity();
		UpdateAnimations();
		GetTransform().Move(myVelocity.Get() * Time::DeltaTime);
		myHealHandler.Update();
		myCrossHair.Update();
		myMagicCircleHandler.Update();
		mySecondaryArm.Update();
		return;
	}

	UpdateInputDirection();

	CalculateGrounded();

	myStateMachine.Update();
	UpdateSkills();
	UpdateSteadiness();
	UpdateVelocity();
	UpdateInteraction();
	UpdateAnimations();
	GetTransform().Move(myVelocity.Get() * Time::DeltaTime);

	myHealHandler.Update();
	if (!GetStatus().overrideHUD)
		myCrossHair.Update();
	myMagicCircleHandler.Update();
	mySecondaryArm.Update();
}

void Player::UpdateInputDirection()
{
	myInputDirection = { 0, 0, 0 };

	if (GetEngine().GetInputManager().IsKeyPressed(CU::KeyCode::W))
		myInputDirection.z += 1;

	if (GetEngine().GetInputManager().IsKeyPressed(CU::KeyCode::S))
		myInputDirection.z -= 1;

	if (GetEngine().GetInputManager().IsKeyPressed(CU::KeyCode::A))
		myInputDirection.x -= 1;

	if (GetEngine().GetInputManager().IsKeyPressed(CU::KeyCode::D))
		myInputDirection.x += 1;
}

void Player::UpdateSkills()
{
	myPrimary.Update();
	mySecondary.Update();
	myMobility.Update();
	myUtility.Update();
}
void Player::PlayFootstep()
{
	myAudioComponent->PlayEvent("Footstep");
}

void Player::UpdateVelocity()
{
	if (GetStatus().overrideVelocity)
		return;

	auto& velocity = myVelocity.Ref();

	if (velocity.y > 1)
	{
		RayCastHit hit;
		if (GetGameObject()->GetScene()->RayCast(GetTransform().GetPosition(), Vec3f(0, 1, 0), GetHeight() * .5f + velocity.y * Time::DeltaTime + 1, eLayer::GROUND | eLayer::DEFAULT, hit))
		{
			velocity.y = 0;
			LOG_INFO(LogType::Game) << "Player Hit ceiling";
		}
	}

	if (IsGrounded())
	{
		if (velocity.y < 0)
		{
			if (!myIsGroundedLastFrame)
			{
				GetAnimatorComponent().GetController().Trigger("Landed");

				const float minFall = GetStats().GetFallSpeedMin();
				const float maxFall = GetStats().GetFallSpeedMax();
				if (GetStats().GetIsFallDamageActive() && abs(velocity.y) > minFall)
				{
					const float fall = std::min(abs(velocity.y), maxFall);
					const float dmg = Math::Lerp(GetStats().GetFallDamageMin(), GetStats().GetFallDamageMax(), (fall - minFall) / (maxFall - minFall));
					myHealth->ApplyDamage(dmg);
				}
			}
			velocity.y = 0;
			GetStatus().isFalling = false;
		}

		if (std::abs(velocity.x) > GetSettings().knockback.stopThreshold)
		{
			velocity.x -= velocity.x * GetSettings().knockback.groundStrength * Time::DeltaTime;
		}
		else
		{
			velocity.x = 0;
		}
		if (std::abs(velocity.z) > GetSettings().knockback.stopThreshold)
		{
			velocity.z -= velocity.z * GetSettings().knockback.groundStrength * Time::DeltaTime;
		}
		else
		{
			velocity.z = 0;
		}

	}
	else
	{
		if (std::abs(velocity.x) > GetSettings().knockback.stopThreshold)
		{
			velocity.x -= velocity.x * GetSettings().knockback.airStrength * Time::DeltaTime;
		}
		else
		{
			velocity.x = 0;
		}
		if (std::abs(velocity.z) > GetSettings().knockback.stopThreshold)
		{
			velocity.z -= velocity.z * GetSettings().knockback.airStrength * Time::DeltaTime;
		}
		else
		{
			velocity.z = 0;
		}

		// TODO: Do terminal velocity
		velocity.y -= myStats.GetGravity() * Time::DeltaTime;
		if (velocity.y < -myStats.GetTerminalVelocity())
		{
			velocity.y += myStats.GetGravity() * 1.5f * Time::DeltaTime;
		}

		GetStatus().isFalling = true;
	}

	GetAnimatorComponent().GetController().SetBool("IsGrounded", IsGrounded());
	GetAnimatorComponent().GetController().SetFloat("VelocityY", GetVelocity().Get().y);
	GetTransform().Move(myVelocity.Get() * Time::DeltaTime);
}

void Player::UpdateSteadiness()
{
	//LOG_INFO(LogType::Game) << "Steadiness: " << mySteadiness.Get();
	mySteadiness.Update();
}

void Player::UpdateInteraction()
{
	Interactable* interactable = InteractableManager::Instance().SearchInteractable(GetTransform().GetPosition(), GetGameObject()->GetScene()->GetMainCameraGameObject()->GetTransform().Forward());
	if (interactable)
	{
		if (GetEngine().GetInputManager().IsKeyDown(interactable->GetKey()))
		{
			if (interactable->CanInteract())
			{
				interactable->Interact();
			}
		}
	}
}

void Player::UpdateAnimations()
{
	GetStatus().upperBodyWeight = C::Lerp(GetStatus().upperBodyWeight, GetStatus().upperBodyWeightGoal, Time::DeltaTime * GetSettings().animation.upperBodyWeightSpeed);
	GetAnimatorComponent().GetController().SetLayerWeight("UpperBody", GetStatus().upperBodyWeight);
	GetAnimatorComponent().GetController().SetBool("IsMoving", GetStatus().isMoving);
}

void Player::UpdateAudioReset()
{
	myAudioTimer += Time::DeltaTime;
	if (myAudioTimer > myAudioTime)
	{
		ResetAudio();
		myAudioTimer = 0;
	}
}

void Player::Dash()
{
	myAudioComponent->PlayEvent("Dash");
	myStateMachine.SetState(myDashState);
}

void Player::CalculateGrounded()
{
	myIsGroundedLastFrame = myIsGrounded;

	if (myController->IsGrounded())
	{
		myIsGrounded = true;
		return;
	}

	if (myVelocity.Get().y <= 0)
	{
		/*RayCastHit hit;
		const float range = GetHeight() * 0.5f + GetGroundedBufferHeight();
		if (GetGameObject()->GetScene()->RayCast(GetTransform().GetPosition(), Vec3f(0, -1, 0), range, eLayer::GROUND | eLayer::DEFAULT, hit))
		{
			myIsGrounded = true;
			return;
		}*/

		std::vector<OverlapHit> hits;
		if (GetGameObject()->GetScene()->SphereCastAll(GetTransform().GetPosition() - Vec3f(0, myCapsuleHeight * 0.5f - myCapsuleRadius, 0) + Vec3f(0, -GetGroundedBufferHeight(), 0), myCapsuleRadius - 1.f, eLayer::GROUND | eLayer::DEFAULT, hits))
		{
			myIsGrounded = true;
			return;
		}
	}
	myIsGrounded = false;
}


void Player::OnDamage(float aDmg)
{
	ItemEventData data;
	data.damage = aDmg;
	data.direction = { 0.f, 0.f, 0.f };
	data.hitPos = GetTransform().GetPosition();
	data.targetObject = myGameObject;

	Main::GetItemManager().PostEvent(eItemEvent::OnPlayerDamaged, &data);

	//Play Damage Sound
	myAudioComponent->PlayEvent("Damage");

	LOG_INFO(LogType::Game) << "Player damaged: " << aDmg;

	myHUDHandler->GetHealth()->SetHealth(myHealth->GetHealth());
}

void Player::OnHeal(float aHealing)
{
	//HEaling sound?
	myHealHandler.Heal();

	myHUDHandler->GetHealth()->SetMaxHealth(myHealth->GetMaxHealth());
	myHUDHandler->GetHealth()->SetHealth(myHealth->GetHealth());
}

void Player::OnDeath(float aDmg)
{
	LOG_INFO(LogType::Game) << "Player Died";
	//DisconnectCamera();
	//myGameObject->Destroy();

	//myHealth->ResetHealthToMax();
	myAudioComponent->PlayEvent("Death");
	myHUDHandler->GetHealth()->SetHealth(0.0f); //TODO: Because we just reset the player on death, set HUD to max health, when we change in the future, set HUD to 0hp
	if (GetStatus().isAlive)
	{
		GetStatus().isAlive = false;
		GetStateMachine().SetState(myDeathState);
	}

	GetGameObject()->GetComponent<InventoryComponent>()->Clear();
}

void Player::ShieldChange(float aShield, float aMaxShield)
{
	myHUDHandler->GetHealth()->SetShields(aShield, aMaxShield);
}

void Player::BarrierChange(float aBarrier)
{
	myHUDHandler->GetHealth()->SetBarrier(aBarrier);
}

void Player::OnKnockback(const Vec3f& aKnock)
{
	myAudioComponent->PlayEvent("Knockback");
	myVelocity.Add(aKnock);
	if (aKnock.y > 1)
		GetAnimatorComponent().GetController().Trigger("Jump");
}

void Player::ActivateLoadingScreen()
{
	if (!myLoadingScreen)
	{
		LOG_ERROR(LogType::Components) << "Missing loading screen reference on player. No loading screen was activated.";
		return;
	}
	myLoadingScreen->GetGameObject()->SetActive(true);
	Time::TimeScale = 0.0f;
}
