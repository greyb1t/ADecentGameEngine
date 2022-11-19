#include "pch.h"
#include "FinalBoss.h"
#include "Engine/GameObject/GameObject.h"
#include "Components/HealthComponent.h"
#include "Engine/Reflection/Reflector.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "FinalBossBody.h"
#include "LeftPunchHand.h"
#include "RightProjectileHand.h"
#include "Constants.h"
#include "Engine/GameObject/Components/CameraShakeComponent.h"
#include "Engine/Engine.h"
#include "Engine/CameraShake/CameraShakeManager.h"
#include "Engine/GameObject/RecursiveGameObjectChildIterator.h"
#include "AI/PollingStation/PollingStationComponent.h"
#include "Engine/GameObject/Components/MusicManager.h"
#include "Engine/CameraShake/KickShake.h"
#include "Engine/GameObject/Components/SkyboxComponent.h"
#include "Engine/GameObject/Components/PostProcessComponent.h"
#include "Engine/GameObject/Components/EnvironmentLightComponent.h"
#include "Engine/CameraShake/Displacement.h"
#include "FinalBossLightning.h"
#include "AI/AIDirector/AIDirector.h"
#include "Engine/GameObject/Components/AudioComponent.h"
#include "GameManager/GameManager.h"
#include "FinalBossPortal.h"
#include "Engine/ResourceManagement/Resources/AnimationCurveResource.h"
#include "Engine/AnimationCurve/Curve.h"
#include "Engine/GameObject/Components/BezierSplineComponent.h"
#include "Engine/GameObject/Components/MusicManager.h"

FB::FinalBoss::FinalBoss(GameObject* aGameObject)
	: Component(aGameObject)
{

}

FB::FinalBoss::~FinalBoss()
{
}

FB::FinalBoss::FinalBoss()
{
}

void FB::FinalBoss::Start()
{
	if (!myBodyGameObject)
	{
		LOG_ERROR(LogType::Game) << "Final boss is missing body gameobject";
		myGameObject->Destroy();
		return;
	}

	if (!myLeftHandGameObject)
	{
		LOG_ERROR(LogType::Game) << "Final boss is missing left hand gameobject";
		myGameObject->Destroy();
		return;
	}

	if (!myRightHandGameObject)
	{
		LOG_ERROR(LogType::Game) << "Final boss is missing right hand gameobject";
		myGameObject->Destroy();
		return;
	}

	if (!myFinalBossPortalRef)
	{
		LOG_ERROR(LogType::Game) << "Final boss is missing portal gameobject";
		myGameObject->Destroy();
		return;
	}

	if (myWaypointsGameObject)
	{
		for (auto it = Engine::GameObjectChildIterator(*myWaypointsGameObject.Get()); it != nullptr; it++)
		{
			FinalBossWaypoint waypoint(it.DerefAsRaw()->GetTransform().GetPosition());
			myWaypoints.push_back(waypoint);
		}
	}
	else
	{
		LOG_ERROR(LogType::Game) << "Final boss is missing waypoints gameobject";
	}

	myBody = myBodyGameObject.Get()->GetComponent<FinalBossBody>();
	myLeftHand = myLeftHandGameObject.Get()->GetComponent<LeftPunchHand>();
	myRightHand = myRightHandGameObject.Get()->GetComponent<RightProjectileHand>();

	myPortal = myFinalBossPortalRef.Get()->GetComponent<FinalBossPortal>();

	if (!myPortal)
	{
		LOG_ERROR(LogType::Game) << "Missing boss portal in final boss";
	}

	RegisterScriptedEventBridgeEvents();

	UIEventManager::SetObjective("Find the Final Boss");

	if (myFlow)
	{
		// Disable in the beginning, then enable on the intro
		ToggleLimbs(false);
	}

	// Init with the default values to avoid issues with lerping to default values
	myInitialThemeJuice = GetActiveThemeJuice();
	myCurrentThemeJuice = GetActiveThemeJuice();
	myTargetThemeJuice = GetActiveThemeJuice();
}

void FB::FinalBoss::Execute(Engine::eEngineOrder aOrder)
{
	UpdateExposure();
	UpdateThemeJuice();

	UpdateGroundPosition();

	// if (!myHasStartedIntro)
	// {
	// 	myHasStartedIntro = true;
	// 
	// 	StartIntro();
	// }

	if (!ShouldStartBoss())
	{
		return;
	}

	if (IsDead())
	{
		return;
	}

	if (Vec3f::DistanceSq(GetTransform().GetPosition(), GetPlayerPos()) >
		(myMaxPlayerDistanceFromCenterArena * myMaxPlayerDistanceFromCenterArena))
	{
		const auto dir = (GetPlayerPos() - GetTransform().GetPosition()).GetNormalized();
		// const float offsetInwards = 0.f;
		// GetPlayer()->GetTransform().SetPosition(dir * myMaxPlayerDistanceFromCenterArena * -offsetInwards);
		if (auto health = GetPlayer()->GetComponent<HealthComponent>())
		{
			health->ApplyKnockback(dir * -1000.f);
		}
	}

	switch (myPhase)
	{
	case FinalBossPhase::Introduction:
		if (myLeftHand->IsIntroFinished() &&
			myRightHand->IsIntroFinished() &&
			myBody->IsIntroFinished())
		{
			if (myFlow)
			{
				ChangePhase(FinalBossPhase::Phase1);
			}
		}
		break;
	case FinalBossPhase::Phase1:
		break;
	case FinalBossPhase::Phase2:
		break;
	default:
		break;
	}

	const bool enterPhase2 =
		myLeftHand->IsDead() &&
		myRightHand->IsDead() &&
		myPhase != +FinalBossPhase::Phase2;

	if (enterPhase2)
	{
		if (myFlow)
		{
			ChangePhase(FinalBossPhase::Phase2);
		}
	}

	if (myBody->IsDead())
	{
		if (myFlow)
		{
			ChangePhase(FinalBossPhase::Dead);
		}
	}
}

void FB::FinalBoss::Render()
{
	for (const auto& waypoint : myWaypoints)
	{
		if (waypoint.IsTaken())
		{
			GDebugDrawer->DrawSphere3D(DebugDrawFlags::Gameplay, waypoint.GetPosition(), 200.f, 0.f, Vec4f(1.f, 0.f, 0.f, 1.f));
		}
		else
		{
			GDebugDrawer->DrawSphere3D(DebugDrawFlags::Gameplay, waypoint.GetPosition(), 200.f);
		}
	}
}

void FB::FinalBoss::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myFlow, "Enable Flow");

	if (aReflector.ReflectEnum(myPhase, "Phase") & Engine::ReflectorResult_Changed)
	{
		ChangePhase(myPhase);
	}

	aReflector.Reflect(myBodyGameObject, "Body");
	aReflector.Reflect(myLeftHandGameObject, "Left Hand");
	aReflector.Reflect(myRightHandGameObject, "Right Hand");
	aReflector.Reflect(myFinalBossPortalRef, "FinalBoss Portal");
	aReflector.Reflect(myBrokenBridgeGameObject, "Broken Bridge");
	aReflector.Reflect(myWaypointsGameObject, "Waypoints");
	aReflector.Reflect(myLeftHandIntroSplineRef, "Left Hand Intro Spline");

	aReflector.Reflect(myCreditsVideoPrefab, "Credits Video");
	aReflector.Reflect(myWhiteSpritePrefab, "White Sprite Prefab");

	aReflector.Reflect(myPillars, "Pillars");
	aReflector.Reflect(myLightningGameObject, "Lightning GameObject");
	aReflector.Reflect(myAfterBridgeBreak, "After Bridge Break");
	aReflector.Reflect(myEnteringPhase2, "Entering Phase2 Theme");
	aReflector.Reflect(myExposureCurveRef, "Exposure Curve");
	aReflector.Reflect(myExposurePulseDuration, "Exposure Pulse Duration");
	aReflector.Reflect(myTimeUntilFadeToCredits, "Time Until Fade To Credits");

	aReflector.Header("Health Scale Factors");

	for (int i = 0; i < GameDifficulty::_size(); ++i)
	{
		aReflector.Reflect(myHealthScaleFactors[i], std::string(GameDifficulty::_from_integral(i)._to_string()));
		myHealthScaleFactors[i] = std::max(myHealthScaleFactors[i], 1.f);
	}
}

FB::RightProjectileHand& FB::FinalBoss::GetRightHand()
{
	return *myRightHand;
}

FB::LeftPunchHand& FB::FinalBoss::GetLeftHand()
{
	return *myLeftHand;
}

FB::FinalBossBody& FB::FinalBoss::GetBody()
{
	return *myBody;
}

FB::FinalBossPortal* FB::FinalBoss::GetPortal()
{
	return myPortal;
}

const Vec3f& FB::FinalBoss::GetGroundPosition() const
{
	return myGroundPosition;
}

FB::FinalBossPhase FB::FinalBoss::GetPhase() const
{
	return myPhase;
}

bool FB::FinalBoss::IsDead() const
{
	return myPhase == +FinalBossPhase::Dead;
}

void FB::FinalBoss::StartBoss()
{
	UIEventManager::SetObjective("Kill the Final Boss");
	Main::SetCanOpenChestShield(false);

	myShouldStartBoss = true;

	EnableEnemiesSpawning();

	// myDifficultyMultiplier

	const float difficultyFactor = GameManager::GetInstance()->GetGameSettings().myDifficultyMultiplier;

	const float bodyBaseHealth = myBody->GetHealth().GetMaxHealth();
	myBody->GetHealth().SetMaxHealth(bodyBaseHealth * GetHealthScaleFactor() * difficultyFactor);

	const float leftHandBaseHealth = myLeftHand->GetHealth().GetMaxHealth();
	myLeftHand->GetHealth().SetMaxHealth(leftHandBaseHealth * GetHealthScaleFactor() * difficultyFactor);

	const float rightHandBaseHealth = myRightHand->GetHealth().GetMaxHealth();
	myRightHand->GetHealth().SetMaxHealth(rightHandBaseHealth * GetHealthScaleFactor() * difficultyFactor);

	LOG_INFO(LogType::Game) << "Body Health: " << myBody->GetHealth().GetMaxHealth();
	LOG_INFO(LogType::Game) << "LeftHand Health: " << myLeftHand->GetHealth().GetMaxHealth();
	LOG_INFO(LogType::Game) << "RightHand Health: " << myRightHand->GetHealth().GetMaxHealth();

	/*
	if (auto musicManager = myGameObject->GetSingletonComponent<Engine::MusicManager>())
	{
		musicManager->ActivateFinalBoss();
	}
	else
	{
		LOG_ERROR(LogType::Game) << "Missing music manager";
	}
	*/
}

void FB::FinalBoss::StopEnemiesSpawning()
{
	if (auto director = myGameObject->GetSingletonComponent<AIDirector>())
	{
		director->DisableAllWaveEmitters();
		director->SetShouldDespawnAll(true);
	}
}

void FB::FinalBoss::EnableEnemiesSpawning()
{
	if (auto director = myGameObject->GetSingletonComponent<AIDirector>())
	{
		// Do not enable this as I will use my own spawn waves emitters
		// director->DisableAllWaveEmitters();
		director->SetShouldDespawnAll(false);
		director->SetIsAtFinalBoss(true);
	}
}

FB::FinalBossWaypoint* FB::FinalBoss::FindFreeWaypointClosestToPosition(
	const Vec3f& aPosition,
	FinalBossWaypoint* aCurrentWaypoint)
{
	std::vector<FB::FinalBossWaypoint*> waypoints;

	for (auto& waypoint : myWaypoints)
	{
		waypoints.push_back(&waypoint);
	}

	std::sort(waypoints.begin(),
		waypoints.end(),
		[&aPosition](const FB::FinalBossWaypoint* aLeft, const FB::FinalBossWaypoint* aRight)
		{
			return Vec3f::DistanceSq(aLeft->GetPosition(), aPosition) <
				Vec3f::DistanceSq(aRight->GetPosition(), aPosition);
		});

	for (auto waypoint : waypoints)
	{
		const bool takenBySomeoneElse = waypoint->IsTaken() && aCurrentWaypoint != waypoint;

		if (!takenBySomeoneElse)
		{
			return waypoint;
		}
	}

	return nullptr;
}

GameObject* FB::FinalBoss::GetPlayer() const
{
	auto polling = myGameObject->GetSingletonComponent<PollingStationComponent>();
	return polling->GetPlayer();
}

Vec3f FB::FinalBoss::GetPlayerPos() const
{
	auto polling = myGameObject->GetSingletonComponent<PollingStationComponent>();
	const auto playerPos = polling->GetPlayerPos();
	return playerPos;
}

const GameObjectPrefabRef& FB::FinalBoss::GetCreditsVideoPrefab() const
{
	return myCreditsVideoPrefab;
}

const GameObjectPrefabRef& FB::FinalBoss::GetWhiteSpritePrefab() const
{
	return myWhiteSpritePrefab;
}

float FB::FinalBoss::GetHealthScaleFactor() const
{
	return myHealthScaleFactors[GameManager::GetInstance()->GetGameDifficulty()];
}

void FB::FinalBoss::ChangePhase(const FinalBossPhase aPhase)
{
	OnEnterPhase(aPhase);

	myPhase = aPhase;
}

void FB::FinalBoss::OnEnterPhase(const FinalBossPhase aPhase)
{
	if (myRightHand)
	{
		myRightHand->OnEnterPhase(aPhase);
	}
	if (myLeftHand)
	{
		myLeftHand->OnEnterPhase(aPhase);
	}
	if (myBody)
	{
		myBody->EnterPhase(aPhase);
	}

	switch (aPhase)
	{
	case FinalBossPhase::Phase1:
		break;
	case FinalBossPhase::Phase2:
	{
		SetTargetThemeJuice(myEnteringPhase2, 1.f);

		if (auto mm = myGameObject->GetSingletonComponent<Engine::MusicManager>())
		{
			mm->SetIndividualParameter(mm->GetGameObject()->GetUUID(), "FinalBoss", "parameter:/Phase2", 1.f);
			LOG_INFO(LogType::Game) << "Boss died, setting music paramter 1";
		}
		else
		{
			LOG_ERROR(LogType::Game) << "missing music manager in final boss";
		}
	} break;
	case FinalBossPhase::Dead:
	{
		if (auto mm = myGameObject->GetSingletonComponent<Engine::MusicManager>())
		{
			mm->SetIndividualParameter(mm->GetGameObject()->GetUUID(), "FinalBoss", "parameter:/End", 1.f);
			LOG_INFO(LogType::Game) << "Boss died, setting music paramter 2";
		}
		else
		{
			LOG_ERROR(LogType::Game) << "missing music manager in final boss";
		}
	} break;
	default:
		break;
	}
}

bool FB::FinalBoss::ShouldStartBoss() const
{
	return myShouldStartBoss;
}

void FB::FinalBoss::UpdateGroundPosition()
{
	RayCastHit hit;

	if (myGameObject->GetScene()->RayCast(
		GetTransform().GetPosition(),
		Vec3f(0.f, -1.f, 0.f),
		Constants::RaycastDistance,
		/*eLayer::NAVMESH*/eLayer::GROUND | eLayer::DEFAULT,
		hit))
	{
		myGroundPosition = hit.Position;
	}
	else
	{
		LOG_ERROR(LogType::Game) << "Finalboss raycast navmesh ground failed";
	}
}

void FB::FinalBoss::ToggleLimbs(const bool aEnabled)
{
	myRightHand->GetModelGameObject()->SetActive(aEnabled);
	myLeftHand->GetModelGameObject()->SetActive(aEnabled);
	myBody->GetModelGameObject()->SetActive(aEnabled);
}

void FB::FinalBoss::BreakBridge(Engine::AudioComponent& aAudioComponent)
{
	if (myHasBrokenBridge)
	{
		return;
	}

	myHasBrokenBridge = true;

	if (myBrokenBridgeGameObject && myBrokenBridgeGameObject.IsValid())
	{
		if (auto animator = myBrokenBridgeGameObject.Get()->GetComponent<Engine::AnimatorComponent>())
		{
			animator->GetController().Trigger("Death");
		}
	}
	else
	{
		LOG_ERROR(LogType::Game) << "Final boss missing Broken Bridge reference";
	}

	// Start the boss music
	if (auto musicManager = myGameObject->GetSingletonComponent<Engine::MusicManager>())
	{
		//musicManager->Play();
		musicManager->ActivateFinalBoss();
	}
	else
	{
		LOG_ERROR(LogType::Game) << "cannot find MusicManager";
	}
}

void FB::FinalBoss::RegisterScriptedEventBridgeEvents()
{
	if (myBrokenBridgeGameObject && myBrokenBridgeGameObject.IsValid())
	{
		if (auto animator = myBrokenBridgeGameObject.Get()->GetComponent<Engine::AnimatorComponent>())
		{
			animator->GetController().AddEventCallback("ShakeCamera",
				[this]()
				{
					auto mainCam = myGameObject->GetScene()->GetMainCameraGameObject();
					auto shakeComponent = mainCam->GetComponent<Engine::CameraShakeComponent>();
					if (shakeComponent)
					{
						shakeComponent->AddPerlinShake("FinalBoss_Destroy_Bridge");
						shakeComponent->AddKickShake("FinalBoss_Destroy_Bridge", Vec3f(-1.f, 0.f, 0.f));
					}

					// CHANGE THE SKY AND ENVIRONMENT
					// 
					if (auto skybox = myGameObject->GetSingletonComponent<Engine::SkyboxComponent>())
					{
						skybox->GetGameObject()->SetActive(false);
					}

					SetTargetThemeJuice(myAfterBridgeBreak, 20.f);

					if (GameObject* lightningGameObject = myLightningGameObject.Get())
					{
						auto lightning = lightningGameObject->GetComponent<FinalBossLightning>();
						lightningGameObject->SetActive(true);
					}

					PulseExposure();
				});
		}
	}
	else
	{
		LOG_ERROR(LogType::Game) << "Final boss missing Broken Bridge reference";
	}
}

void FB::FinalBoss::UpdateExposure()
{
	if (myIsExposureTimerStarted)
	{
		myExposureTimer.Tick(Time::DeltaTime);

		if (myExposureTimer.JustFinished())
		{
			myIsExposureTimerStarted = false;
		}

		if (auto pp = myGameObject->GetSingletonComponent<Engine::PostProcessComponent>())
		{
			if (myExposureCurveRef && myExposureCurveRef->IsValid())
			{
				pp->GetTonemappingSettingsRef().myExposure =
					myExposureCurveRef->Get().Evaluate(myExposureTimer.Percent());
			}
		}
	}

}

void FB::FinalBoss::PulseExposure()
{
	myExposureTimer = TickTimer::FromSeconds(myExposurePulseDuration);
	myIsExposureTimerStarted = true;
}

float FB::FinalBoss::GetTimeUntilFadeToCredits() const
{
	return myTimeUntilFadeToCredits;
}

Engine::BezierSplineComponent* FB::FinalBoss::GetLeftHandIntroBezier()
{
	if (GameObject* g = myLeftHandIntroSplineRef.Get())
	{
		if (const auto bezier = g->GetComponent<Engine::BezierSplineComponent>())
		{
			return bezier;
		}
	}

	return nullptr;
}

Vec3f FB::FinalBoss::GetBrokenBridgePos() const
{
	if (myBrokenBridgeGameObject && myBrokenBridgeGameObject.IsValid())
	{
		return myBrokenBridgeGameObject.Get()->GetTransform().GetPosition();
	}

	return Vec3f();
}

void FB::FinalBoss::SetTargetThemeJuice(const ThemeJuice& aThemeJuice, const float aTransitionSpeed)
{
	myThemeJuiceTransitionSpeed = aTransitionSpeed;
	myTargetThemeJuice = aThemeJuice;
}

void FB::FinalBoss::SetThemeJuice(const ThemeJuice& aThemeJuice)
{
	if (auto pp = myGameObject->GetSingletonComponent<Engine::PostProcessComponent>())
	{
		auto& fog = pp->GetFogSettingsRef();
		fog.myColor = aThemeJuice.myFogColor;
		fog.myStart = aThemeJuice.myFogStart;
		fog.myEnd = aThemeJuice.myFogEnd;

		pp->GetBloomSettingsRef().myIntensity = aThemeJuice.myBloomIntensity;
	}

	if (auto envLightGameObject = myGameObject->GetScene()->GetEnvironmentLight())
	{
		envLightGameObject->GetTransform().SetRotation(aThemeJuice.myEnvironmentLightDirection);

		if (auto environmentLight = envLightGameObject->GetComponent<Engine::EnvironmentLightComponent>())
		{
			// make it black and purple
			environmentLight->SetCubemapIntensity(aThemeJuice.myEnvironmentLightCubemapIntensity);
			environmentLight->SetColor(aThemeJuice.myEnvironmentLightColor);
			environmentLight->SetColorIntensity(aThemeJuice.myEnvironmentLightColorIntensity);
		}
	}

	if (myPortal)
	{
		myPortal->SetPortalColor(aThemeJuice.myPortalColor);
	}
}

void FB::FinalBoss::UpdateThemeJuice()
{
	float t = Time::DeltaTime * myThemeJuiceTransitionSpeed;
	t = Math::Clamp(t, 0.f, 1.f);

	myCurrentThemeJuice.myFogColor = Math::Lerp(myCurrentThemeJuice.myFogColor, myTargetThemeJuice.myFogColor, t);
	myCurrentThemeJuice.myFogStart = Math::Lerp(myCurrentThemeJuice.myFogStart, myTargetThemeJuice.myFogStart, t);
	myCurrentThemeJuice.myFogEnd = Math::Lerp(myCurrentThemeJuice.myFogEnd, myTargetThemeJuice.myFogEnd, t);

	myCurrentThemeJuice.myPortalColor = Math::Lerp(myCurrentThemeJuice.myPortalColor, myTargetThemeJuice.myPortalColor, t);

	myCurrentThemeJuice.myBloomIntensity = Math::Lerp(
		myCurrentThemeJuice.myBloomIntensity,
		myTargetThemeJuice.myBloomIntensity,
		t);

	myCurrentThemeJuice.myEnvironmentLightDirection = Quatf::Slerp(
		myCurrentThemeJuice.myEnvironmentLightDirection,
		myTargetThemeJuice.myEnvironmentLightDirection,
		t).EulerAngles();

	myCurrentThemeJuice.myEnvironmentLightCubemapIntensity = Math::Lerp(
		myCurrentThemeJuice.myEnvironmentLightCubemapIntensity,
		myTargetThemeJuice.myEnvironmentLightCubemapIntensity,
		t);

	myCurrentThemeJuice.myEnvironmentLightColor = Math::Lerp(
		myCurrentThemeJuice.myEnvironmentLightColor,
		myTargetThemeJuice.myEnvironmentLightColor,
		t);

	myCurrentThemeJuice.myEnvironmentLightColorIntensity = Math::Lerp(
		myCurrentThemeJuice.myEnvironmentLightColorIntensity,
		myTargetThemeJuice.myEnvironmentLightColorIntensity,
		t);

	SetThemeJuice(myCurrentThemeJuice);
}

FB::FinalBoss::ThemeJuice FB::FinalBoss::GetActiveThemeJuice() const
{
	ThemeJuice theme;

	if (auto pp = myGameObject->GetSingletonComponent<Engine::PostProcessComponent>())
	{
		auto& fog = pp->GetFogSettingsRef();
		theme.myFogColor = fog.myColor;
		theme.myFogStart = fog.myStart;
		theme.myFogEnd = fog.myEnd;

		theme.myBloomIntensity = pp->GetBloomSettingsRef().myIntensity;
	}

	if (auto envLightGameObject = myGameObject->GetScene()->GetEnvironmentLight())
	{
		theme.myEnvironmentLightDirection = envLightGameObject->GetTransform().GetRotation().EulerAngles();

		if (auto environmentLight = envLightGameObject->GetComponent<Engine::EnvironmentLightComponent>())
		{
			// make it black and purple
			theme.myEnvironmentLightCubemapIntensity = environmentLight->GetCubemapIntensity();
			theme.myEnvironmentLightColor = environmentLight->GetColor();
			theme.myEnvironmentLightColorIntensity = environmentLight->GetColorIntensity();
		}
	}

	return theme;
}

const FB::FinalBoss::ThemeJuice& FB::FinalBoss::GetInitialThemeJuice() const
{
	return myInitialThemeJuice;
}

void FB::FinalBoss::StartIntro()
{
	if (myHasStartedIntro)
	{
		return;
	}

	myHasStartedIntro = true;

	if (myPortal)
	{
		myPortal->Pulse(10.f, 1.f, 1.f);
	}

	ToggleLimbs(true);

	//if (auto musicManager = myGameObject->GetSingletonComponent<Engine::MusicManager>())
	//{
	//	//musicManager->Play();
	//	musicManager->ActivateFinalBoss();
	//}
	//else
	//{
	//	LOG_ERROR(LogType::Game) << "cannot find MusicManager";
	//}

	auto mainCam = myGameObject->GetScene()->GetMainCameraGameObject();
	auto shakeComponent = mainCam->GetComponent<Engine::CameraShakeComponent>();
	if (shakeComponent)
	{
		shakeComponent->AddPerlinShake("FinalBossIntro");
	}

	if (myFlow)
	{
		ChangePhase(FinalBossPhase::Introduction);
	}
}
