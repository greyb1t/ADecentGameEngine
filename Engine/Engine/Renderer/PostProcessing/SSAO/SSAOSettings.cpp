#include "pch.h"
#include "SSAOSettings.h"
#include "Engine\Reflection\Reflector.h"

void Engine::SSAOSettings::Reflect(Reflector& aReflector)
{
	aReflector.Reflect(myEnabled, "Enabled");

	if (myEnabled)
	{
		aReflector.SetNextItemRange(0.f, 1000.f);
		aReflector.SetNextItemSpeed(0.01f);
		aReflector.Reflect(myIntensity, "Intensity");
	}
}
