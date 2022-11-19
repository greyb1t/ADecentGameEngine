#pragma once

#include "Engine/Utils/Flags.h"

struct ID3D11ShaderReflection;

namespace Engine
{
	enum class ShaderDefines
	{
		None,
		Instanced,

		Count
	};

	// Must line up with ShaderDefines enum
	const std::string ShaderDefinesStrings[]
	{
		"",
		"INSTANCED"
	};

	CREATE_FLAG_OF_ENUM(ShaderDefines);

	inline int OrDefines(const std::vector<ShaderDefines>& aDefines)
	{
		ShaderDefines result = ShaderDefines::None;

		for (const auto& define : aDefines)
		{
			result |= define;
		}

		return static_cast<int>(result);
	}

	class Shader
	{
	public:

		ID3D11ShaderReflection* GetReflection() const;

	protected:
		// Solely for the reason to be able to FileWatch them
		// std::vector<std::string> myIncludedFilePaths;

		ComPtr<ID3D11ShaderReflection> myReflection = nullptr;
	};
}