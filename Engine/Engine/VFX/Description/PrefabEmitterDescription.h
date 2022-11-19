#pragma once

#include "EmitterDescription.h"
#include "PrefabDescription.h"

namespace VFX
{
	struct PrefabEmitterDescription
	{
		EmitterDescription base;
		PrefabDescription PrefabDescription;
	};
}
