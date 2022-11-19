#pragma once

#include "Engine\Reflection\Reflectable.h"

namespace Engine
{
	struct FXAASettings : Reflectable
	{
		void Reflect(Reflector& aReflector) override;

		bool myEnabled = true;
	};
}