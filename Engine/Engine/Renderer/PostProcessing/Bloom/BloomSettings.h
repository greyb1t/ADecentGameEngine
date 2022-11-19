#pragma once

#include "Engine\Reflection\Reflectable.h"

namespace Engine
{
	struct BloomSettings2 : Reflectable
	{
		bool myEnabled = true;

		float myIntensity = 3.f;
		float mySampleScale = 1.f;
		float myThreshold = 1.f;
		float mySoftThreshold = 0.25f;

		int myIterationsOffset = 2;

		void Reflect(Reflector& aReflector) override;
	};
}