#pragma once

namespace Engine
{
	struct ExtraSceneJsonData
	{
		std::string defaultCubemapPath = "Assets\\Engine/PlaceholderTextures\\PlaceholderCubemap.dds";
		std::string defaultFogPresetName = "Default";

		Vec3f cubemapColor = Vec3f(0.035f, 0.15f, 0.337);

		float cubemapColorIntensity = 1.f;
		float cubemapIntensity = 1.f;

		float shadowStrength = 1.f;

		bool InitFromJson(const Path& aExtraJsonPath);
	};
}