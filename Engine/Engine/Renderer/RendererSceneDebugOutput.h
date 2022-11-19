#pragma once

namespace Engine
{
	enum class RendererSceneDebugOutput
	{
		Nothing,

		Position,
		Albedo,
		Normal,
		VertexNormal,

		Material,
		Metalness, // Just the material but the R value as grayscale
		Roughness, // Just the material but the G value as grayscale
		Emissive, // Just the material but the B value as grayscale

		LinearDepth,

		AmbientOcclusion,

		SSAO,
		SSAOBlurred,

		Count
	};

	inline const std::string RendererSceneDebugOutputStrings[] =
	{
		"Nothing",

		"Position",
		"Albedo",
		"Normal",
		"VertexNormal",

		"Material",
		"Metalness",
		"Roughness",
		"Emissive",

		"LinearDepth",

		"AmbientOcclusion",

		"SSAO",
		"SSAOBlurred"
	};
}