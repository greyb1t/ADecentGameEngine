#include "pch.h"
#include "LevelBoss.h"

#include "Engine/Engine.h"
#include "Engine/Reflection/Reflector.h"
#include "Engine/GameObject/GameObject.h"
#include "Game/GameManager/GameManager.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"

#pragma region Components
#include "Game/Items/ItemManager.h"
#include "Game/Player/Player.h"
#include "Components/Interactable.h"
#include "Components/HealthComponent.h"
#include "Components/Item/ItemChoice.h"
#include "Engine/GameObject/Components/VFXComponent.h"
#include "Engine/GameObject/Components/ModelComponent.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "Engine/GameObject/Components/SkyboxComponent.h"
#include "Engine/GameObject/Components/MusicManager.h"
#include "Engine/GameObject/Components/CameraShakeComponent.h"
#include "Engine/GameObject/Components/AudioComponent.h"
#include "AI/PollingStation/PollingStationComponent.h"
#include "AI/AIDirector/AIDirector.h"
#include "Objects/BossDome.h"
#pragma endregion

#pragma region States
#include "State/BaseState.h"
#include "State/EngageState.h"
#include "State/UnengagedState.h"
#include "State/RoamState.h"
#include "State/GatlingEyeState.h"
#include "State/PoisonCloudState.h"
#include "State/MortarState.h" 
#include "State/SpikeAttackState.h"
#include "State/DisengageState.h"
#include "State/DeathState.h"
#pragma endregion

#include "Common/Random.h"
#include "Common/CommonUtilities.h"

LevelBoss::LevelBoss(GameObject* aParentObject) : Component(aParentObject)
{
}

LevelBoss::~LevelBoss()
{
	for (auto& state : myStates)
	{
		delete state.second;
		state.second = nullptr;
	}
}

void LevelBoss::Awake()
{
	myWasInitialized = true;
	InitComponents();
}

void LevelBoss::Start()
{
	if (!myWasInitialized)
	{
		LOG_ERROR(LogType::Game) << "Level Boss could not be initialized and was removed!";
		myGameObject->Destroy();
		return;
	}

	myHealthBarChild.Get()->SetActive(false);

	InitStateCache();
	InitStartValues();
	InitStateMachine();
	InitAudio();
}

void LevelBoss::Execute(Engine::eEngineOrder aOrder)
{
	if (!myPlayerObject)
	{
		LOG_ERROR(LogType::Game) << "Player could not be found!";
		return;
	}

	if (!myCurrentState)
	{
		LOG_ERROR(LogType::Game) << "Current state is nullptr!";
		return;
	}

	UpdatePortal();

	myInteractable->SetCanInteract(myState == +LevelBossState::Unengaged || (myCanEnterNextLevel && myState == +LevelBossState::Death));

	if (IsDisengaged())
	{
		auto gm = GameManager::GetInstance();
		if (auto& difficulty = gm->GetGameDifficulty())
		{
			if (myGameDifficulty != difficulty)
			{
				myGameDifficulty = difficulty;
				myHealth->SetMaxHealthModifier(GetHealthMultiplier());
			}
		}
	}

	if (myHealth->GetHealth() <= 0.0f && !IsDead())
	{
		ChangeState(LevelBossState::Death);
		return;
	}

	if (!myHasSpawned)
	{
		// Kopierat fr�n EngageState::LookTowards(), g�r att snygga till men orkar nog inte, vem bryr sig - Pontus

		Vec3f position = GetTransform().GetPosition();
		Vec3f targetPosition = myGameObject->GetSingletonComponent<PollingStationComponent>()->GetPlayerPos();
		Vec3f direction = (targetPosition - position).GetNormalized();

		const float angle = std::atan2f(direction.x, direction.z) + CU::PI;

		Quatf startRot = GetTransform().GetRotation();
		Quatf endRot = Quatf(Vec3f(
			startRot.EulerAngles().x,
			-angle,
			startRot.EulerAngles().z
		));

		// Framerate independant, men fungerar inte n�r man roterar �ver 360 grader, d� tar den omv�gen
		//Vec3f euler = Math::SmoothDamp(startRot.EulerAngles(), endRot.EulerAngles(), mySmoothDampVelocity, 0.25f, 45.0f, Time::DeltaTime);
		//GetTransform().SetRotation(Quatf(euler));

		// Framerate dependant (yikes!), men fungerar i alla grader
		Quatf lerpRot = Quatf::Slerp(startRot, endRot, 7.5f * Time::DeltaTime);
		GetTransform().SetRotation(lerpRot);
		GetTransform().SetRotation(lerpRot);
		return;
	}

	if (!IsDead())
	{
		myShockwaveProgress -= Time::DeltaTime;
		if (myShockwaveProgress <= 0.0f)
		{
			auto scene = myGameObject->GetScene();
			std::vector<OverlapHit> hits;
			if (scene->SphereCastAll(myGameObject->GetTransform().GetPosition(), myShockwaveRange, eLayer::PLAYER_DAMAGEABLE, hits))
			{
				auto player = Main::GetPlayer();
				for (auto& hit : hits)
				{
					if (player && hit.GameObject == player)
					{
						KnockPlayer();
					}
				}
			}
		}

		if (myOnHitIndicatorProgress > 0.0f)
		{
			myOnHitIndicatorProgress -= Time::DeltaTime;
			
			auto& mesh = myModel->GetMeshMaterialInstanceByIndex(0);
			Vec4f fl4 = mesh.GetFloat4("emissiveStr");

			if (myOnHitIndicatorProgress <= 0.0f)
			{
				myOnHitIndicatorProgress = 0.0f;
				fl4.z = 0.0f;
			}
			else 
			{
				fl4.z = 1.0f;
			}
			mesh.SetFloat4("emissiveStr", fl4);
		}
	}

	myCurrentState->Update();
	for (auto& state : myStates)
	{
		if (state.second)
		{
			state.second->ForceUpdate();
		}
	}
}

void LevelBoss::Render()
{
	//GDebugDrawer->DrawSphere3D(DebugDrawFlags::Always, myGameObject->GetTransform().GetPosition(), myShockwaveRange, 0.0f, Vec4f(0, 1, 0, 1));
}

void LevelBoss::Engage()
{
	ChangeState(LevelBossState::Idle);

	if (myHealthBarChild.IsValid())
	{
		if (!myHealthBarChild.Get()->IsDestroyed())
		{
			myHealthBarChild.Get()->SetActive(true);
		}
	}
}

void LevelBoss::Disengage()
{
	ChangeState(LevelBossState::Disengaged);
}

void LevelBoss::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Header("DEBUG");

	bool gatlingEye = IsAbilityUnlocked(LevelBossState::GatlingEye);
	if (aReflector.Reflect(gatlingEye, "GatlingEye Debug") & Engine::ReflectorResult_Changed)
	{
		gatlingEye ? AddAbilityDebug(LevelBossState::GatlingEye) : RemoveAbilityDebug(LevelBossState::GatlingEye);
	}

	bool poisonCloud = IsAbilityUnlocked(LevelBossState::PoisonCloud);
	if (aReflector.Reflect(poisonCloud, "PoisonCloud Debug") & Engine::ReflectorResult_Changed)
	{
		poisonCloud ? AddAbilityDebug(LevelBossState::PoisonCloud) : RemoveAbilityDebug(LevelBossState::PoisonCloud);
	}

	bool mortar = IsAbilityUnlocked(LevelBossState::Mortar);
	if (aReflector.Reflect(mortar, "Mortar Debug") & Engine::ReflectorResult_Changed)
	{
		mortar ? AddAbilityDebug(LevelBossState::Mortar) : RemoveAbilityDebug(LevelBossState::Mortar);
	}

	bool spike = IsAbilityUnlocked(LevelBossState::SpikeAttack);
	if (aReflector.Reflect(spike, "Spike Attack Debug") & Engine::ReflectorResult_Changed)
	{
		spike ? AddAbilityDebug(LevelBossState::SpikeAttack) : RemoveAbilityDebug(LevelBossState::SpikeAttack);
	}

	if (aReflector.Button("Reset Boss"))
	{
		myLevelIndex = 1;

		ResetBoss();
		InitStateCache();

		AddAbility(myLevelIndex);
	}

	aReflector.Reflect(myDisplayScaling, "Display Scaling Settings");

	if (myDisplayScaling)
	{
		aReflector.Header("Health Scaling");
		for (int i = 0; i < GameDifficulty::_size(); i++)
		{
			std::string label = "HS_" + std::string(GameDifficulty::_from_index(i)._to_string());
			aReflector.Reflect(myHealthScalings[i], label);
			if (myHealthScalings[i] < 1.0f)
				myHealthScalings[i] = 1.0f;
		}

		aReflector.Header("Damage Scaling");
		for (int i = 0; i < GameDifficulty::_size(); i++)
		{
			std::string label = "DS_" + std::string(GameDifficulty::_from_index(i)._to_string());
			aReflector.Reflect(myDmgScalings[i], label);
			if (myDmgScalings[i] < 1.0f)
				myDmgScalings[i] = 1.0f;
		}
	}

	aReflector.Header("Settings");

	aReflector.Reflect(myDomeObject, "Dome Object");
	aReflector.Reflect(myPortalObject, "Portal Object");
	aReflector.Reflect(myPortalSwirlObject, "Portal Swirl Object");
	aReflector.Reflect(myEyeMesh, "Eye Mesh Object");
	aReflector.Reflect(myPortalVFXObj, "Portal VFX Object");
	aReflector.Reflect(myItemPrefab, "Item Choice Prefab");
	
	int index = myLevelIndex;
	if (aReflector.Reflect(index, "Level Index") & Engine::ReflectorResult_Changed)
	{
		if (index < 1)
			index = 1;

		AddAbility(index);
	}

	if (aReflector.ReflectEnum(myState, "State", Engine::ReflectionFlags::ReflectionFlags_ReadOnly) & Engine::ReflectorResult_Changed)
	{
		if (!myStates.empty())
		{
			if(myState != 0)
				ChangeState(myState);
		}
	}

	aReflector.Reflect(myShockwaveRange, "Shockwave Range");
	aReflector.Reflect(myShockwaveForce, "Shockwave Force");
	aReflector.Reflect(myShockwaveVFXRef, "Shockwave VFX");
	aReflector.Reflect(myChanceForLegendary, "Chance For Legendary Drop");


	aReflector.Reflect(myRoamVars, "Idle");
	aReflector.Reflect(myDisengageVars, "Disengage");
	aReflector.Reflect(myGatlingEyeVars, "Gatling Eye");
	aReflector.Reflect(myPoisonCloudVars, "Poison Cloud");
	aReflector.Reflect(myMortarVars, "Mortar");
	aReflector.Reflect(mySpikeAttackVars, "Spike Attack");
	aReflector.Reflect(myHealthBarChild, "Health bar child");
}

void LevelBoss::ChangeState(const LevelBossState aState)
{
	if (myCurrentState)
	{
		myCurrentState->OnExitState();
	}
	myCurrentState = myStates[aState];
	myCurrentState->OnEnterState();

	myState = aState;

	OnChangeState(aState);
}

const LevelBossState& LevelBoss::GetCurrentState() const
{
	return myState;
}

void LevelBoss::SetMaximumCooldown()
{
	myGatlingEyeVars.myCooldownDuration = Random::RandomFloat(myGatlingEyeVars.myCooldownDurationMin, myGatlingEyeVars.myCooldownDurationMax);
	myGatlingEyeVars.myCooldownProgress = myGatlingEyeVars.myCooldownDuration;

	myPoisonCloudVars.myCooldownDuration = Random::RandomFloat(myPoisonCloudVars.myCooldownDurationMin, myPoisonCloudVars.myCooldownDurationMax);
	myPoisonCloudVars.myCooldownProgress = myPoisonCloudVars.myCooldownDuration;

	myMortarVars.myCooldownDuration = Random::RandomFloat(myMortarVars.myCooldownDurationMin, myMortarVars.myCooldownDurationMax);
	myMortarVars.myCooldownProgress = myMortarVars.myCooldownDuration;

	mySpikeAttackVars.myCooldownDuration = Random::RandomFloat(mySpikeAttackVars.myCooldownDurationMin, mySpikeAttackVars.myCooldownDurationMax);
	mySpikeAttackVars.myCooldownProgress = mySpikeAttackVars.myCooldownDuration;
}

float LevelBoss::GetCooldownNormalized(const LevelBossState aState)
{
	float progress = 1.0f;
	float duration = 1.0f;

	auto iter = std::find(ourActiveStates.begin(), ourActiveStates.end(), aState);
	if (iter == ourActiveStates.end())
		return 1.0f;

	switch (aState)
	{
	case LevelBossState::GatlingEye:
		progress = myGatlingEyeVars.myCooldownProgress;
		duration = myGatlingEyeVars.myCooldownDuration;
		break;

	case LevelBossState::PoisonCloud:
		progress = myPoisonCloudVars.myCooldownProgress;
		duration = myPoisonCloudVars.myCooldownDuration;
		break;

	case LevelBossState::Mortar:
		progress = myMortarVars.myCooldownProgress;
		duration = myMortarVars.myCooldownDuration;
		break;

	case LevelBossState::SpikeAttack:
		progress = mySpikeAttackVars.myCooldownProgress;
		duration = mySpikeAttackVars.myCooldownDuration;
		break;

	default:
		break;
	}

	return progress / duration;
}

bool LevelBoss::IsDead() const
{
	return myHasSpawned &&
		myState == +LevelBossState::Death;
}

bool LevelBoss::IsEngaged() const
{
	return myHasSpawned &&
		myState != +LevelBossState::Unengaged &&
		myState != +LevelBossState::Disengaged &&
		myState != +LevelBossState::Death;
}

bool LevelBoss::IsDisengaged() const
{
	return myHasSpawned &&
		myState == +LevelBossState::Disengaged;
}

bool LevelBoss::IsFightStarted() const
{
	return myHasSpawned &&
		myState != +LevelBossState::Unengaged &&
		myState != +LevelBossState::Death;
}

bool LevelBoss::IsAbilityUnlocked(const LevelBossState aState)
{
	auto iter = std::find(ourActiveStates.begin(), ourActiveStates.end(), aState);
	if (iter == ourActiveStates.end())
		return false;

	return true;
}

GameObject* LevelBoss::GetPlayerObject()
{
	return myPlayerObject;
}

Engine::AudioComponent* LevelBoss::GetAudioSource()
{
	return myAudioComponent;
}

Engine::AnimatorComponent* LevelBoss::GetAnimator()
{
	return myAnimator;
}

HealthComponent* LevelBoss::GetHealth()
{
	return myHealth;
}

void LevelBoss::ResetBoss()
{
	ourIsInitialized = false;
}

float LevelBoss::GetHealthMultiplier() const
{
	auto gm = GameManager::GetInstance();
	GameDifficulty diff = GameManager::GetInstance()->GetGameDifficulty();
	return myHealthScalings[diff] * gm->GetGameSettings().myDifficultyMultiplier;
}

float LevelBoss::GetDmgMultiplier() const
{
	auto gm = GameManager::GetInstance();
	GameDifficulty diff = gm->GetGameDifficulty();
	return myDmgScalings[diff] * gm->GetGameSettings().myDifficultyMultiplier;
}

void LevelBoss::SetShowEye(const bool aState)
{
	if (myEyeMesh && myEyeMesh.Get())
	{
		myEyeMesh.Get()->SetActive(aState);
	}
}

GameObject* LevelBoss::GetEye()
{
	if (myEyeMesh && myEyeMesh.Get())
	{
		if (auto parent = myEyeMesh.Get()->GetTransform().GetParent())
		{
			return parent->GetGameObject();
		}
	}
	return nullptr;
}

void LevelBoss::SetCanEnterNextLevel(const bool aState)
{
	myCanEnterNextLevel = aState;
}

void LevelBoss::SetPortalTransition(const PortalTransition aTransition)
{
	auto& mesh = myPortalModel->GetMeshMaterialInstanceByIndex(0);
	const Vec3f& portal_color = mesh.GetAdditiveColor();
	const float intensity = mesh.GetFloat4("POMVar").x;
	const Vec3f& skybox_color = mySkybox->GetAdditiveColor();

	myPortalStartColor = portal_color;
	myPortalStartVarY = intensity;
	mySkyboxStartColor = skybox_color;
	
	switch (aTransition)
	{
	case LevelBoss::PortalTransition::None:
	{
		myPortalEndColor = PORTAL_COLOR_DEFAULT;
		myPortalEndVarY = PORTAL_INTENSITY_DEFAULT;
		mySkyboxEndColor = SKYBOX_COLOR_DEFAULT;

		myPortalDuration = 0.0f;
	} break;
	case LevelBoss::PortalTransition::Spawn_ToRed:
	{
		myPortalEndColor = PORTAL_COLOR_SPAWN_RED;
		myPortalEndVarY = PORTAL_INTENSITY_SPAWN_RED;
		mySkyboxEndColor = SKYBOX_COLOR_SPAWN_RED;

		myPortalDuration = 0.7f;
	} break;
	case LevelBoss::PortalTransition::Spawn_ToPurple:
	{
		myPortalEndColor = PORTAL_COLOR_SPAWN_PURPLE;
		myPortalEndVarY = PORTAL_INTENSITY_SPAWN_PURPLE;
		mySkyboxEndColor = SKYBOX_COLOR_SPAWN_PURPLE;

		myPortalDuration = 2.0f;
	} break;
	case LevelBoss::PortalTransition::Cleansed_ToBlue:
	{
		myPortalEndColor = PORTAL_COLOR_CLEANSED_BLUE;
		myPortalEndVarY = PORTAL_INTENSITY_CLEANSED_BLUE;
		mySkyboxEndColor = SKYBOX_COLOR_CLEANSED_BLUE;

		myAudioComponent->PlayEvent("PortalCleanse");

		myPortalDuration = 2.0f;
	} break;
	default:
		break;
	}

	myPortalProgress = myPortalDuration;
	myPortalTransition = aTransition;
}

void LevelBoss::InitComponents()
{
	if (!(myInteractable = myGameObject->GetComponent<Interactable>()))
	{
		LOG_ERROR(LogType::Game) << "No interactable component was attached to the level boss!";
		myWasInitialized = false;
		return;
	}

	if (!(myHealth = myGameObject->GetComponent<HealthComponent>()))
	{
		LOG_ERROR(LogType::Game) << "No health component was attached to the level boss!";
		myWasInitialized = false;
		return;
	}

	if (!(myAnimator = myGameObject->GetComponent<Engine::AnimatorComponent>()))
	{
		LOG_ERROR(LogType::Game) << "No animator component was attached to the level boss!";
		myWasInitialized = false;
		return;
	}

	if (!(myModel = myGameObject->GetComponent<Engine::ModelComponent>()))
	{
		LOG_ERROR(LogType::Game) << "No model component was attached to the level boss!";
		myWasInitialized = false;
		return;
	}

	if (!myAnimator->HasMachine())
	{
		LOG_ERROR(LogType::Game) << "Missing animation state machine on animator component!";
		myWasInitialized = false;
		return;
	}

	if (!(myDomeObject.Get()))
	{
		LOG_ERROR(LogType::Game) << "No dome object was assigned on the level boss!";
		myWasInitialized = false;
		return;
	}

	if (!(myDome = myDomeObject.Get()->GetComponent<BossDome>()))
	{
		LOG_ERROR(LogType::Game) << "Missing dome component on dome object!";
		myWasInitialized = false;
		return;
	}

	if (!(myPortalObject.Get()))
	{
		LOG_ERROR(LogType::Game) << "No portal object was assigned on the level boss!";
		myWasInitialized = false;
		return;
	}

	if (!(myPortalModel = myPortalObject.Get()->GetComponent<Engine::ModelComponent>()))
	{
		LOG_ERROR(LogType::Game) << "Missing model component on portal object!";
		myWasInitialized = false;
		return;
	}

	if (!(myPortalSwirlObject.Get()))
	{
		LOG_ERROR(LogType::Game) << "No portal swirl object was assigned on the level boss!";
		myWasInitialized = false;
		return;
	}

	if (!(myPortalSwirlModel = myPortalSwirlObject.Get()->GetComponent<Engine::ModelComponent>()))
	{
		LOG_ERROR(LogType::Game) << "Missing model component on portal swirl object!";
		myWasInitialized = false;
		return;
	}

	if (!(mySkybox = myGameObject->GetSingletonComponent<Engine::SkyboxComponent>()))
	{
		LOG_ERROR(LogType::Game) << "There is no skybox in this scene. Level boss requires it to exist!";
		return;
	}

	if (!(myItemPrefab->IsValid()))
	{
		LOG_ERROR(LogType::Game) << "No item object was assigned on the level boss!";
		myWasInitialized = false;
		return;
	}

	if (!(myPortalVFXObj.IsValid()))
	{
		LOG_ERROR(LogType::Game) << "No portal VFX object was assigned on the level boss!";
		myWasInitialized = false;
		return;
	}
}

void LevelBoss::InitStateMachine()
{
	myStates[LevelBossState::Unengaged] = DBG_NEW LBS::UnengagedState(*this);

	myStates[LevelBossState::Idle] = DBG_NEW LBS::RoamState(*this, myRoamVars);

	myStates[LevelBossState::GatlingEye] = DBG_NEW LBS::GatlingEyeState(*this, myGatlingEyeVars);
	myStates[LevelBossState::PoisonCloud] = DBG_NEW LBS::PoisonCloudState(*this, myPoisonCloudVars);
	myStates[LevelBossState::Mortar] = DBG_NEW LBS::MortarState(*this, myMortarVars);
	myStates[LevelBossState::SpikeAttack] = DBG_NEW LBS::SpikeAttackState(*this, mySpikeAttackVars);

	myStates[LevelBossState::Disengaged] = DBG_NEW LBS::DisengageState(*this, myDisengageVars);

	myStates[LevelBossState::Death] = DBG_NEW LBS::DeathState(*this, mySpikeAttackVars);

	ChangeState(myState);
}

void LevelBoss::InitStartValues()
{
	if (!(myDome->Init(this)))
	{
		LOG_ERROR(LogType::Game) << "Failed to init dome component on dome object!";
		myWasInitialized = false;
		return;
	}

	auto pollingStation = myGameObject->GetSingletonComponent<PollingStationComponent>();

	if(!pollingStation)
	{
		LOG_ERROR(LogType::Game) << "No PollingStation found";
		return;
	}
	myPlayerObject = pollingStation->GetPlayer();

	myHasStartedNewLevel = false;
	myCanEnterNextLevel = false;

	myInteractable->ObserveInteract([&]
		{
			if (myState == +LevelBossState::Unengaged)
			{
				OnBeginFight();
			}

			if (!myHasStartedNewLevel && myState == +LevelBossState::Death)
			{
				UIEventManager::TryToFadeOut([&]()
				{
					auto player = Main::GetPlayer();
					if (auto comp = player->GetComponent<Player>())
					{
						comp->ActivateLoadingScreen();
					}

					myAudioComponent->PlayEvent("EnterPortal");

					Main::SaveProgress();
					GameManager::GetInstance()->ChangeLevel();
					myHasStartedNewLevel = true;
				}, true);
			}
		});

	myHealth->ObserveDeath([&](float anAmount)
		{
			if (myState != +LevelBossState::Death)
			{
				OnDeath();
			}
		});

	myHealth->ObserveHealing([&](float anAmount)
		{
			if (myState == +LevelBossState::Death)
			{
				ChangeState(LevelBossState::Idle);
			}
		});

	myHealth->ObserveDamage([&](float anAmount)
		{
			myAudioComponent->PlayEvent("Hurt");
			myOnHitIndicatorProgress = myOnHitIndicatorDuration;
		});

	myAnimator->GetController().AddStateOnExitCallback("BASE - (Main)", "Spawn", [this]() {
			OnIdleOverrideEvents();
			SetMaximumCooldown();
			myAnimator->GetController().SetBool("HasSpawned", true);
			myGameObject->SetLayer(eLayer::ENEMY | eLayer::ENEMY_DAMAGEABLE | eLayer::IMMOVABLE);
			myHasSpawned = true;
			SetPortalTransition(LevelBoss::PortalTransition::Spawn_ToPurple);
		});

	if (auto camera = myGameObject->GetScene()->GetMainCameraGameObject())
	{
		if (auto shake = camera->GetComponent<Engine::CameraShakeComponent>())
		{
			myAnimator->GetController().AddEventCallback("Screenshake_Spawn", [shake]() { shake->AddPerlinShake("LevelBoss_Spawn"); });
			myAnimator->GetController().AddEventCallback("Screenshake_Death", [shake]() { shake->AddPerlinShake("LevelBoss_Death"); });
			myAnimator->GetController().AddEventCallback("Screenshake_Poison", [shake]() { shake->AddPerlinShake("LevelBoss_PoisonCloud"); });
		}
	}

	myAnimator->GetController().SetLayerWeight("Gatling Is Disabled - (Override)", 0.0f);
	myAnimator->GetController().SetLayerWeight("Mortar + Poison Is Disabled - (Override)", 0.0f);

	uint32_t index = GameManager::GetInstance()->GetLevelChangeCount();
	AddAbility((int)index);

	for (int i = 0; i < GameDifficulty::_size(); i++)
	{
		if (myHealthScalings[i] < 1.0f)
			myHealthScalings[i] = 1.0f;

		if (myDmgScalings[i] < 1.0f)
			myDmgScalings[i] = 1.0f;
	}
}

void LevelBoss::InitStateCache()
{
	if (ourIsInitialized)
		return;

	ourCachedStates.clear();

	ourCachedStates.push_back(LevelBossState::GatlingEye);
	ourCachedStates.push_back(LevelBossState::PoisonCloud);
	ourCachedStates.push_back(LevelBossState::Mortar);
	ourCachedStates.push_back(LevelBossState::SpikeAttack);

	ourActiveStates.clear();

	AddRandomAbility();

	ourIsInitialized = true;
}

void LevelBoss::InitAudio()
{
	myAudioComponent = myGameObject->AddComponent<Engine::AudioComponent>();
	
	myAudioComponent->AddEvent("Death", "event:/SFX/NPC/LEVEL BOSS/Death");
	myAudioComponent->AddEvent("Hurt", "event:/SFX/NPC/LEVEL BOSS/Hurt");

	myAudioComponent->AddEvent("GattlingEyeStart", "event:/SFX/NPC/LEVEL BOSS/GattlingEyeStart");
	myAudioComponent->AddEvent("GattlingEyeLoop", "event:/SFX/NPC/LEVEL BOSS/GattlingEye");
	myAudioComponent->AddEvent("GattlingEyeEnd", "event:/SFX/NPC/LEVEL BOSS/GattlingEyeEnd");

	myAudioComponent->AddEvent("Mortar", "event:/SFX/NPC/LEVEL BOSS/Mortar");
	myAudioComponent->AddEvent("PoisonCloud", "event:/SFX/NPC/LEVEL BOSS/PoisonCloud");

	myAudioComponent->AddEvent("ForceFieldPush", "event:/SFX/NPC/LEVEL BOSS/ForcefieldPush");
	myAudioComponent->AddEvent("EnterPortal", "event:/SFX/INT/EnterPortal");
	myAudioComponent->AddEvent("PortalCleanse", "event:/SFX/INT/PortalCleanse");
}

void LevelBoss::AddAbility(const int aLevelIndex)
{
	while (myLevelIndex < aLevelIndex)
	{
		if (ourCachedStates.empty())
		{
			myLevelIndex = aLevelIndex;
			break;
		}

		AddRandomAbility();
		myLevelIndex++;
	}

	while (myLevelIndex > aLevelIndex)
	{
		if (ourActiveStates.empty())
		{
			myLevelIndex = aLevelIndex;
			break;
		}

		if (myLevelIndex <= ourActiveStates.size())
		{
			LevelBossState state = ourActiveStates.back();
			ourActiveStates.pop_back();
			ourCachedStates.push_back(state);
			if (auto atk = dynamic_cast<LBS::EngageState*>(myStates[state]))
			{
				atk->OnDisableAttack();
			}
		}
		myLevelIndex--;
	}
}

void LevelBoss::AddAbilityDebug(const LevelBossState aState)
{
	for (int i = 0; i < ourCachedStates.size(); i++)
	{
		if (ourCachedStates[i] == aState)
		{
			LevelBossState state = ourCachedStates[i];
			ourCachedStates.erase(ourCachedStates.begin() + i);
			ourActiveStates.push_back(state);
			if (auto atk = dynamic_cast<LBS::EngageState*>(myStates[state]))
			{
				atk->OnEnableAttack();
			}
			break;
		}
	}
}

void LevelBoss::RemoveAbilityDebug(const LevelBossState aState)
{
	for (int i = 0; i < ourActiveStates.size(); i++)
	{
		if (ourActiveStates[i] == aState)
		{
			LevelBossState state = ourActiveStates[i];
			ourActiveStates.erase(ourActiveStates.begin() + i);
			ourCachedStates.push_back(state);
			if (auto atk = dynamic_cast<LBS::EngageState*>(myStates[state]))
			{
				atk->OnDisableAttack();
			}
			break;
		}
	}
}

void LevelBoss::OnIdleOverrideEvents()
{
	for (int i = 0; i < ourActiveStates.size(); i++)
	{
		if (auto atk = dynamic_cast<LBS::EngageState*>(myStates[ourActiveStates[i]]))
		{
			atk->OnEnableAttack();
		}
	}

	for (int i = 0; i < ourCachedStates.size(); i++)
	{
		if (auto atk = dynamic_cast<LBS::EngageState*>(myStates[ourCachedStates[i]]))
		{
			atk->OnDisableAttack();
		}
	}
}

void LevelBoss::AddRandomAbility()
{
	if (ourCachedStates.empty())
		return;

	const int index = Random::RandomInt(0, ourCachedStates.size() - 1);
	ourActiveStates.push_back(ourCachedStates[index]);
	ourCachedStates.erase(ourCachedStates.begin() + index);

	if (auto atk = dynamic_cast<LBS::EngageState*>(myStates[LevelBossState::_from_index(index)]))
	{
		atk->OnEnableAttack();
	}
}

void LevelBoss::OnChangeState(const LevelBossState aState)
{

}

void LevelBoss::OnBeginFight()
{
	myGameDifficulty = GameManager::GetInstance()->GetGameDifficulty();

	myHealth->SetMaxHealthModifier(GetHealthMultiplier());
	myHealth->ResetHealthToMax();

	myAnimator->GetController().Trigger("Spawn");
	myDome->OnBeginFight();

	Main::SetCanOpenChestShield(false);

	{
		//MUSIC
		// ???? failsafea gärna nästa gång det krashade här
		if (auto musicManager = myGameObject->GetSingletonComponent<Engine::MusicManager>())
		{
			myTempLevel = musicManager->GetLevel();
			musicManager->SetLevel(Engine::eMusicLevel::MINIBOSS);
			musicManager->Stop();
			musicManager->Play();
		}
	}

	const int size = mySpikeAttackVars.myBossSpikes.size();
	for (int i = 0; i < size; i++)
	{
		if (auto animator = mySpikeAttackVars.myBossSpikes[i].Get()->GetComponent<Engine::AnimatorComponent>())
		{
			auto& controller = animator->GetController();
			controller.SetBool("HasUnlockedSpikes", IsAbilityUnlocked(LevelBossState::SpikeAttack));
			controller.Trigger("Spawn");
		}
	}

	SetPortalTransition(PortalTransition::Spawn_ToRed);

	KnockPlayer();

	Engage();
}

void LevelBoss::OnDeath()
{
	Main::SetCanOpenChestShield(true);

	myDome->OnDeath();

	myAudioComponent->PlayEvent("Death");

	if (auto musicManager = myGameObject->GetSingletonComponent<Engine::MusicManager>())
	{
		musicManager->SetLevel(Engine::eMusicLevel::_from_integral(myTempLevel));
		musicManager->PlayDelayed(18.0f);
	}

	auto mm = myGameObject->GetSingletonComponent<Engine::MusicManager>();
	if (mm)
	{
		if (auto ac = mm->GetGameObject()->GetComponent<Engine::AudioComponent>())
		{
			ac->ChangeParameter(mm->GetGameObject()->GetUUID(), "event:/ST/TRACK/Miniboss", "End", 1.0f);
		}
	}

	auto ai = myGameObject->GetSingletonComponent<AIDirector>();
	if (ai)
	{
		ai->DisableAllWaveEmitters();
	}

	for (int i = 0; i < myLevelIndex; i++)
	{
		DropLoot();
	}

	myPortalVFXObj.Get()->SetActive(false);

	ChangeState(LevelBossState::Death);
}

void LevelBoss::DropLoot()
{
	eRarity rarity = eRarity::Rare;

	if (!myHasDroppedLegendary)
	{
		if (Random::RandomFloat(0.0f, 1.0f) <= myChanceForLegendary)
		{
			rarity = eRarity::Legendary;
			myHasDroppedLegendary = true;
		}
	}

	Item* item = Main::GetItemManager().GetRandomItem(rarity, std::vector<Item*>());

	//Set Target Position
	auto ps = myGameObject->GetSingletonComponent<PollingStationComponent>();
	Vec3f position = ps->GetPlayerPos();

	position.x += Random::RandomFloat(-250.f, 250.f);
	position.z += Random::RandomFloat(-250.f, 250.f);

	if (auto go = ps->GetPlayer())
	{
		if (auto player = go->GetComponent<Player>())
		{
			auto velocity = player->GetStatus().movementDirection;
			position += velocity;
		}
	}

	auto scene = myGameObject->GetScene();
	RayCastHit hit;
	if (scene->RayCast(position, Vec3f(0, -1, 0), FLT_MAX, eLayer::DEFAULT | eLayer::GROUND, hit))
	{
		position = hit.Position;
	}
	position.y += 150.f;

	//Create Item Choice
	if (myItemPrefab->IsValid())
	{
		Engine::GameObjectPrefab& prefab = myItemPrefab->Get();

		if (auto choice = prefab.Instantiate(*scene).GetComponent<ItemChoice>())
		{
			Vec3f top = GetTransform().GetPosition() + Vec3f(0, 500, 0);

			choice->SetItem(item);
			choice->SetLerpSpeed(0.8f);
			choice->SetInteractRange(choice->GetInteractRange() * 1.5f);
			choice->DestroyOnSelect();
			choice->SetPontusBoolToTrue();
			choice->Open(0, nullptr, top, position);
		}
	}
}

void LevelBoss::KnockPlayer()
{
	if (auto player = Main::GetPlayer())
	{
		if (auto health = player->GetComponent<HealthComponent>())
		{
			Vec3f playerPos = player->GetTransform().GetPosition();
			Vec3f bossPos = myGameObject->GetTransform().GetPosition();
			//playerPos.y = bossPos.y + 125.0f;
			Vec3f direction = (playerPos - bossPos).GetNormalized();
			direction.y = 0.4f;

			//if (auto comp = player->GetComponent<Player>())
			//{
			//	comp->GetStatus().movementDirection = Vec3f(0,0,0);
			//	comp->GetTransform().ResetMovement();
			//}

			health->ApplyKnockback(direction * myShockwaveForce);
			

			if (myShockwaveVFXRef->IsValid())
			{
				if (auto go = myGameObject->GetScene()->AddGameObject<GameObject>())
				{
					go->GetTransform().SetPosition(bossPos);
					if (auto vfx = go->AddComponent<Engine::VFXComponent>(myShockwaveVFXRef->Get()))
					{
						vfx->Play();
						vfx->AutoDestroy();
						myAudioComponent->PlayEvent("ForceFieldPush");
					}
				}
			}
		}
	}
	myShockwaveProgress = myShockwaveDuration;
}

void LevelBoss::UpdatePortal()
{
	if (myPortalProgress > 0.0f)
	{
		myPortalProgress -= Time::DeltaTime;
		if (myPortalProgress <= 0.0f)
		{
			myPortalProgress = 0.0f;
			myPortalTransition = PortalTransition::None;
		}
		
		auto& portal_mesh = myPortalModel->GetMeshMaterialInstanceByIndex(0);
		auto& swirl_mesh = myPortalSwirlModel->GetMeshMaterialInstanceByIndex(0);

		const float ratio = 1.0f - (myPortalProgress / myPortalDuration);
		const Vec3f portal_color = CU::Lerp(myPortalStartColor, myPortalEndColor, ratio);
		portal_mesh.SetAdditiveColor(portal_color);
		swirl_mesh.SetAdditiveColor(portal_color);

		Vec4f var = portal_mesh.GetFloat4("POMVar");
		var.x = CU::Lerp(myPortalStartVarY, myPortalEndVarY, ratio);
		portal_mesh.SetFloat4("POMVar", var);

		const Vec3f skybox_color = CU::Lerp(mySkyboxStartColor, mySkyboxEndColor, ratio);
		if (mySkybox)
		{
			mySkybox->SetAdditiveColor(skybox_color);
		}
	}
}
