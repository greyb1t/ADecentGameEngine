#pragma once

namespace Engine
{
	enum BindEffectFlags : uint32_t
	{
		BindEffectFlags_None = 0,

		BindEffectFlags_PixelShader = 1 << 0,
		BindEffectFlags_VertexShader = 1 << 1,
		BindEffectFlags_GeometryShader = 1 << 2,
		BindEffectFlags_EffectSpecificStuff = 1 << 3
	};

	enum BindModelFlags : uint32_t
	{
		BindModelFlags_None = 0,
		BindModelFlags_Textures = 1 << 0,
	};

	CREATE_FLAG_OF_ENUM(BindEffectFlags);
	CREATE_FLAG_OF_ENUM(BindModelFlags);
}