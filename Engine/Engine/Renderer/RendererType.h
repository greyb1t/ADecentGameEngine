#pragma once

namespace Engine
{
	enum class RendererType
	{
		Forward,
		Deferred,

		Count
	};

	const static inline std::string RendererTypeStrings[] =
	{ 
		"Forward",
		"Deferred",
	};
}