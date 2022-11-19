#include "pch.h"
#include "BloomSettings.h"
#include "Engine\Reflection\Reflector.h"

void Engine::BloomSettings2::Reflect(Reflector& aReflector)
{
	aReflector.Reflect(myEnabled, "Enabled");

	if (myEnabled)
	{
		aReflector.SetNextItemRange(0.f, 1000.f);
		aReflector.SetNextItemSpeed(0.01f);
		aReflector.Reflect(myIntensity, "Intensity");

		aReflector.SetNextItemRange(0.f, 100.f);
		aReflector.SetNextItemSpeed(0.01f);
		aReflector.Reflect(mySampleScale, "Sample Scale");

		aReflector.SetNextItemTooltip("Should usually be 1 because we using HDR, and only pixels above 1 should be affected by bloom");
		aReflector.SetNextItemRange(0.f, 100.f);
		aReflector.SetNextItemSpeed(0.01f);
		aReflector.Reflect(myThreshold, "Threshold");

		aReflector.SetNextItemRange(0.f, 1.f);
		aReflector.SetNextItemSpeed(0.01f);
		aReflector.Reflect(mySoftThreshold, "Soft Threshold");

		aReflector.SetNextItemRange(0.f, 20.f);
		aReflector.SetNextItemSpeed(1);
		aReflector.Reflect(myIterationsOffset, "Iterations Offset");
	}
}
