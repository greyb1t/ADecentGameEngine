#pragma once

#include "Fog.h"

class JsonManager;

namespace Engine
{
	class FogPresets;

	class FogManager
	{
	public:
		bool Init(JsonManager& aJsonManager);

		const FogPresets& GetPresets();

	private:
		const FogPresets* myPresets = nullptr;
	};
}