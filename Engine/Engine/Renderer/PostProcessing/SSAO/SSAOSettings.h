#pragma once

#include "Engine\Reflection\Reflectable.h"

namespace Engine
{
	struct SSAOSettings : Reflectable
	{
		bool myEnabled = false;

		float myIntensity = 1.f;

		void Reflect(Reflector& aReflector) override;
	};
}