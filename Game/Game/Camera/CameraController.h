#pragma once
#include "Common/Math/Internal/Vector3.h"
#include "Engine/GameObject/Components/Component.h"
#include "Engine\ResourceManagement\ResourceRef.h"
#include "Engine\AnimationCurve\Curve.h"

#include "CameraRig.h"


namespace Engine
{
	class AnimationController;
	class AudioComponent;
}	

class PlayerStateController;

class CameraController : public Component
{
public:
	COMPONENT_MASK(CameraController, "Camera Controller", Engine::eEngineOrder::UPDATE);

	CameraController() = default;
	CameraController(GameObject* aGameObject);
	~CameraController();

	void Start() override;

	void Reflect(Engine::Reflector& aReflector) override;

	void Execute(Engine::eEngineOrder aOrder) override;

	const Vec3f GetCurrentRotation() const;
	const Vec3f GetCurrentDirection() const;

	const bool& GetIfControllingCamera() const;

	void SetCameraPosition(const Vec3f& aPosition);

	void SetIscontrollingCamera(const bool aBool);
	void SetTransitionTime(const float aLerpTime);

	void LoadMetrics();

	void SetIsRunning(bool aIsRunning);

	const Vec3f GetPivotPoint() const;

	void SetMaxVelocity(float aVelocity);

	Transform& GetCamera();
private:
	void MouseInput();
	void ControllerInput(float aDeltaTime);

	void ControlCamera(float aDeltaTime);
	void CameraCollisionAvoidance(float aDeltaTime);

	void VerticalDistance();

	void HandleTransition();

	const float UpdateRunFOVValue();
	void LerpRigs();

	const Vec3f GetTargetPosition();

	void ShowCursor(bool aShouldShow);

	Vec3f myCurrentPosition;

	CameraRig myMainRig;

	float myYawAngle = 0.0f;
	float myPitchAngle = 0.0f;

	GameObject* myCamera = nullptr;

	float myMouseSensitivityMultiplier = 0.0f;
	float myControllerSensitivityMultiplier = 0.0f;

	float myZoomLerpVal = 0.0f;
	float myZoomSpeed = 0.0f;

	Vec3f myCurrentPivotPosition;
	Vec3f myCurrentCameraPosition;

	float myLastWallCollisionDistance = 0.0f;
	float myZoomOutSpeed = 0.9f;
	float myCurrentZoomVal = 0.0f;

	float myStartFOV = 90.0f;
	float myRunFOVMultiplier = 1.1f;

	float myVerticalZoomSpeed = 1.0f;
	float myCurrentVetricalZoomValue = 0.0f;
	float myVerticalMaxDistance = 300.0f;

	float myDecayCoefficient = 2.0f;
	float myMaxAcceleration = 1000.0f;
	float myMaxDistance = 2.0f;

	float myTransitionLerpTime = -1.f;
	float myTransitionLerpTimer = -1.f;

	bool myIsControllingCamera = true;
	bool myIsOnlyControllingPos = false;
	bool myIsShowingCursor = false;

	AnimationCurveRef myZoomCurveRef;
	AnimationCurveRef myVerticalZoomCurveRef;

	Engine::Curve myZoomCurve;
	Engine::Curve myVerticalZoomCurve;

	bool myIsRunning = false;

	CameraRig::CameraMetrics myMainMetrics;
};
