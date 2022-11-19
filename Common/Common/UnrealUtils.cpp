#include "UnrealUtils.h"

Common::Vector3f UnrealUtils::ConvertPosition(CU::Vector3f aUnrealVector)
{
	return { aUnrealVector.x, aUnrealVector.z, -aUnrealVector.y };
}
