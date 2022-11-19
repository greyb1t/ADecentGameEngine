#include "pch.h"
#include "CameraController.h"

#include "Engine/Engine.h"

#include "Common/InputManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/SceneManager.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/CameraShakeComponent.h"

#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "Engine/GameObject/Components/CameraComponent.h"

#include "Engine\Renderer\GraphicsEngine.h"

#include "Engine/GameObject/Components/AudioComponent.h"

#include "Engine\AnimationCurve\Curve.h"
#include "Engine\Reflection\Reflector.h"
#include "Engine\ResourceManagement\Resources\AnimationCurveResource.h"

#include "Game/GameManager/GameManager.h"

#include "Engine\GameObject\Components\CameraShakeComponent.h"

CameraController::CameraController(GameObject* aGameObject) : Component(aGameObject)
{
}

CameraController::~CameraController()
{
	GetEngine().GetInputManager().ShowCursorGuaranteed();

	GetEngine().SetCursorMode(Engine::CursorMode::None);
	GetEngine().SetCursorVisiblity(true);
}

void CameraController::Start()
{
	myCamera = myGameObject->GetScene()->GetMainCameraGameObject();

	auto camShake = myCamera->GetComponent<Engine::CameraShakeComponent>();
	if(!camShake)
		myCamera->AddComponent<Engine::CameraShakeComponent>();

	if (myZoomCurveRef)
	{
		myZoomCurve = myZoomCurveRef->Get();
	}
	if (myVerticalZoomCurveRef)
	{
		myVerticalZoomCurve = myVerticalZoomCurveRef->Get();
	}

	LoadMetrics();

	myIsControllingCamera = true;
	ShowCursor(false);

	myCurrentPosition = myGameObject->GetTransform().GetPosition();
}

void CameraController::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myIsControllingCamera, "Controlling Camera");
	aReflector.Reflect(myIsOnlyControllingPos, "Is Only Controlling Position");

	aReflector.Header("Sensitivity");
	aReflector.Reflect(myMouseSensitivityMultiplier, "Mouse Sensitivity Mulitplier");
	aReflector.Reflect(myControllerSensitivityMultiplier, "Controller Sensitivity Mulitplier");

	aReflector.Header("Camera Rig");
	aReflector.Reflect(myMainMetrics.myPivotOffset.y, "myPivotOffsetY");

	aReflector.Reflect(myMainMetrics.myUpperCirlceRadius, "myUpperCirlceRadius");
	aReflector.Reflect(myMainMetrics.myUpperCircleHeight, "myUpperCircleHeight");
	
	aReflector.Reflect(myMainMetrics.myMiddleCirlceRadius, "myMiddleCirlceRadius");

	aReflector.Reflect(myMainMetrics.myLowerCirlceRadius, "myLowerCirlceRadius");
	aReflector.Reflect(myMainMetrics.myLowerCircleHeight, "myLowerCircleHeight");

	aReflector.Header("Run FOV Change");
	aReflector.Reflect(myStartFOV, "Base FOV");
	aReflector.Reflect(myRunFOVMultiplier, "myRunFOVMultiplier");
	aReflector.Reflect(myZoomCurveRef, "Zoom Curve");
	aReflector.Reflect(myZoomSpeed, "Zoom Speed");

	aReflector.Header("Camera Follow Strength");
	aReflector.Reflect(myDecayCoefficient, "Decay Coefficient");
	aReflector.Reflect(myMaxAcceleration, "Max Acceleration");
	aReflector.Reflect(myMaxDistance, "Max Distance");

	aReflector.Header("Vertical Zoom");
	aReflector.Reflect(myVerticalZoomCurveRef, "Vertical Zoom Curve");
	aReflector.Reflect(myVerticalZoomSpeed, "Vertical Zoom Speed");
	aReflector.Reflect(myVerticalMaxDistance, "Vertical Zoom Max Distace");

	aReflector.Reflect(myZoomOutSpeed, "Camera Collision Zoom Speed");

	myMainRig.LoadMetrics(myMainMetrics);
}

void CameraController::Execute(Engine::eEngineOrder aOrder)
{
	auto& input = GetEngine().GetInputManager();
	float deltaTime = Time::DeltaTimeUnscaled;



#ifndef _RETAIL
	myMainRig.DrawRig(myGameObject->GetTransform().GetPosition());


	if (input.IsKeyDown(C::KeyCode::F3))
	{
		myIsControllingCamera = !myIsControllingCamera;
	}
#endif


	if (myIsShowingCursor == myIsControllingCamera)
	{
		ShowCursor(!myIsShowingCursor);
	}

	if (myIsControllingCamera)
	{
		// Center the cursor in window when released right mouse button
		HWND windowHandle = GetEngine().GetGraphicsEngine().GetWindowHandler().GetWindowHandle();

		RECT rect;
		GetWindowRect(windowHandle, &rect);
		const auto& windowPos = GetEngine().GetGraphicsEngine().GetWindowHandler().GetPosition();
	}


	if (!myIsControllingCamera && !myIsOnlyControllingPos)
		return;

	ControlCamera(deltaTime);
	VerticalDistance();
	HandleTransition();
	CameraCollisionAvoidance(deltaTime);
}

const Vec3f CameraController::GetCurrentRotation() const
{
	Vec3f forwardDir = myCamera->GetTransform().Forward();
	Vec3f result;
	result.y = atan2f(forwardDir.z, forwardDir.x) + C::PI * -0.5f;

	return result;
}

const Vec3f CameraController::GetCurrentDirection() const
{
	Vec3f cameraToMiddleDir = (myGameObject->GetTransform().GetPosition() - myCamera->GetTransform().GetPosition()).GetNormalized();

	return cameraToMiddleDir;
}

const bool& CameraController::GetIfControllingCamera() const
{
	return myIsControllingCamera;
}

void CameraController::SetCameraPosition(const Vec3f& aPosition)
{
	myCurrentPosition = aPosition;
}

void CameraController::LoadMetrics()
{
	myMainRig.LoadMetrics(myMainMetrics);

	auto mainCamera = myGameObject->GetScene()->GetMainCamera();
	mainCamera.SetFov(myStartFOV);
}

void CameraController::SetIsRunning(bool aIsRunning)
{
	myIsRunning = aIsRunning;
}

const Vec3f CameraController::GetPivotPoint() const
{
	return myCurrentPivotPosition;
}

void CameraController::SetMaxVelocity(float aVelocity)
{
	myMaxAcceleration = aVelocity;
}

Transform& CameraController::GetCamera()
{
	return myCamera->GetTransform();
}

void CameraController::SetIscontrollingCamera(const bool aBool)
{
	myIsControllingCamera = aBool;
}

void CameraController::SetTransitionTime(const float aLerpTime)
{
	myTransitionLerpTime = aLerpTime;
	myTransitionLerpTimer = 0.f;
}

void CameraController::MouseInput()
{
	auto& input = GetEngine().GetInputManager();

	auto mouseChange = input.GetMouseDeltaRaw();

	float mouseSensitivityMultiplier = GameManager::GetInstance()->GetGameSettings().myMouseSensitivityMultiplier;

	float shitvalue = 0.003f;
	myYawAngle -= static_cast<float>(mouseChange.x) * mouseSensitivityMultiplier * shitvalue;
	myPitchAngle += static_cast<float>(mouseChange.y) * mouseSensitivityMultiplier * shitvalue;
}

void CameraController::ControllerInput(float aDeltaTime)
{
	auto& input = GetEngine().GetInputManager();

	float magicNumber = 4.0f;
	myYawAngle -= input.GetRightX() * myControllerSensitivityMultiplier * magicNumber * Time::DeltaTimeUnscaled;
	myPitchAngle -= input.GetRightY() * myControllerSensitivityMultiplier * magicNumber * Time::DeltaTimeUnscaled;
}

void CameraController::ControlCamera(float aDeltaTime)
{
	auto& input = GetEngine().GetInputManager();

	if (!myIsOnlyControllingPos)
	{
		MouseInput();
	}

	ControllerInput(aDeltaTime);
	myPitchAngle = Common::Clamp(-1.0f, 1.0f, myPitchAngle);

	LerpRigs();


	float curveVal = UpdateRunFOVValue();
	float newFOV = C::Lerp(myStartFOV, myStartFOV * myRunFOVMultiplier, curveVal);
	myGameObject->GetScene()->GetMainCamera().SetFov(newFOV);
}

void CameraController::CameraCollisionAvoidance(float aDeltaTime)
{
	Vec3f toCameraDir = (myCurrentCameraPosition - myCurrentPivotPosition).GetNormalized();

	float maxDistance = (myCurrentCameraPosition - myCurrentPivotPosition).Length();

	RayCastHit hit;
	if (myGameObject->GetScene()->RayCast(myCurrentPivotPosition, toCameraDir, maxDistance, eLayer::CAMERACOLLIDABLE, hit) == true)
	{
		float currentDistance = (hit.Position - myCurrentPivotPosition).Length();

		myLastWallCollisionDistance = currentDistance * 0.9f;
		myCurrentZoomVal = 1.0f;
	}

	Vec3f wallCamPos = myCurrentPivotPosition + toCameraDir * myLastWallCollisionDistance;


	myCurrentZoomVal -= myZoomOutSpeed * aDeltaTime;
	myCurrentZoomVal = myCurrentZoomVal < 0.0f ? 0.0f : myCurrentZoomVal;

	Vec3f result;
	result = Math::Lerp(myCurrentCameraPosition, wallCamPos, myCurrentZoomVal);

	myCamera->GetTransform().SetPosition(result);
}

void CameraController::VerticalDistance()
{

	Vec3f toCameraDir = (myCurrentCameraPosition - myCurrentPivotPosition).GetNormalized();
	Vec3f farCamPos = myCurrentCameraPosition + toCameraDir * myVerticalMaxDistance;

	float curveVal = myVerticalZoomCurve.Evaluate(myPitchAngle);


	myCurrentVetricalZoomValue = C::Lerp(myCurrentVetricalZoomValue, curveVal, myVerticalZoomSpeed * Time::DeltaTime);
;
	Vec3f resultPos = Math::Lerp(myCurrentCameraPosition, farCamPos, myCurrentVetricalZoomValue);

	myCurrentCameraPosition = resultPos;
}

void CameraController::HandleTransition()
{
	if (myTransitionLerpTime > 0.f)
	{
		myTransitionLerpTimer += Time::DeltaTime;
		if (myTransitionLerpTimer > myTransitionLerpTime)
		{
			myTransitionLerpTime = -1.f;
			myTransitionLerpTimer = -1.f;
		}
		else
		{
			myCurrentCameraPosition = CU::Lerp(
				myCamera->GetTransform().GetPosition(), 
				myCurrentCameraPosition, 
				myTransitionLerpTimer / myTransitionLerpTime);
		}
	}
}

const float CameraController::UpdateRunFOVValue()
{
	bool shouldZoom = myIsRunning;

	float maxLerpVal = 1.0f;


	if (myZoomLerpVal > maxLerpVal)
		shouldZoom = false;

	float aimChange = (shouldZoom ? myZoomSpeed : -myZoomSpeed) * Time::DeltaTime;
	myZoomLerpVal += aimChange;


	if ((aimChange < 0.0f && myZoomLerpVal > maxLerpVal) == false)
	{
		myZoomLerpVal = Common::Clamp(0.0f, maxLerpVal, myZoomLerpVal);
	}

	return myZoomCurve.Evaluate(myZoomLerpVal); //Return the curve vaue
}

void CameraController::LerpRigs()
{
	Vec3f targetPosition = GetTargetPosition();

	Vec3f mainCamPos = myMainRig.GetPositionOnRig(myYawAngle, myPitchAngle, targetPosition);
	Vec3f mainPivotPos = myMainRig.GetPivotPosition(targetPosition);

	auto& cameraTransform = myCamera->GetTransform();

	cameraTransform.SetPosition(mainCamPos);
	cameraTransform.LookAt(mainPivotPos);

	myCurrentPivotPosition = mainPivotPos;
	myCurrentCameraPosition = cameraTransform.GetPosition();
}

const Vec3f CameraController::GetTargetPosition()
{
	Vec3f targetPosition = myGameObject->GetTransform().GetPosition();

	return targetPosition;

	//if (Time::DeltaTime > 0.05f)
	//{
	//	myCurrentPosition = targetPosition;
	//	return myCurrentPosition;
	//}

	//Vec3f toPlayer = targetPosition - myCurrentPosition;
	//float distance = toPlayer.Length();

	//if (distance > myMaxDistance)
	//{
	//	const float strength = (std::min)(myDecayCoefficient * (distance * distance), myMaxAcceleration);
	//	myCurrentPosition += toPlayer.GetNormalized() * strength * Time::DeltaTime;

	//	//LOG_INFO(LogType::Simon) << strength * Time::DeltaTime;
	//}

	////myCurrentPosition = targetPosition;

	//return myCurrentPosition;
}

void CameraController::ShowCursor(bool aShouldShow)
{
	if (aShouldShow)
	{
		GetEngine().SetCursorMode(Engine::CursorMode::None);
		GetEngine().SetCursorVisiblity(true);
	}
	else
	{
		GetEngine().SetCursorMode(Engine::CursorMode::CenterLocked);
		GetEngine().SetCursorVisiblity(false);
	}

	myIsShowingCursor = aShouldShow;
}
