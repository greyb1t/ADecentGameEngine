#include "pch.h"
#include "TonemappingSettings.h"
#include "Engine\Reflection\Reflector.h"

void Engine::TonemappingSettings::Reflect(Reflector& aReflector)
{
	aReflector.Reflect(myEnabled, "Enabled");

	if (myEnabled)
	{
		aReflector.Reflect(myTonemapType, "Tonemap Type");

		aReflector.SetNextItemRange(0.f, 1000.f);
		aReflector.SetNextItemSpeed(0.01f);
		aReflector.Reflect(myExposure, "Exposure");
	}
}
