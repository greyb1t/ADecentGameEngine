#pragma once

#include "Engine/GameObject/Components/Component.h"
#include "Engine/GameObject/GameObjectRef.h"
#include "FinalBossPhase.h"
#include "Engine/TriggerEventManager/TriggerEventListener.h"
#include "FinalBossWaypoint.h"
#include "Utils/TickTimer.h"

class HealthComponent;

namespace FB
{
	class FinalBossBody;
	class RightProjectileHand;
	class LeftPunchHand;
	class FinalBossPortal;

	class FinalBoss : public Component
	{
	public:
		struct ThemeJuice : Engine::Reflectable
		{
			Vec3f myEnvironmentLightDirection = Vec3f(-10.2f, -25.85f, 37.58f) * Math::DegToRad;

			float myEnvironmentLightCubemapIntensity = 1.f;
			Vec3f myEnvironmentLightColor = Vec3f(0.041f, 0.000f, 0.247f);
			float myEnvironmentLightColorIntensity = 6.3f;

			Vec3f myFogColor = Vec3f(0.139f, 0.000f, 0.212f);
			float myFogStart = 0.f;
			float myFogEnd = 12000.f;

			float myBloomIntensity = 0.92f;

			Vec3f myPortalColor = Vec3f(1.f, 1.f, 1.f);

			void Reflect(Engine::Reflector& aReflector) override
			{
				aReflector.Reflect(myEnvironmentLightDirection, "EnvironmentLight Direction");

				aReflector.Reflect(myEnvironmentLightCubemapIntensity, "EnvironmentLight Cubemap Intensity");
				aReflector.Reflect(myEnvironmentLightColor, "EnvironmentLight Color");
				aReflector.Reflect(myEnvironmentLightColorIntensity, "EnvironmentLight Color Intensity");

				aReflector.Reflect(myFogColor, "Fog Color");
				aReflector.Reflect(myFogStart, "Fog Start");
				aReflector.Reflect(myFogEnd, "Fog End");

				aReflector.Reflect(myBloomIntensity, "Bloom Intensity");
				aReflector.Reflect(myPortalColor, "Portal Color");
			}
		};

		COMPONENT(FinalBoss, "Final Boss");

		FinalBoss();
		FinalBoss(GameObject* aGameObject);
		virtual ~FinalBoss();

		void Start() override;

		void Execute(Engine::eEngineOrder aOrder) override;
		void Render() override;

		void Reflect(Engine::Reflector& aReflector) override;

		RightProjectileHand& GetRightHand();
		LeftPunchHand& GetLeftHand();
		FinalBossBody& GetBody();

		FinalBossPortal* GetPortal();

		const Vec3f& GetGroundPosition() const;

		FinalBossPhase GetPhase() const;

		bool IsDead() const;

		void BreakBridge(Engine::AudioComponent& aAudioComponent);
		void StartIntro();
		// Starts the actual boss FIGHT
		void StartBoss();
		void StopEnemiesSpawning();
		void EnableEnemiesSpawning();

		FinalBossWaypoint* FindFreeWaypointClosestToPosition(
			const Vec3f& aPosition, 
			FinalBossWaypoint* aCurrentWaypoint);

		GameObject* GetPlayer() const;
		Vec3f GetPlayerPos() const;

		const GameObjectPrefabRef& GetCreditsVideoPrefab() const;
		const GameObjectPrefabRef& GetWhiteSpritePrefab() const;

		float GetHealthScaleFactor() const;

		bool ShouldStartBoss() const;

		void PulseExposure();

		float GetTimeUntilFadeToCredits() const;

		Engine::BezierSplineComponent* GetLeftHandIntroBezier();

		Vec3f GetBrokenBridgePos() const;

		void SetTargetThemeJuice(const ThemeJuice& aThemeJuice, const float aTransitionSpeed);
		const ThemeJuice& GetInitialThemeJuice() const;

	private:
		void ChangePhase(const FinalBossPhase aPhase);

		void OnEnterPhase(const FinalBossPhase aPhase);

		void UpdateGroundPosition();

		void ToggleLimbs(const bool aEnabled);
		void RegisterScriptedEventBridgeEvents();

		void UpdateExposure();

		void SetThemeJuice(const ThemeJuice& aThemeJuice);
		void UpdateThemeJuice();
		ThemeJuice GetActiveThemeJuice() const;

	private:
		RightProjectileHand* myRightHand = nullptr;
		LeftPunchHand* myLeftHand = nullptr;
		FinalBossBody* myBody = nullptr;

		FinalBossPortal* myPortal = nullptr;

		FinalBossPhase myPhase = FinalBossPhase::None;

		Engine::GameObjectRef myBodyGameObject;
		Engine::GameObjectRef myLeftHandGameObject;
		Engine::GameObjectRef myRightHandGameObject;

		Engine::GameObjectRef myBrokenBridgeGameObject;
		Engine::GameObjectRef myLightningGameObject;

		Engine::GameObjectRef myFinalBossPortalRef;

		Engine::GameObjectRef myLeftHandIntroSplineRef;

		GameObjectPrefabRef myCreditsVideoPrefab;
		GameObjectPrefabRef myWhiteSpritePrefab;

		Engine::GameObjectRef myWaypointsGameObject;
		std::vector<FinalBossWaypoint> myWaypoints;

		bool myFlow = true;

		bool myHasStartedIntro = false;

		bool myShouldStartBoss = false;
		bool myHasBrokenBridge = false;

		std::vector<Engine::GameObjectRef> myPillars;

		Vec3f myGroundPosition;

		std::array<float, GameDifficulty::_size()> myHealthScaleFactors = { 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f };

		ThemeJuice myInitialThemeJuice;
		ThemeJuice myAfterBridgeBreak;
		ThemeJuice myEnteringPhase2;
		ThemeJuice myCurrentThemeJuice;
		ThemeJuice myTargetThemeJuice;
		float myThemeJuiceTransitionSpeed = 1.f;

		float myMaxPlayerDistanceFromCenterArena = 9000.f;

		TickTimer myExposureTimer;
		bool myIsExposureTimerStarted = false;
		AnimationCurveRef myExposureCurveRef;
		float myExposurePulseDuration = 0.2f;

		float myTimeUntilFadeToCredits = 8.f;
	};
}