#pragma once
#include "AI/PollingStation/PollingStationComponent.h"
#include "Camera/CameraController.h"
#include "Engine/GameObject/Components/Component.h"
#include "Player/Player.h"
#include "Utils/TickTimer.h"

class CinematicCameraComponent : public Component
{
	COMPONENT(CinematicCameraComponent, "Cinematic Camera");

public:
	void Start() override;
	void Execute(Engine::eEngineOrder aOrder) override;
	void Reflect(Engine::Reflector& aReflector) override;
	void ToggleHUDVisibility(bool aVisible);

private:
	Engine::GameObjectRef myCameraRef;
	Engine::GameObjectRef myCameraMoveBezierRef;
	Engine::GameObjectRef myLookAtBezierRef;
	Engine::GameObjectRef myLookAtRef;
	TickTimer myCameraTimer = { };

	bool myIsFinished = false;
	float myCinematicLenght = 0;
	BezierSpline* myLookAtBezier = nullptr;
	BezierSpline* myCameraMoveBezier = nullptr;
	GameObject* myPlayerGameObject = nullptr;
	CameraController* myCameraController = nullptr;
	Player* myPlayerComponent = nullptr;
};

