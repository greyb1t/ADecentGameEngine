#pragma once

#include "Fog\Fog.h"
#include "TonemappingSettings.h"
#include "Bloom/BloomSettings.h"
#include "SSAO/SSAOSettings.h"
#include "FXAASettings.h"

namespace Engine
{
	struct PostProcessData
	{
		bool myFogEnabled = true;
		FogData myFogData;

		FXAASettings myFXAASettings;

		BloomSettings2 myBloomSettings;

		TonemappingSettings myTonemappingData;

		SSAOSettings mySSAOSettings;
	};
}