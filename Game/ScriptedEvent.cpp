#include "pch.h"
#include "ScriptedEvent.h"

#include "Camera/CameraController.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/BezierSplineComponent.h"
#include "Engine/TimeSystem/TimeSystem.h"
#include "Engine/TriggerEventManager/TriggerEventManager.h"
#include <Engine/GameObject/Components/InfoRigidStaticComponent.h>

#include "PhysicsUtilities.h"
#include "Engine/GameObject/Components/ModelComponent.h"
#include "Engine/GameObject/Components/RigidStaticComponent.h"
#include "Physics/RigidStatic.h"
#include "Player/Player.h"
#include "Player/States/MovementState.h"
#include "Player/States/PlayerEnterBossScriptedState.h"
#include "AI/FinalBoss/FinalBoss.h"
#include "AI/AIDirector/AIDirector.h"
#include "AI/PollingStation/PollingStationComponent.h"
#include "Components/HealthComponent.h"
#include "Engine/GameObject/Components/AudioComponent.h"
#include "Engine/GameObject/Components/MusicManager.h"

ScriptedEvent::ScriptedEvent()
{
}

ScriptedEvent::ScriptedEvent(GameObject* aGameObject)
	: Component(aGameObject)
{
}

ScriptedEvent::~ScriptedEvent()
{
	TriggerEventManager::GetInstance().Unsubscribe(this, "EnteredBossArena");
}

void ScriptedEvent::Start()
{
	Component::Start();

	TriggerEventManager::GetInstance().Subscribe(this, "EnteredBossArena");

	if (!myLookAtRef)
	{
		LOG_ERROR(LogType::Game) << "Scripted Event is missing LookAt gameobject";
		myGameObject->Destroy();
		return;
	}

	if (!myCameraRef)
	{
		LOG_ERROR(LogType::Game) << "Scripted Event is missing camera gameobject";
		myGameObject->Destroy();
		return;
	}

	// if (!myPlayerRef)
	// {
	// 	LOG_ERROR(LogType::Game) << "Scripted Event is missing player gameobject";
	// 	myGameObject->Destroy();
	// 	return;
	// }

	if (!myCameraMoveBezierRef)
	{
		LOG_ERROR(LogType::Game) << "Scripted Event is missing bezier curve gameobject";
		myGameObject->Destroy();
		return;
	}


	if (!myCameraMoveBezierRef)
	{
		LOG_ERROR(LogType::Game) << "Scripted Event is missing animation curve";
		myGameObject->Destroy();
		return;
	}

	if (!myFinalBossRef)
	{
		LOG_ERROR(LogType::Game) << "Scripted Event is missing final boss ref";
		myGameObject->Destroy();
		return;
	}

	if (!myHandRef)
	{
		LOG_ERROR(LogType::Game) << "Scripted Event is missing hand ref";
		myGameObject->Destroy();
		return;
	}

	if (!myHandSplineRef)
	{
		LOG_ERROR(LogType::Game) << "Scripted Event is missing hand spline ref";
		myGameObject->Destroy();
		return;
	}

	if (!myHandSpeedCurveRef)
	{
		LOG_ERROR(LogType::Game) << "Scripted Event is missing hand speed curve";
		myGameObject->Destroy();
		return;
	}

	if (!myBridgeAudioSourceRef)
	{
		LOG_ERROR(LogType::Game) << "Scripted Event is missing bridge audio source";
		myGameObject->Destroy();
		return;
	}

	if (GameObject* audioSourceObj = myBridgeAudioSourceRef.Get())
	{
		if (auto audio = audioSourceObj->GetComponent<Engine::AudioComponent>())
		{
			myBridgeAudio = audio;
		}
		else
		{
			LOG_ERROR(LogType::Game) << "Bridge audio source is missing AudioComponent";
			myGameObject->Destroy();
			return;
		}
	}

	myBridgeAudio->AddEvent("BridgeBreak", "event:/CINE/BridgeCollapse");

	myFinalBoss = myFinalBossRef.Get()->GetComponent<FB::FinalBoss>();
	myHand = myHandRef.Get();
	myHandSpline = &myHandSplineRef.Get()->GetComponent<Engine::BezierSplineComponent>()->GetSpline();
	myHandSpeedCurve = &myHandSpeedCurveRef->Get();

	myCameraMoveBezier = &myCameraMoveBezierRef.Get()->GetComponent<Engine::BezierSplineComponent>()->GetSpline();
	myLookAtBezier = &myLookAtBezierRef.Get()->GetComponent<Engine::BezierSplineComponent>()->GetSpline();

	if (auto polling = myGameObject->GetSingletonComponent<PollingStationComponent>())
	{
		//myPlayerGameObject = polling->GetPlayer()->GetTransform().GetChildByGameObjectName("Player");
		myPlayerGameObject = polling->GetPlayer();
	}
	else
	{
		LOG_ERROR(LogType::Game) << "scipted event missing polling station";
		return;
	}

	//myPlayerGameObject = myPlayerRef.Get()->GetTransform().GetChildByGameObjectName("Player");
	myPlayerComponent = myPlayerGameObject->GetComponent<Player>();
	myCameraController = myPlayerGameObject->GetComponent<CameraController>();

	myBridgeGameObject = myPlayerGameObject->GetScene()->FindGameObject("HelBro");
	if (!myBridgeGameObject)
	{
		LOG_ERROR(LogType::Game) << "Scripted Event is missing bridge gameobject";
		myGameObject->Destroy();
		return;
	}
}

void ScriptedEvent::Reflect(Engine::Reflector& aReflector)
{
	Component::Reflect(aReflector);

	aReflector.Reflect(myTurnTime, "Show turn time");
	aReflector.Reflect(myEventDuration, "Camera transition time");
	aReflector.Reflect(myBossShowTime, "Show boss time");

	aReflector.Reflect(myCameraRef, "Camera");
	// aReflector.Reflect(myPlayerRef, "Player");
	aReflector.Reflect(myCameraMoveBezierRef, "CameraMove Bezier");
	aReflector.Reflect(myLookAtRef, "LookAt Target");
	aReflector.Reflect(myLookAtBezierRef, "LookAt Bezier");
	aReflector.Reflect(myCameraCurveRef, "CameraMove Curve");
	aReflector.Reflect(myLookAtCurveRef, "LookAt Curve");
	aReflector.Reflect(myFinalBossRef, "Final Boss");
	aReflector.Reflect(myHandRef, "Hand");
	aReflector.Reflect(myHandSplineRef, "Hand Spline");

	aReflector.Reflect(myBreakBridgeAt, "Break Bridge At");
	aReflector.Reflect(myDoBossIntroAt, "Do Boss Intro At");
	aReflector.Reflect(myHandSlamDuration, "HandSlam Duration");

	aReflector.Reflect(myHandSpeedCurveRef, "Hand Speed Curve");
	aReflector.Reflect(myTimeScale, "Time Scale");

	aReflector.Reflect(myBridgeAudioSourceRef, "Bridge Audio Source");
}

void ScriptedEvent::Execute(Engine::eEngineOrder aOrder)
{
	Component::Execute(aOrder);

	if (myIsTriggered)
	{
		if (UIEventManager::GetIsPaused())
		{
			ToggleHUDVisibility(true);
		}
		else
		{
			ToggleHUDVisibility(false);
		}

		myTurnTimer.Tick(Time::DeltaTime * myTimeScale);

		if (myTurnTimer.JustFinished())
		{
			myBridgeAudio->PlayEvent("BridgeBreak");
		}

		if (myTurnTimer.IsFinished())
		{
			myCameraController->SetIscontrollingCamera(false);

			// NOTE(filip): Det gjorde att man inte kunde få musen i main menu, tog bort

			if (!UIEventManager::GetIsPaused())
			{
				GetEngine().SetCursorMode(Engine::CursorMode::CenterLocked);
				GetEngine().SetCursorVisiblity(false);
			}
			else
			{
				GetEngine().SetCursorMode(Engine::CursorMode::None);
				GetEngine().SetCursorVisiblity(true);
			}

			myCameraTimer.Tick(myTimeScale * Time::DeltaTime * myCameraCurveRef->Get().Evaluate(myCameraTimer.Percent()));
			myLookAtTimer.Tick(myTimeScale * Time::DeltaTime * myLookAtCurveRef->Get().Evaluate(myLookAtTimer.Percent()));

			// LOG_INFO(LogType::Filip) << "CameraTimer: " << myCameraTimer.Percent();

			myIsFinished = false;
			if (myCameraTimer.IsFinished() && myLookAtTimer.IsFinished())
			{
				myIsFinished = true;
				myBossCameraBool = true;
				myBossTimer.Tick(myTimeScale * Time::DeltaTime);
			}

			if (!myIsFinished)
			{
				const float cameraTime = myCameraTimer.Percent();
				const float lookAtTime = myLookAtTimer.Percent();

				const Vec3f cameraCurvePos = myCameraMoveBezier->Evaluate(cameraTime);
				const Vec3f lookAtCurvePos = myLookAtBezier->Evaluate(lookAtTime);

				myCameraRef.Get()->GetTransform().SetPosition(myCameraMoveBezierRef.Get()->GetTransform().GetPosition() + cameraCurvePos);
				myLookAtRef.Get()->GetTransform().SetPosition(myLookAtBezierRef.Get()->GetTransform().GetPosition() + lookAtCurvePos);

				myCameraRef.Get()->GetTransform().LookAt(myLookAtRef.Get()->GetTransform().GetPosition());
			}

			if (myCameraTimer.Percent() >= myDoBossIntroAt)
			{
				myFinalBoss->StartIntro();
			}

			UpdateHandPosition();
		}

		if (myTurnTimer.JustFinished())
		{
			// Must be done after releasing the camera
			GetEngine().SetCursorVisiblity(false);
		}

		if (myBossTimer.JustFinished() && myBossCameraBool)
		{
			SetPlayerInvincibility(false);
			ToggleHUDVisibility(true);

			myFinalBoss->StartBoss();

			myHand->SetActive(false);

			myCameraController->SetIscontrollingCamera(true);

			myPlayerComponent->GetStateMachine().SetState(myPlayerComponent->GetMovementState());
			Destroy();
		}
	}
}

void ScriptedEvent::OnEvent()
{
	TriggerEventListener::OnEvent();

	LOG_INFO(LogType::Jonathan) << "Scripted Event triggered";

	myIsTriggered = true;
	myTurnTimer = TickTimer::FromSeconds(myTurnTime);
	myBossTimer = TickTimer::FromSeconds(myBossShowTime);
	myCameraTimer = TickTimer::FromSeconds(myEventDuration);
	myLookAtTimer = TickTimer::FromSeconds(myEventDuration);
	myHandTimer = TickTimer::FromSeconds(myHandSlamDuration);

	//myPlayerGameObject->GetTransform().SetPosition(myGameObject->GetTransform().GetPosition());
	myPlayerComponent->GetStateMachine().SetState(myPlayerComponent->GetEnterBossScriptedState());

	if (myBridgeGameObject)
	{
		myBridgeGameObject->Destroy();
	}

	myFinalBoss->StopEnemiesSpawning();

	SetPlayerInvincibility(true);

	ToggleHUDVisibility(false);
}

void ScriptedEvent::UpdateHandPosition()
{
	myHand->SetActive(true);

	myHandTimer.Tick(myTimeScale * Time::DeltaTime * myHandSpeedCurve->Evaluate(myHandTimer.Percent()));

	if (myHandTimer.Percent() >= myBreakBridgeAt)
	{
		myFinalBoss->BreakBridge(*myBridgeAudio);
	}

	const Vec3f splinePos = myHandSpline->Evaluate(myHandTimer.Percent());

	myHand->GetTransform().SetPosition(myHandSplineRef.Get()->GetTransform().GetPosition() + splinePos);

	// LOG_INFO(LogType::Filip) << "Hand Percent: " << myHandTimer.Percent() << ", Pos: " << splinePos.x << ", " << splinePos.y << ", " << splinePos.z;
}

void ScriptedEvent::SetPlayerInvincibility(const bool aIsInvinsible)
{
	if (auto playerHealth = myPlayerComponent->GetGameObject()->GetComponent<HealthComponent>())
	{
		playerHealth->SetIsInvincible(aIsInvinsible);
	}
	else
	{
		LOG_ERROR(LogType::Game) << "Player from scripted event missing HealthComponent";
	}
}

void ScriptedEvent::ToggleHUDVisibility(const bool aVisible)
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
