#pragma once

#include "Engine/Utils/Flags.h"

namespace Engine
{
	enum ShaderFlags
	{
		ShaderFlags_None = 0,

		ShaderFlags_VertexShader = 1 << 0,
		ShaderFlags_PixelShader = 1 << 1,
		ShaderFlags_GeometryShader = 1 << 2
	};

	CREATE_FLAG_OF_ENUM(ShaderFlags);
}