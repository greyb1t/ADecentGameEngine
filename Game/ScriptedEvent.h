#pragma once
#include "Camera/CameraController.h"
#include "Engine/GameObject/Components/Component.h"
#include "Engine/ResourceManagement/Resources/AnimationCurveResource.h"
#include "Engine/TriggerEventManager/TriggerEventListener.h"
#include "Player/Player.h"
#include "Utils/TickTimer.h"

namespace FB
{
	class FinalBoss;
}

namespace Engine
{
	class Curve;
}

class ScriptedEvent : public Component, public TriggerEventListener
{
public:
	COMPONENT(ScriptedEvent, "Scripted Event");

	ScriptedEvent();
	ScriptedEvent(GameObject* aGameObject);
	virtual ~ScriptedEvent();

	void Start() override;
	void Reflect(Engine::Reflector& aReflector) override;
	void Execute(Engine::eEngineOrder aOrder) override;

	void OnEvent() override;

private:
	void UpdateHandPosition();
	void SetPlayerInvincibility(const bool aIsInvinsible);
	void ToggleHUDVisibility(const bool aVisible);

private:
	Engine::GameObjectRef myFinalBossRef;
	FB::FinalBoss* myFinalBoss = nullptr;
	Engine::GameObjectRef myHandRef;
	GameObject* myHand = nullptr;
	Engine::GameObjectRef myHandSplineRef;
	BezierSpline* myHandSpline = nullptr;
	TickTimer myHandTimer;
	AnimationCurveRef myHandSpeedCurveRef;
	Engine::Curve* myHandSpeedCurve = nullptr;
	Engine::GameObjectRef myBridgeAudioSourceRef;
	Engine::AudioComponent* myBridgeAudio = nullptr;

	Engine::GameObjectRef myCameraRef;
	// Engine::GameObjectRef myPlayerRef;
	Engine::GameObjectRef myCameraMoveBezierRef;
	Engine::GameObjectRef myLookAtBezierRef;
	Engine::GameObjectRef myLookAtRef;
	GameObject* myBridgeGameObject;
	AnimationCurveRef myCameraCurveRef;
	AnimationCurveRef myLookAtCurveRef;
	bool myIsTriggered = false;
	bool myIsFinished = false;
	bool myBossCameraBool;

	BezierSpline* myCameraMoveBezier = nullptr;
	BezierSpline* myLookAtBezier = nullptr;
	CameraController* myCameraController = nullptr;
	GameObject* myPlayerGameObject = nullptr;
	Player* myPlayerComponent = nullptr;

	TickTimer myCameraTimer = { };
	TickTimer myLookAtTimer = { };
	TickTimer myTurnTimer = { };
	TickTimer myBossTimer = { };
	float myEventDuration = 1.f;
	float myTurnTime = 1.f;
	float myBossShowTime = 1.f;

	float myBreakBridgeAt = 0.3f;
	float myDoBossIntroAt = 0.8f;

	float myHandSlamDuration = 1.f;

	float myTimeScale = 1.f;
};

