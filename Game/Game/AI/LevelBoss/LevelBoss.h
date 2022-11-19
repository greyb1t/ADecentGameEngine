#pragma once

#include "Engine/GameObject/Components/Component.h"
#include "Engine/GameObject/Prefab/GameObjectPrefab.h"
#include "Engine/GameObject/Components/MusicManager.h"

#include "State/StateVars.h"

static const Vec3f SKYBOX_COLOR_DEFAULT = Vec3f(0.0f, 0.0f, 0.0f);
static const Vec3f SKYBOX_COLOR_SPAWN_RED = Vec3f(1.0f, 0.0f, 0.0f);
static const Vec3f SKYBOX_COLOR_SPAWN_PURPLE = Vec3f(1.0f, 0.0f, 0.0f);
static const Vec3f SKYBOX_COLOR_CLEANSED_BLUE = SKYBOX_COLOR_DEFAULT;

static const Vec3f PORTAL_COLOR_DEFAULT = Vec3f(1.0f, 1.0f, 1.0f);
static const Vec3f PORTAL_COLOR_SPAWN_RED = Vec3f(1.0f, 0.0f, 0.0f);
static const Vec3f PORTAL_COLOR_SPAWN_PURPLE = Vec3f(0.949f, 0.335f, 0.335f);
static const Vec3f PORTAL_COLOR_CLEANSED_BLUE = Vec3f(0.185f, 0.383f, 0.486f);

static constexpr float PORTAL_INTENSITY_DEFAULT = 40.0f;
static constexpr float PORTAL_INTENSITY_SPAWN_RED = 120.0f;
static constexpr float PORTAL_INTENSITY_SPAWN_PURPLE = PORTAL_INTENSITY_DEFAULT;
static constexpr float PORTAL_INTENSITY_CLEANSED_BLUE = PORTAL_INTENSITY_DEFAULT;

namespace LevelBossStates
{
	class BaseState;
}
namespace LBS = LevelBossStates;

class Interactable;
class HealthComponent;
class BossDome;

namespace Engine
{
	class SkyboxComponent;
}

class LevelBoss : public Component
{
	COMPONENT(LevelBoss, "Level Boss");
public:
	enum class PortalTransition
	{
		None,

		Spawn_ToRed,
		Spawn_ToPurple,

		Cleansed_ToBlue
	};

	LevelBoss() = default;
	LevelBoss(GameObject* aParentObject);
	~LevelBoss();

	void Reflect(Engine::Reflector& aReflector) override;

	void Awake() override;
	void Start() override;

	void Execute(Engine::eEngineOrder aOrder) override;
	void Render() override;

	void Engage();
	void Disengage();

	void ChangeState(const LevelBossState aState);
	const LevelBossState& GetCurrentState() const;

	void SetMaximumCooldown();
	float GetCooldownNormalized(const LevelBossState aState);

	bool IsDead() const;
	bool IsEngaged() const;
	bool IsDisengaged() const;
	bool IsFightStarted() const;
	bool IsAbilityUnlocked(const LevelBossState aState);

	GameObject* GetPlayerObject();

	Engine::AudioComponent* GetAudioSource();
	Engine::AnimatorComponent* GetAnimator();
	HealthComponent* GetHealth();

	static void ResetBoss();

	float GetHealthMultiplier() const;
	float GetDmgMultiplier() const;

	void SetShowEye(const bool aState);
	GameObject* GetEye();

	void SetCanEnterNextLevel(const bool aState);

	void SetPortalTransition(const PortalTransition aTransition);

private:
	void InitComponents();
	void InitStateMachine();
	void InitStartValues();
	void InitStateCache();
	void InitAudio();

	void AddAbility(const int aLevelIndex);
	void AddRandomAbility();

	void AddAbilityDebug(const LevelBossState aState);
	void RemoveAbilityDebug(const LevelBossState aState);

	void OnIdleOverrideEvents();

	void OnChangeState(const LevelBossState aState);
	
	void OnBeginFight();
	void OnDeath();

	void DropLoot();

	void KnockPlayer();
	void UpdatePortal();

private:
	inline static bool ourIsInitialized = false;
	inline static std::vector<LevelBossState> ourCachedStates;
	inline static std::vector<LevelBossState> ourActiveStates;

	inline static std::vector<std::string> ourLevels{};

	bool myHasSpawned = false;
	bool myHasStartedNewLevel = false;

	float myOnHitIndicatorProgress = 0.0f;
	float myOnHitIndicatorDuration = 0.1f;

	float myShockwaveRange = 1500.f;
	float myShockwaveForce = 1600.f;
	VFXRef myShockwaveVFXRef;

	float myShockwaveProgress = 0.0f;
	float myShockwaveDuration = 1.0f;
	int myTempLevel;

	float myChanceForLegendary = 0.75f;
	bool myHasDroppedLegendary = false;

	inline static int myLevelIndex = 1;
	std::map<LevelBossState, LBS::BaseState*> myStates;

	LevelBossState myState = LevelBossState::Unengaged;
	LBS::BaseState* myCurrentState = nullptr;

	GameObject* myPlayerObject = nullptr;
	Interactable* myInteractable = nullptr;
	HealthComponent* myHealth = nullptr;
	Engine::AnimatorComponent* myAnimator = nullptr;
	Engine::ModelComponent* myModel = nullptr;
	Engine::ModelComponent* myPortalModel = nullptr;
	Engine::ModelComponent* myPortalSwirlModel = nullptr;
	Engine::SkyboxComponent* mySkybox = nullptr;
	Engine::AudioComponent* myAudioComponent = nullptr;

	LBS::RoamVars myRoamVars;
	LBS::DisengageVars myDisengageVars;
	LBS::SpikeAttackVars mySpikeAttackVars;
	LBS::PoisonCloudVars myPoisonCloudVars;
	LBS::MortarVars myMortarVars;
	LBS::GatlingEyeVars myGatlingEyeVars;

	GameObjectPrefabRef myItemPrefab;
	Engine::GameObjectRef myDomeObject;
	Engine::GameObjectRef myPortalObject;
	Engine::GameObjectRef myPortalVFXObj;
	Engine::GameObjectRef myPortalSwirlObject;
	Engine::GameObjectRef myHealthBarChild;
	Engine::GameObjectRef myEyeMesh;
	BossDome* myDome = nullptr;

	bool myWasInitialized = true;
	bool myCanEnterNextLevel = false;

	bool myDisplayScaling = false;
	std::array<float, GameDifficulty::_size()> myHealthScalings{};
	std::array<float, GameDifficulty::_size()> myDmgScalings{};

	PortalTransition myPortalTransition = PortalTransition::None;
	float myPortalProgress = 0.0f;
	float myPortalDuration = 1.0f;

	Vec3f myPortalStartColor = {};
	Vec3f myPortalEndColor = {};

	Vec3f mySkyboxStartColor = {};
	Vec3f mySkyboxEndColor = {};

	float myPortalStartVarY = 0.0f;
	float myPortalEndVarY = 40.0f;

	GameDifficulty myGameDifficulty = GameDifficulty::Beginner;
};

