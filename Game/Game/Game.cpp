#include "pch.h"
#include "Game.h"

#include <Scenes/MainMenuScene.h>
#include <Scenes/ScriptedEventsTestScene.h>
#include <Scenes/MarkusTestScene.h>
#include "Engine/Engine.h"
#include "Engine/EngineSettings.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/SceneManager.h"
#include "Engine/Scene/SceneRegistry.h"
#include "Scenes/FilipTestScene.h"
#include "Engine/GameObject/Components/VideoComponent.h"


#include "Engine/Scene/LogoScene.h"
#include "Scenes/JesperTestScene.h"
#include "Scenes/JonathanTestScene.h"
#include "Scenes/SimonTestScene.h"
#include "Scenes/ViktorTestScene.h"

#include "Engine/GameObject/Components/2DComponents/ButtonComponent.h"
#include "Engine/GameObject/Components/FollowBoneComponent.h"
#include "Engine/GameObject/Components/MusicManager.h"
#include "Game/Prototype/HomingProjectile.h"
#include "Game/Components/HealthComponentProxy.h"

#include "Camera/CameraController.h"

#include "Physics/Physics.h"
#include "Physics/PhysicsMaterialManager.h"
#include "Engine/GameObject/Components/InfoRigidBodyComponent.h"

#include "Game/Components/HealthComponent.h"

#include "MainSingleton/MainSingleton.h"

#include "Engine/GameObject/SingletonComponent.h"
#include "Engine/Navmesh/NavmeshComponent.h"

#include <future>
// #include <Engine/GameObject/Components/AudioComponent.h>
#include <Engine/GameObject/Components/InfoRigidStaticComponent.h>

#include "Components/DestructibleComponent.h"
#include <Engine/GameObject/Components/2DComponents/SliderComponent.h>

#include "Engine/GameObject/Components/PhysicsObjectEmitterComponent.h"
#include <Engine/GameObject/Components/TriggerComponent.h>
#include <Game/Components/PortalComponent.h>
#include <Game/Components/PortalAltarComponent.h>

#include "VFXEditorScene.h"

#include "Engine/GameObject/Components/VFXSpawnerComponent.h"
#include "Engine/GameObject/Components/ReflectionCaptureComponent.h"

#include "Engine\Renderer/Shaders\ShaderLoader.h"
#include "Engine\Engine.h"
#include "Engine\Renderer\GraphicsEngine.h"
#include "Engine\Renderer\Directx11Framework.h"

#include <D3DCompiler.h>

#include <d3d11.h>

#include "Game/UI/MainMenu/AudioHandler.h"

#include "AI/AIDirector/AIDirector.h"
#include "AI/Enemies/Base/EnemyScalableStats.h"
#include "AI/Enemies/Prototype/BulletEnemy.h"
#include "AI/Enemies/SuicideRunner/Runner.h"
#include "AI/Enemies/GroundTankLad/GroundTankLad.h"
#include "AI/Enemies/Slime/Slime.h"
#include "Game/AI/Enemies/Popcorn/FlyPopcorn.h"
#include "Game/AI/Enemies/GroundPopcorn/GroundPopcorn.h"
#include "Game/AI/Enemies/Mortar/Mortar.h"
#include "Game/AI/Enemies/BurstTank/BurstTank.h"
#include "Game/AI/VFX/EnemyVFXRef.h"
#include "Game/AI/Portal/EnemySpawnPortal.h"
#include "Game/AI/Portal/EnemyspawnPortalFly.h"
#include "Game/AI/Explosion/GeneralExplosion.h"
#include "Game/AI/FinalBoss/HandHomingProjectile.h"
#include "Game/AI/FinalBoss/HandHomingProjectileLauncher.h"
#include "Game/AI/FinalBoss/HandGatlingProjectile.h"
#include "Game/AI/FinalBoss/FinalBossHealthbar.h"
#include "Game/AI/FinalBoss/FinalBossPortal.h"
#include "Game/AI/FinalBoss/FinalBossBody.h"
#include "Game/AI/FinalBoss/FinalBossLightning.h"
#include "Game/AI/FinalBoss/EmittedShockwave.h"
#include "Game/AI/Enemies/Projectiles/Projectile.h"
#include "AI/PollingStation/PollingStationComponent.h"
#include "Prototype/RorPlayer.h"
#include "AI/LevelBoss/LevelBoss.h"
#include "AI/LevelBoss/Objects/BossSpikeExtension.h"
#include "AI/FinalBoss/FinalBoss.h"
#include "ScriptedEvent.h"
#include "CinematicCameraComponent.h"
#include "AI/FinalBoss/RightProjectileHand.h"
#include "AI/FinalBoss/LeftPunchHand.h"
#include "Components/Interactable.h"
#include "Game/Components/ExplosiveObjectComponent.h"
#include "Components\WalletComponent.h"
#include "Components\MoneyDropComponent.h"
#include "Player/Player.h"
#include "Components\RespawnComponent.h"
#include "Interactables\GamlingStatue.h"
#include "Interactables\BeamComponent.h"
#include "Components/PlayerShield.h"

#include "HUD/HUDHandler.h"
#include "HUD/HUDAbility.h"
#include "HUD/HUDHealth.h"
#include "HUD/HUDItemDescription.h"
#include "HUD/HUDInteraction.h"
#include "HUD/HUDDifficulty.h"
#include "HUD/HUDDamageVignette.h"
#include "UI/PauseMenuComponent.h"
#include "UI/WinScreen.h"

#include "Game/Components/Item/ChestComponent.h"
#include "Game/World/LaunchpadComponent.h"
#include "Game/Components/Item/ItemChoice.h"
#include "Game/Components/HUD/HealthBarComponent.h"
#include "Game/Components/Item/ChestSpawnerComponent.h"
#include "Game/Components/ExplosionComponent.h"

#include "GameManager/GameManager.h"
#include "Game/Components/GameManagerUpdater.h"
#include "Game/World/LevelBossSpawnerComponent.h"

#include "Engine/GameObject/Components/LandscapeComponent.h"
#include "Components/DayNightComponent.h"

#include "Game/UI/MainMenu/MenuHandler.h"
#include "Game/UI/MainMenu/VideoHandler.h"
#include "Game/UI/MainMenu/MainMenu.h"
#include "Game/UI/MainMenu/LevelSelect.h"
#include "Game/UI/MainMenu/Options.h"
#include "Game/UI/MainMenu/GeneralOptions.h"
#include "Game/UI/MainMenu/VideoOptions.h"
#include "Game/UI/MainMenu/AudioOptions.h"
#include "Game/UI/MainMenu/Credits.h"
#include "Game/UI/MainMenu/LoadingScreen.h"
#include "Game/UI/MainMenu/HowToPlay.h"
#include "Game/UI/MainMenu/ControlsHowToPlay.h"
#include "Game/UI/MainMenu/ItemsHowToPlay.h"
#include "Game/UI/MainMenu/AbilitiesHowToPlay.h"
#include "Game/UI/MainMenu/LoadingBar.h"
#include "Game/UI/MainMenu/Play.h"
#include "Game/UI/MainMenu/Leaderboard.h"

#include "Game/UI/UI_Fade.h"

#include "Components/VFXSpawner.h"

#include "Components/DestroyAfterSecondsComponent.h"
#include "LookAt.h"
#include "AI/FinalBoss/FinalBossFollowGameObject.h"
#include "AI/FinalBoss/Behaviours/Body/PoisonCloudDamageComponent.h"
#include "AI/FinalBoss/FinalBossShootFlash.h"
#include "AI/FinalBoss/FinalBossArenaTrigger.h"

Game::Game()
{
}

Game::~Game()
{
	// Remove the references we we can destroy the
	// main menu before ResourceManager gets destroyed
	// Must be in this order to be destroyed properly
	// myGlobalResourceReferences = nullptr;
	Main::Destroy();
	myEngine = nullptr;
}

bool Game::Init()
{
	EngineSettings engineSettings;
	{
		engineSettings.WindowTitle = "Starfall";
		engineSettings.myTargetWindowSize = { 1920, 1080 };
		engineSettings.myCursorHandle = LoadCursorFromFile(L"Assets/Cursor.cur");

		RECT windowSize{};
		SystemParametersInfoA(SPI_GETWORKAREA, 0, &windowSize, 0);
		engineSettings.WindowSize = CU::Vector2ui(windowSize.right, windowSize.bottom);

#ifdef _RETAIL
		engineSettings.myEnableFilewatcher = false;

		// engineSettings.myWindowState = Engine::WindowState::WindowedBorderless;
		engineSettings.myWindowState = Engine::WindowState::Windowed;
#else
		engineSettings.myEnableFilewatcher = true;
#endif

#ifndef _RETAIL
		engineSettings.myResourceManagerSettings.UsePrecompiledShaders = false;
#else
		//engineSettings.myResourceManagerSettings.UsePrecompiledShaders = true;
		engineSettings.myResourceManagerSettings.UsePrecompiledShaders = true;
#endif
	}

	myEngine = MakeOwned<Engine::Engine>();
	Engine::Engine::SetInstance(*myEngine);

	RegisterSingletonComponents();

	//MUST REGISTER COMPONENTS BEFORE ENGINE INIT TO SHOW IN NODE EDITOR
	RegisterComponents();

	if (!myEngine->Init(engineSettings))
	{
		return false;
	}

	RegisterPhysicsMaterials();

	RegisterScenes();

	bool gameMode = false;
	if (gameMode)
	{
		myEngine->GetSceneManager().LoadSceneAsync("Assets\\Scenes\\MainMenu");
	}
	else
	{
		GetEngine().StartEditor();
	}

	Main::Start();
	GameManager::GetInstance()->Start();
	GameManager::GetInstance()->Init();

	return true;
}

void Game::Update()
{
	ZoneScopedN("Game::Update");

	myEngine->Update();
	Main::Update();
}

void Game::RegisterSingletonComponents()
{
	// DO NOT USE THIS ANYMORE!
	// WE REGISTER COMPONENTS IN A NEW WAY, ASK FILIP

	// DONT USE ASK FILIP
	// DONT USE ASK FILIP

	// DONT USE ASK FILIP
	// DONT USE ASK FILIP
	// DONT USE ASK FILIP
	// DONT USE ASK FILIP
}

void Game::RegisterComponents()
{
	// DO NOT USE THIS ANYMORE!
	// WE REGISTER COMPONENTS IN A NEW WAY, ASK FILIP

	// DONT USE ASK FILIP
	// DONT USE ASK FILIP

	// DONT USE ASK FILIP
	// DONT USE ASK FILIP
	// DONT USE ASK FILIP
	// DONT USE ASK FILIP
}

void Game::RegisterPhysicsMaterials()
{
	PhysicsMaterialManager::RegisterMaterial(ePhysicsMaterial::Default, .5f, .5f, .5f);
	PhysicsMaterialManager::RegisterMaterial(ePhysicsMaterial::GRENADE, 51.0f, 50.0f, 0.7f);
}

void Game::RegisterScenes()
{
	Engine::SceneRegistry::RegisterType<FilipTestScene>("FilipTestScene");
	Engine::SceneRegistry::RegisterType<MarkusTestScene>("MarkusTestScene");
	Engine::SceneRegistry::RegisterType<JesperTestScene>("JesperTestScene");
	Engine::SceneRegistry::RegisterType<ViktorTestScene>("ViktorTestScene");
	Engine::SceneRegistry::RegisterType<SimonTestScene>("SimonTestScene");
	Engine::SceneRegistry::RegisterType<MainMenuScene>("MainMenuScene");
	Engine::SceneRegistry::RegisterType<ScriptedEventsTestScene>("ScriptedEventsTestScene");
	Engine::SceneRegistry::RegisterType<JonathanTestScene>("JonathanTestScene");
	Engine::SceneRegistry::RegisterType<Engine::FolderScene>("FolderScene");
	Engine::SceneRegistry::RegisterType<Engine::VFXEditorScene>("VFX Editor");
}
