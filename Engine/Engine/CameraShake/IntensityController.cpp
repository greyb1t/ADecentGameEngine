#include "pch.h"
#include "IntensityController.h"

Engine::IntensityController::IntensityController(const IntensityControllerDesc& aDesc)
	: myDesc(aDesc),
	mySustainTimerSec(aDesc.mySustainIntensityDurationSeconds)
{
}

static float easeInOutSine(float x)
{
	return -(cos(C::PI * x) - 1) / 2;
}

void Engine::IntensityController::Update(const float aDeltaTime)
{
	if (myFinished)
	{
		return;
	}

	switch (myState)
	{
		case Engine::IntensityController::Increase:
		{
			myInternalIntensity += aDeltaTime * myDesc.myHowFastToIncreaseIntensity;

			if (myInternalIntensity > myDesc.myTargetIntensity)
			{
				myInternalIntensity = myDesc.myTargetIntensity;

				myState = Sustain;
			}
		} break;

		case Engine::IntensityController::Sustain:
		{
			mySustainTimerSec -= aDeltaTime;

			if (mySustainTimerSec <= 0.f)
			{
				myState = Decrease;
			}
		} break;

		case Engine::IntensityController::Decrease:
		{
			myInternalIntensity -= aDeltaTime * myDesc.myHowFastToDecreaseIntensity;

			if (myInternalIntensity <= 0.f)
			{
				myFinished = true;
			}
		} break;

		default:
			assert(false);
			break;
	}

	myIntensity = easeInOutSine(Math::Clamp(myInternalIntensity, 0.f, 1.f));
}

float Engine::IntensityController::GetIntensity() const
{
	return myIntensity;
}

bool Engine::IntensityController::IsFinished() const
{
	return myFinished;
}

void Engine::IntensityControllerDesc::InitFromJson(const nlohmann::json& aJson)
{
	myTargetIntensity = aJson["TargetIntensity"];
	myHowFastToIncreaseIntensity = aJson["HowFastToIncreaseIntensity"];
	mySustainIntensityDurationSeconds = aJson["SustainIntensityDurationSeconds"];
	myHowFastToDecreaseIntensity = aJson["HowFastToDecreaseIntensity"];
}
