#include "pch.h"
#include "CameraShakeBase.h"
#include "Engine/Engine.h"
#include "Engine/Renderer\Camera\Camera.h"

// void Engine::CameraShakeBase::Init(const float aDurationSeconds, const float aCooldown)
// {
// 	myTimeLeftSeconds = aDurationSeconds;
// 
// 	// -1 to start first shake immediately
// 	myCooldownTimer = -1.f;
// 
// 	myCooldown = aCooldown;
// }

//CU::Vector3f Engine::CameraShakeBase::Update(Renderer::Camera& aCamera)
//{
//	const float deltaTime = GetEngine().GetTimer().GetDeltaTime();
//
//	if (myCooldownTimer < 0.f)
//	{
//		myCooldownTimer = myCooldown;
//
//		const auto shakeOffset = GetShakeOffset();
//
//		myActiveShakeOffset = shakeOffset;
//	}
//
//	myTimeLeftSeconds -= deltaTime;
//	myCooldownTimer -= deltaTime;
//
//	return myActiveShakeOffset;
//}

// void Engine::CameraShakeBase::ResetCameraOffset(Renderer::Camera& aCamera)
// {
// 	LOG_INFO(LogType::Filip) << "reset offset";
// 	aCamera.SetOffsetPosition(myActiveShakeOffset);
// }

bool Engine::CameraShakeBase::IsFinished() const
{
	return myIsFinished;
}

const Engine::Displacement& Engine::CameraShakeBase::GetDisplacement() const
{
	return myDisplacement;
}
