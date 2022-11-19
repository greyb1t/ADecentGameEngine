#pragma once

namespace Engine
{
	struct DeferredRenderingStatistics
	{
		int myEnvironmentLightDrawCalls = 0;
		int mySpotLightDrawCalls = 0;
		int myPointLightDrawCalls = 0;

		float myRenderTime = 0.f;

		DeferredRenderingStatistics operator+(const DeferredRenderingStatistics& aOther) const
		{
			DeferredRenderingStatistics result;

			result.myEnvironmentLightDrawCalls = myEnvironmentLightDrawCalls + aOther.myEnvironmentLightDrawCalls;
			result.mySpotLightDrawCalls = mySpotLightDrawCalls + aOther.mySpotLightDrawCalls;
			result.myPointLightDrawCalls = myPointLightDrawCalls + aOther.myPointLightDrawCalls;
			result.myRenderTime = myRenderTime + aOther.myRenderTime;

			return result;
		}
	};

	struct DeferredGBufferStats
	{
		int myMeshDrawCalls = 0;

		float myRenderTime = 0.f;

		DeferredGBufferStats operator+(const DeferredGBufferStats& aOther) const
		{
			DeferredGBufferStats result;

			result.myMeshDrawCalls = myMeshDrawCalls + aOther.myMeshDrawCalls;
			result.myRenderTime = myRenderTime + aOther.myRenderTime;

			return result;
		}
	};

	struct ForwardRenderingStatistics
	{
		int myMeshDrawCalls = 0;

		float myRenderTime = 0.f;

		ForwardRenderingStatistics operator+(const ForwardRenderingStatistics& aOther) const
		{
			ForwardRenderingStatistics result;

			result.myMeshDrawCalls = myMeshDrawCalls + aOther.myMeshDrawCalls;
			result.myRenderTime = myRenderTime + aOther.myRenderTime;

			return result;
		}
	};

	struct TextRendererStatistics
	{
		int myDrawCalls = 0;

		float myRenderTime = 0.f;

		TextRendererStatistics operator+(const TextRendererStatistics& aOther) const
		{
			TextRendererStatistics result;

			result.myDrawCalls = myDrawCalls + aOther.myDrawCalls;
			result.myRenderTime = myRenderTime + aOther.myRenderTime;

			return result;
		}
	};

	struct ShadowRendererStatistics
	{
		int myDrawCalls = 0;

		float myRenderTime = 0.f;

		ShadowRendererStatistics operator+(const ShadowRendererStatistics& aOther) const
		{
			ShadowRendererStatistics result;

			result.myDrawCalls = myDrawCalls + aOther.myDrawCalls;
			result.myRenderTime = myRenderTime + aOther.myRenderTime;

			return result;
		}
	};

	struct RenderingStatistics
	{
		DeferredRenderingStatistics myDeferredRenderingStats;
		ForwardRenderingStatistics myForwardRenderingStats;
		TextRendererStatistics myTextRenderingStats;
		DeferredGBufferStats myGBufferRenderingStats;
		ShadowRendererStatistics myEnvironmentLightShadowRenderStats;
		ShadowRendererStatistics myPointLightShadowRenderStats;
		ShadowRendererStatistics mySpotLightShadowRenderStats;

		RenderingStatistics operator+(const RenderingStatistics& aOther) const
		{
			RenderingStatistics result;

			result.myDeferredRenderingStats = myDeferredRenderingStats + aOther.myDeferredRenderingStats;
			result.myForwardRenderingStats = myForwardRenderingStats + aOther.myForwardRenderingStats;
			result.myTextRenderingStats = myTextRenderingStats + aOther.myTextRenderingStats;
			result.myGBufferRenderingStats = myGBufferRenderingStats + aOther.myGBufferRenderingStats;
			result.myEnvironmentLightShadowRenderStats = myEnvironmentLightShadowRenderStats + aOther.myEnvironmentLightShadowRenderStats;
			result.myPointLightShadowRenderStats = myPointLightShadowRenderStats + aOther.myPointLightShadowRenderStats;
			result.mySpotLightShadowRenderStats = mySpotLightShadowRenderStats + aOther.mySpotLightShadowRenderStats;

			return result;
		}
	};

	struct ScenesRenderingStatistics
	{
		// Each entry is a scene
		std::vector<RenderingStatistics> myScenesRenderingStats;
	};
}