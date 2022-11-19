#pragma once

namespace Engine
{
	// Must match 1:1 with BlendStateStrings
	// DO NOT CHANGE THE ORDER
	enum BlendState
	{
		BlendState_Disable,
		BlendState_AlphaBlend,
		BlendState_AlphaBlend_AlphaToCoverage,
		BlendState_Additive,

		BlendState_NotSet,
		BlendState_Count
	};

	// Must match 1:1 with BlendState
	// DO NOT CHANGE THE ORDER
	const static inline std::string BlendStateStrings[BlendState_Count] =
	{
		"Disable",
		"AlphaBlend",
		"AlphaBlend_AlphaToCoverage",
		"Additive",
		"Not Set",
	};

	// Must match 1:1 with DepthStencilStateStrings
	// DO NOT CHANGE THE ORDER
	enum DepthStencilState
	{
		DepthStencilState_Default,
		DepthStencilState_ReadOnly,
		DepthStencilState_StencilAlwaysWithDepthTest,
		DepthStencilState_StencilNotEqualWithDepthTest,

		DepthStencilState_PointlightPass1,
		DepthStencilState_PointlightPass2,

		DepthStencilState_Skybox,

		DepthStencilState_NotSet,
		DepthStencilState_Count
	};

	// Must match 1:1 with DepthStencilState
	// DO NOT CHANGE THE ORDER
	const static inline std::string DepthStencilStateStrings[DepthStencilState_Count] =
	{
		"Default",
		"ReadOnly",
		"StencilAlwaysWithDepthTest",
		"StencilNotEqualWithDepthTest",
		"PointlightPass1",
		"PointlightPass2",
		"Skybox"
		"Not Set"
	};

	// Must match 1:1 with RasterizerStateStrings
	// DO NOT CHANGE THE ORDER
	enum RasterizerState
	{
		RasterizerState_CW, // This is default
		RasterizerState_CCW,
		RasterizerState_NoCulling,
		RasterizerState_Wireframe,

		// Contains depth bias, slope to improve on shadow acne
		RasterizerState_ShadowCastersCWDepthBias,

		RasterizerState_PointlightPass1,
		RasterizerState_PointlightPass2,

		RasterizerState_NotSet,
		RasterizerState_Count
	};

	// Must match 1:1 with RasterizerState
	// DO NOT CHANGE THE ORDER
	const static inline std::string RasterizerStateStrings[RasterizerState_Count] =
	{
		"CW",
		"CCW",
		"NoCulling",
		"Wireframe",

		"ShadowCastersCWDepthBias",

		"PointlightPass1",
		"PointlightPass2",
		"Not Set"
	};

	// Must match 1:1 with SampleStateStrings
	// DO NOT CHANGE THE ORDER
	enum SamplerState
	{
		SamplerState_Trilinear,
		SamplerState_TrilinearWrap,
		SamplerState_Point,
		SamplerState_TrilinearClamp,
		SamplerState_ShadowSamplerPCF,
		SamplerState_Count
	};

	// Must match 1:1 with SamplerState
	// DO NOT CHANGE THE ORDER
	const static inline std::string SampleStateStrings[SamplerState_Count] =
	{
		"Trilinear",
		"TrilinearWrap",
		"Point",
		"TrilinearClamp",
		"ShadowSamplerPCF",
	};
}