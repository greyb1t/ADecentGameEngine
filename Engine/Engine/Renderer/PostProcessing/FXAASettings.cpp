#include "pch.h"
#include "FXAASettings.h"
#include "Engine\Reflection\Reflector.h"

void Engine::FXAASettings::Reflect(Reflector& aReflector)
{
	aReflector.Reflect(myEnabled, "Enabled");

	if (myEnabled)
	{
	}
}
