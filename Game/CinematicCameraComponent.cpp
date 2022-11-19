#include "pch.h"
#include "CinematicCameraComponent.h"

#include "AI/PollingStation/PollingStationComponent.h"
#include "Camera/CameraController.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/BezierSplineComponent.h"
#include "Player/Player.h"


void CinematicCameraComponent::Start()
{
	Component::Start();

	if (auto polling = myGameObject->GetSingletonComponent<PollingStationComponent>())
	{
		myPlayerGameObject = polling->GetPlayer();
		myPlayerComponent = myPlayerGameObject->GetComponent<Player>();
		myCameraController = myPlayerGameObject->GetComponent<CameraController>();
	}

	myCameraMoveBezier = &myCameraMoveBezierRef.Get()->GetComponent<Engine::BezierSplineComponent>()->GetSpline();
	myLookAtBezier = &myLookAtBezierRef.Get()->GetComponent<Engine::BezierSplineComponent>()->GetSpline();
}

void CinematicCameraComponent::Execute(Engine::eEngineOrder aOrder)
{
	Component::Execute(aOrder);

	if (GetEngine().GetInputManager().IsKeyDown(C::KeyCode::C))
	{
		myCameraTimer = TickTimer::FromSeconds(myCinematicLenght);
		ToggleHUDVisibility(false);
	}

	if (!myCameraTimer.IsFinished())
	{
		myCameraTimer.Tick(Time::DeltaTime);
		const Vec3f cameraCurvePos = myCameraMoveBezier->Evaluate(myCameraTimer.Percent());
		const Vec3f lookAtCurvePos = myLookAtBezier->Evaluate(myCameraTimer.Percent());

		myCameraRef.Get()->GetTransform().SetPosition(myCameraMoveBezierRef.Get()->GetTransform().GetPosition() + cameraCurvePos);
		myLookAtRef.Get()->GetTransform().SetPosition(myLookAtBezierRef.Get()->GetTransform().GetPosition() + lookAtCurvePos);

		myCameraRef.Get()->GetTransform().LookAt(myLookAtRef.Get()->GetTransform().GetPosition());
	}
}

void CinematicCameraComponent::Reflect(Engine::Reflector& aReflector)
{
	Component::Reflect(aReflector);

	aReflector.Reflect(myCinematicLenght, "Camera transition time");
	aReflector.Reflect(myCameraRef, "Camera");
	aReflector.Reflect(myCameraMoveBezierRef, "CameraMove Bezier");
	aReflector.Reflect(myLookAtRef, "LookAt Target");
	aReflector.Reflect(myLookAtBezierRef, "LookAt Bezier");
}

void CinematicCameraComponent::ToggleHUDVisibility(const bool aVisible)
{
	if (Transform* parent = &myPlayerGameObject->GetTransform().GetParent()->GetChildByGameObjectName("HUD")->GetTransform())
	{
		if (aVisible)
		{
			parent->SetPosition(Vec3f(0.f, 0.f, 0.f));
		}
		else
		{
			parent->SetPosition(Vec3f(0.f, 1000.f, 0.f));
		}
	}
	else
	{
		LOG_ERROR(LogType::Game) << "Missing HUD obj in scripted event";
	}

	if (myPlayerComponent)
	{
		if (aVisible)
		{
			myPlayerComponent->GetStatus().overrideHUD = false;
		}
		else
		{
			myPlayerComponent->GetStatus().overrideHUD = true;
		}
	}
	else
	{
		LOG_ERROR(LogType::Game) << "Missing player component in scripted event";
	}
}