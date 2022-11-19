#pragma once

#include "CommonUtilities.h"
#include "Math/Math.h"

namespace UnrealUtils
{
	//Converts from Unreal Coords to our own
	Common::Vector3f ConvertPosition(CU::Vector3f aUnrealVector);
}
