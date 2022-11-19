#include "pch.h"
#include "RenderManager.h"
#include "Directx11Framework.h"
#include "Engine/Renderer/Texture/FullscreenTextureFactory.h"
#include "WindowHandler.h"
#include "Engine/Renderer/Scene/RendererScene.h"
#include "Model/ModelInstance.h"
#include "Camera/Camera.h"
#include "Renderers/DeferredRenderer.h"
#include "Renderers/DebugRenderer.h"
#include "Renderers/ParticleRenderer.h"
#include "Renderers/SpriteRenderer.h"
#include "Renderers/ShadowRenderer.h"
#include "Text/TextRenderer.h"
#include "Renderers/ForwardRenderer.h"
#include "Utils/DxUtils.h"
#include "FrameConstantBuffer.h"
#include "Engine/Engine.h"
#include "Engine/DebugManager/DebugDrawer.h"
#include "Statistics/RenderingStatistics.h"
#include "Engine/Console/CommandHandler.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/MeshComponent.h"
#include "Engine/GameObject/Components/PointLightComponent.h"
#include "Engine/GameObject/Components/SpotLightComponent.h"
#include "Renderers/CascadedShadows.h"
#include "Engine/Scene/Scene.h"
#include "Engine/GameObject/Components/EnvironmentLightComponent.h"
#include "PostProcessing/FullscreenRenderer.h"
#include "TracyProfilingGPU.h"
#include "PostProcessing/SSAO/SSAO.h"
#include "Engine/ResourceManagement/Resources/TextureResource.h"
#include "Engine/ResourceManagement/ResourceManager.h"
#include "Engine/Renderer/Texture/Texture2D.h"
#include "Engine/Paths.h"
#include "Renderers/SelectionRenderer.h"
#include "Engine/Renderer/Scene/CulledSceneData.h"
#include "Engine/GameObject/Components/CameraComponent.h"
#include "Text/Text.h"
#include "../GameObject/Components/SpriteComponent.h"

namespace Engine
{
	RenderManager::RenderManager(
		ResourceManager& aResourceManager,
		ResourceReferences& aResourceRefs,
		FullscreenTextureFactory& aFullscreenTextureFactory,
		WindowHandler& aWindowHandler,
		Directx11Framework& aFramework)
		: myResourceManager(aResourceManager),
		myResourceRefs(aResourceRefs),
		myFullscreenTextureFactory(aFullscreenTextureFactory),
		myWindowHandler(aWindowHandler),
		myFramework(aFramework)
	{
		myWindowHandler.AddObserver(EventType::WindowResized, this);
	}

	RenderManager::~RenderManager()
	{
	}

	bool RenderManager::Init(Directx11Framework& aFramework)
	{
		myFrameBuffer = MakeOwned<FrameBufferTempName>();

		if (!myFrameBuffer->Init(aFramework))
		{
			return false;
		}

		if (!mySharedRendererData.Init(aFramework))
		{
			return false;
		}

		if (!InitBlendStates(aFramework))
		{
			return false;
		}

		if (!InitDepthStencilStates(aFramework))
		{
			return false;
		}

		if (!InitRasterizerStates(aFramework))
		{
			return false;
		}

		if (!InitSamplerStates(aFramework))
		{
			return false;
		}

		myForwardRenderer = MakeOwned<ForwardRenderer>(aFramework, *myFrameBuffer, mySharedRendererData);

		if (!myForwardRenderer->Init())
		{
			return false;
		}

		myFullscreenRenderer = MakeOwned<FullscreenRenderer>(
			myResourceManager,
			myFullscreenTextureFactory,
			myWindowHandler);

		if (!myFullscreenRenderer->Init(aFramework.GetDevice(), aFramework.GetContext()))
		{
			return false;
		}

		myDeferredRenderer = MakeOwned<DeferredRenderer>(
			aFramework,
			myResourceManager,
			myResourceRefs,
			*myFrameBuffer,
			myFullscreenRenderer->GetFullscreenVertexShader(),
			mySharedRendererData);

		if (!myDeferredRenderer->Init())
		{
			return false;
		}

		myShadowRenderer = MakeOwned<ShadowRenderer>(aFramework, *myFrameBuffer);

		if (!myShadowRenderer->Init(myResourceManager))
		{
			return false;
		}

		//myCascadedShadows = MakeOwned<CascadedShadows>();
		//
		//if (!myCascadedShadows->Init())
		//{
		//	return false;
		//}

		myParticleRenderer = MakeOwned<ParticleRenderer>(aFramework, *myFrameBuffer);

		if (!myParticleRenderer->Init())
		{
			return false;
		}

		mySpriteRenderer = MakeOwned<SpriteRenderer>();

		if (!mySpriteRenderer->Init(myResourceManager, aFramework, myWindowHandler))
		{
			return false;
		}

		myTextRenderer = MakeOwned<TextRenderer>(myResourceManager, aFramework);

		if (!myTextRenderer->Init())
		{
			return false;
		}

		myDebugRenderer = MakeOwned<DebugRenderer>(myResourceRefs, *myFrameBuffer, aFramework, myWindowHandler);

		if (!myDebugRenderer->Init())
		{
			return false;
		}

		mySelectionRenderer = MakeOwned<SelectionRenderer>(aFramework, *myFrameBuffer, mySharedRendererData);

		if (!mySelectionRenderer->Init())
		{
			return false;
		}

		CreateTextures();

		return true;
	}

	//ScenesRenderingStats RenderManager::Render(RendererScene& aScene, RendererScene* /aUIScene)
	//{
	//	ScenesRenderingStats stats;
	//
	//	Camera* mainCamera = aScene.GetMainCamera();
	//
	//	const auto sceneRenderStats = RenderScene(aScene, *mainCamera, false);
	//	stats.myScenesRenderingStats.push_back(sceneRenderStats);
	//
	//	return stats;
	//}

	void RenderManager::EndRender(RendererScene& aScene)
	{
		// NOTE(filip): Draw the result to the scene texture
		// At same time, Gamma Corrects the by turning it into Gamma colors
		// to display on screen
		// Why?
		// Because the proper solution would be to have the backbuffer in *_SRGB
		// format, but, when drawing ImGui, it gets washed out when drawing the
		// imgui to the backbuffer
		// That's why we manually correct gamma
		// TODO: In RETAIL, consider changing the backbuffer to *_SRGB
		// because we do not need ImGui there
		{
			auto& targetTexture = aScene.GetTextureToRenderTo();
			targetTexture.ClearTexture();

			targetTexture.SetAsActiveTarget();
			myDeferredTexture2.SetAsResourceOnSlot(0);
			myFullscreenRenderer->Render(FullscreenRenderer::Shader::GammaCorrection/*Copy*/);
			DxUtils::UnbindShaderResourceView(*myFramework.GetContext(), 0);
		}

		{
			auto& selectionTexture = aScene.GetSelectionTexture();
			selectionTexture.SetAsActiveTarget();
			selectionTexture.ClearTexture();

			mySelectionTexture.SetAsResourceOnSlot(0);
			myFullscreenRenderer->Render(FullscreenRenderer::Shader::Copy);
			DxUtils::UnbindShaderResourceView(*myFramework.GetContext(), 0);
		}
	}

	void RenderManager::EndRender2()
	{
		/*
				auto& targetTexture = aScene.GetTextureToRenderTo();
				targetTexture.ClearTexture();

				targetTexture.SetAsActiveTarget();
				myDeferredTexture2.SetAsResourceOnSlot(0);
				myFullscreenRenderer->Render(FullscreenRenderer::Shader::GammaCorrection);
				DxUtils::UnbindShaderResourceView(*myFramework.GetContext(), 0);
		*/

		myDeferredTexture2.SetAsResourceOnSlot(0);
		myFullscreenRenderer->Render(FullscreenRenderer::Shader::GammaCorrection);
		DxUtils::UnbindShaderResourceView(*myFramework.GetContext(), 0);
	}

	RenderingStatistics RenderManager::RenderScene(
		RendererScene& aScene,
		const Camera& aCamera,
		const bool aUI,
		const CulledSceneData& aCulledSceneData)
	{
		GPU_ZONE_NAMED(mainRenderSceneZone, "RenderScene");
		ZoneNamedN(zone1, "RenderManager::RenderScene", true);

		RenderingStatistics stats;

		// Camera* mainCamera = aScene.GetMainCamera();

		// TODO: Rename myDeferredTexture myActiveTexture?
		myDepthTexture.ClearDepth();
		myDepthBufferCopy.ClearTexture();
		myDepthBufferCopy2.ClearTexture();
		//myGBuffer.ClearTextures();
		myDeferredTexture.ClearTexture({ 0.f, 0, 0, 0, });
		myDeferredTexture2.ClearTexture({ 0.f, 0.f, 0.f, 0.f });

		myTempFogTexture.ClearTexture();

		myDebugResultTexture.ClearTexture();

		myFullscreenRenderer->ClearTextures();

		if (mySSAO)
		{
			mySSAO->ClearTextures();
		}

		// myIntermediateTexture.ClearTexture();
		//myIntermediateDepth.ClearDepth();
		myGBuffer.ClearTextures();
		//myDeferredTexture.ClearTexture();

		//myFullscreenRenderer->ClearTextures();


		if (!aScene.CanRender())
		{
			auto missingCubemapTexture = GResourceManager->CreateRef<TextureResource>(GetEngine().GetPaths().GetPathString(PathEnum::MissingCubemapTexture));
			missingCubemapTexture->Load();

			myDeferredTexture2.SetAsActiveTarget();

			myFramework.GetContext()->PSSetShaderResources(0, 1, missingCubemapTexture->Get().GetSRVPtr());

			SetBlendState(BlendState_Disable);
			SetSamplerState(0, SamplerState_Trilinear);
			SetRasterizerState(RasterizerState_CW);

			myFullscreenRenderer->Render(FullscreenRenderer::Shader::Copy);

			DxUtils::UnbindRenderTargetView(*myFramework.GetContext());

			return {};
		}

		// Update/Bind the shared things among all shaders
		{
			mySharedRendererData.UpdateDataAndBind();

			// Bind the samplers
			// Starts at slot 10
			SetSamplerState(10, SamplerState_TrilinearWrap);
			SetSamplerState(11, SamplerState_Point);
			SetSamplerState(12, SamplerState_TrilinearClamp);
		}

		SetBlendState(BlendState_Disable);

		SetRasterizerState(RasterizerState_CW);

		// myIntermediateTexture.SetAsActiveTarget(&myIntermediateDepth);

		EnvironmentLightComponent* environmentLight = aScene.GetEnvironmentalLight();
		assert(environmentLight);

		myFrameBuffer->UpdateEnvironmentLight(*environmentLight);
		myFrameBuffer->UpdateCamera(aCamera);
		myFrameBuffer->UpdateFog(aScene.GetPostProcessData().myFogData);
		myFrameBuffer->UpdateShadowStrength(aScene.GetShadowStrength());
		myFrameBuffer->UpdatePlayerPos(aScene.GetPlayerPos());
		myFrameBuffer->Bind();

		//CulledSceneData culledSceneData = aScene.CullEverything(aCamera);
		// TODO: do not make copy?
		//CulledSceneData culledSceneData = aScene.GetCulledSceneData();
		// TODO: COPY OMG omg om
		CulledSceneData culledSceneData = aCulledSceneData;
		const auto& renderCommandChain = aScene.GetRenderCommandChain();

		/*
		SetDepthStencilState(DepthStencilState_Default);

		//DepthPrePass(renderCommandChain, myDepthTexture);
		// Depth prepass
		{
			//myDepthTexture.ClearDepth(); // already done above
			myDepthTexture.SetAsActiveDepth();

			// Render opaque objects as prepass to occlude objects behind them that
			// wont be visible
			myShadowRenderer->Render(aCamera, culledSceneData.myDeferredMeshes);
		}

		// Must set to read only after the depth prepass
		SetDepthStencilState(DepthStencilState_ReadOnly);
		*/

		// TODO: We are currently sorting by the parent ModelInstance
		// if a ModelInstance has multiple transparent meshes, this 
		// will not work, I need to sort by the position offseted by local mesh position as well
		// Sort the model instances front to back because they are all opaque since being used by deferred renderer
		// NOTE(Filip): I removed this because when I tried 
		// instancing 6000 cubes, this was VERY SLOW
		// when adding instancing, I think its better to do
		// a depth prepass to avoid the overdraw instead
		// for performance
		/*
		std::sort(
			culledSceneData.myDeferredMeshes.begin(),
			culledSceneData.myDeferredMeshes.end(),
			[&aCamera](CulledMeshInstance& aLeft, CulledMeshInstance& aRight)
			{
				const float distSqToLeft =
					(aLeft.myMeshInstance->GetModelInstance()->GetTransformMatrix().GetTranslation() -
						aCamera.GetTransform().GetPosition()).LengthSqr();

				const float distSqToRight =
					(aRight.myMeshInstance->GetModelInstance()->GetTransformMatrix().GetTranslation() -
						aCamera.GetTransform().GetPosition()).LengthSqr();

				return distSqToLeft < distSqToRight;
			});
		*/

		{
			ZoneNamedN(zone2, "RenderManager::RenderScene SortDeferredMeshComponents", true);

			std::sort(
				culledSceneData.myDeferredMeshComponents.begin(),
				culledSceneData.myDeferredMeshComponents.end(),
				[&aCamera](const CulledMeshComponent& aLeft, const CulledMeshComponent& aRight)
				{
					const float distSqToLeft =
						(aLeft.myMeshComponent->GetGameObject()->GetTransform().GetPosition() -
							aCamera.GetTransform().GetPosition()).LengthSqr();

					const float distSqToRight =
						(aRight.myMeshComponent->GetGameObject()->GetTransform().GetPosition() -
							aCamera.GetTransform().GetPosition()).LengthSqr();

					return distSqToLeft < distSqToRight;
				});
		}

		SetSamplerState(0, SamplerState_TrilinearWrap);

		{
			// Must use a bias and slope scale bias to fix shadow acne
			SetRasterizerState(RasterizerState_ShadowCastersCWDepthBias);

			// Render shadows maps for environment light
			if (environmentLight->IsCastingShadows())
			{
				// CASCADED SHADOW MAP TEST
#if 0
				myCascadedShadows->Render(aCamera, *environmentLight);
#else
				Camera& shadowCamera = environmentLight->GetCamera();
				FullscreenTexture& shadowMap = environmentLight->GetShadowMap();
				// const auto shadowCasters = aScene.CullModels(shadowCamera);

				aScene.GetDebugDrawer().DrawFrustumOrthographic(
					DebugDrawFlags::EnvironmentLight,
					environmentLight->GetGameObject()->GetTransform().GetPosition(),
					shadowCamera.GetViewFrustum(),
					0.f,
					{ 1, 0, 0, 1 },
					false);

				if (!culledSceneData.myEnvironmentLightShadowCastersMeshes.empty())
				{
					GPU_ZONE_NAMED(envLightShadowZone, "Environment Light Shadows");

					//shadowMap.ClearDepth();
					//shadowMap.SetAsActiveDepth();

					stats.myEnvironmentLightShadowRenderStats = myShadowRenderer->Render(environmentLight, culledSceneData.myEnvironmentLightShadowCastersMeshes);
				}
#endif
			}

			// render shadows map for spotlights
			{
				// Culling for the lights
				// First we cull the models in our camera, then we cull the models we just culled from the light camera
				// the result is models that we and the light sees

				GPU_ZONE_NAMED(spotLightShadowsZone, "Spot Lights Shadows");

				ShadowRendererStatistics spotlightsShadowRenderStats;

				for (auto& spotlightCommand : renderCommandChain.mySpotlightCommands)
					//for (auto& spotlightCommand : culledSceneData.frustumCulledSpotLights)
				{
					auto spotlight = spotlightCommand.mySpotLight;

					if (spotlight->IsCastingShadows())
					{
						Camera& shadowCamera = spotlight->GetCamera();
						FullscreenTexture& shadowMap = spotlight->GetShadowMap();

						shadowMap.ClearDepth();
						shadowMap.SetAsActiveDepth();

						//const auto s = myShadowRenderer->Render(
						//	shadowCamera,
						//	spotlightCommand.mySpotLightShadowCastersMeshes);

						myShadowRenderer->Render(
							shadowCamera,
							spotlightCommand.myShadowCasters);

						// spotlightsShadowRenderStats = spotlightsShadowRenderStats + s;
					}
				}

				stats.mySpotLightShadowRenderStats = spotlightsShadowRenderStats;
			}

			// render shadows map for pointlights
			{
				GPU_ZONE_NAMED(pointLightShadowsZone, "Point Lights Shadows");

				ShadowRendererStatistics pointlightsShadowRenderStats;

				for (auto& pointLightCommand : renderCommandChain.myPointLightCommands)
					//for (auto& pointLightCommand : culledSceneData.frustumCulledPointLights)
				{
					auto pointlight = pointLightCommand.myPointLight;

					if (pointlight->IsCastingShadows())
					{
						FullscreenTextureCube* shadowMap = pointlight->GetShadowMap();

						// Clear all sides, using the texturecube depth stencil view
						shadowMap->ClearDepthWholeCube();

						// DRAW POINT LIGHT USING 1 DRAWCALL, IT SLOWER BECAUSE WE
						// ARE GPU BOUND ATM
#if 0
					// Set the whole cube as a target
						shadowMap->SetAsActiveDepth();

						const auto s = myShadowRenderer->Render2(
							*pointlight,
							pointLightCullData.myNewWholeSceneAllDirectionsShadowCastersMeshes);

						pointlightsShadowRenderStats = pointlightsShadowRenderStats + s;
#else

					// For each of the cube's sides, render the shadow map
						for (int i = 0; i < 6; ++i)
						{
							// Render to each cube side using a depth stencil view for each side
							shadowMap->SetAsActiveDepthSide(i);

							const auto& shadowCamera = pointlight->GetCameras()[i];

							//if (!shadowCasters.empty())

							//const auto s = myShadowRenderer->Render(
							//	shadowCamera,
							//	pointLightCommand.myPointLightShadowCastersMeshes[i]);
							//pointlightsShadowRenderStats = pointlightsShadowRenderStats + s;

							myShadowRenderer->Render(
								shadowCamera,
								pointLightCommand.myShadowCasters[i]);
						}
#endif
					}
				}

				stats.myPointLightShadowRenderStats = pointlightsShadowRenderStats;
			}
		}

		SetRasterizerState(RasterizerState_CW);

		// if (!culledSceneData.myDeferredMeshes.empty())
		{
			GPU_ZONE_NAMED(gbufferZone, "Generate GBuffer");

			SetSamplerState(0, SamplerState_TrilinearWrap);
			myGBuffer.SetAsActiveTarget(&myDepthTexture);
			stats.myGBufferRenderingStats = myDeferredRenderer->GenerateGBuffer(
				&aCamera,
				culledSceneData.myDeferredMeshes,
				culledSceneData.myDeferredMeshComponents,
				renderCommandChain);
		}

#if 1
		// Copy the depth buffer because we need it when drawing the decals
		myDepthBufferCopy.SetAsActiveTarget();
		myDepthTexture.SetAsResourceOnSlot(0);
		myFullscreenRenderer->Render(FullscreenRenderer::Shader::CopyDepth);

		// Drawing the decals by rendering directly into the gbuffer
		// I must access the world position texture in the gbuffer, but write to albedo, normal and material
		// Therefore a mix has to occur by reading and writing to the gbuffer textures

		DxUtils::UnbindRenderTargetView(*myFramework.GetContext());
		// unbind depth
		DxUtils::UnbindShaderResourceView(*myFramework.GetContext(), 0);

		// Set world texture to resource (to read)
		myGBuffer.SetAsResourceOnSlot(GBuffer::GBufferTexture::Position, 5);

		// 18 is DecalStructs.hlsli
		myDepthBufferCopy.SetAsResourceOnSlot(18);
		//myGBuffer.SetAsResourceOnSlot(GBuffer::GBufferTexture::Depth, 6);

		// hur fan ska jag l�sa och skriva normal textur?
		// KAN INTE JUST NU, K�R BARA ALBEDO I B�RJAN

		// Must use alpha blend to make transparent decals or decals with low opacity
		SetBlendState(BlendState_AlphaBlend);

		ID3D11RenderTargetView* decalRenderTargets[]
		{
			myGBuffer.GetRenderTarget(GBuffer::Albedo), // TARGET 0
			myGBuffer.GetRenderTarget(GBuffer::Material), // TARGET 1
			myGBuffer.GetRenderTarget(GBuffer::Normal) // TARGET 2
		};

		// Set albedo, normal, material as render target (to write)
		myFramework.GetContext()->OMSetRenderTargets(
			std::size(decalRenderTargets), &decalRenderTargets[0], myDepthTexture.GetDepth());

		// Must disable write to depth because otherwise background will be black and fog not drawn
		// because the cube we draw is supposed to be invisible
		SetDepthStencilState(DepthStencilState_ReadOnly);
		// Draw decals before light to have the lights affect them teet
		if (!culledSceneData.myDeferredDecalComponents.empty())
		{
			GPU_ZONE_NAMED(decalsZone, "Decals");

			myDeferredRenderer->RenderDecals(&aCamera, culledSceneData.myDeferredDecalComponents);
		}
		SetDepthStencilState(DepthStencilState_Default);

		// unbind the depth copy
		DxUtils::UnbindShaderResourceView(*myFramework.GetContext(), 18);

		// is this needed?
		DxUtils::UnbindRenderTargetView(*myFramework.GetContext());

		SetBlendState(BlendState_Disable);
#endif // 0

		if (aScene.GetRendererDebugOutput() != RendererSceneDebugOutput::Nothing)
		{
			aScene.GetDebugGBufferTexture().ClearTexture();

			aScene.GetDebugGBufferTexture().SetAsActiveTarget();

			switch (aScene.GetRendererDebugOutput())
			{
			case RendererSceneDebugOutput::Position:
				myGBuffer.SetAsResourceOnSlot(GBuffer::GBufferTexture::Position, 0);
				myFullscreenRenderer->Render(FullscreenRenderer::Shader::Copy);
				break;
			case RendererSceneDebugOutput::Albedo:
				myGBuffer.SetAsResourceOnSlot(GBuffer::GBufferTexture::Albedo, 0);
				myFullscreenRenderer->Render(FullscreenRenderer::Shader::Copy);
				break;
			case RendererSceneDebugOutput::Normal:
				myGBuffer.SetAsResourceOnSlot(GBuffer::GBufferTexture::Normal, 0);
				myFullscreenRenderer->Render(FullscreenRenderer::Shader::Copy);
				break;
			case RendererSceneDebugOutput::VertexNormal:
				myGBuffer.SetAsResourceOnSlot(GBuffer::GBufferTexture::VertexNormal, 0);
				myFullscreenRenderer->Render(FullscreenRenderer::Shader::Copy);
				break;
			case RendererSceneDebugOutput::Material:
				myGBuffer.SetAsResourceOnSlot(GBuffer::GBufferTexture::Material, 0);
				myFullscreenRenderer->Render(FullscreenRenderer::Shader::Copy);
				break;
			case RendererSceneDebugOutput::AmbientOcclusion:
				myGBuffer.SetAsResourceOnSlot(GBuffer::GBufferTexture::AmbientOcclusion, 0);
				myFullscreenRenderer->Render(FullscreenRenderer::Shader::Copy);
				break;
			case RendererSceneDebugOutput::Metalness:
				myGBuffer.SetAsResourceOnSlot(GBuffer::GBufferTexture::Material, 0);
				myFullscreenRenderer->Render(FullscreenRenderer::Shader::CopyR);
				break;
			case RendererSceneDebugOutput::Roughness:
				myGBuffer.SetAsResourceOnSlot(GBuffer::GBufferTexture::Material, 0);
				myFullscreenRenderer->Render(FullscreenRenderer::Shader::CopyG);
				break;
			case RendererSceneDebugOutput::Emissive:
				myGBuffer.SetAsResourceOnSlot(GBuffer::GBufferTexture::Material, 0);
				myFullscreenRenderer->Render(FullscreenRenderer::Shader::CopyB);
				break;
			case RendererSceneDebugOutput::LinearDepth:
				myGBuffer.SetAsResourceOnSlot(GBuffer::GBufferTexture::Depth, 0);
				myFullscreenRenderer->Render(FullscreenRenderer::Shader::CopyR);
				break;
			default:
				break;
			}
		}

		// SSAO
		// must do the ssao pass before this, because
		// this pass needs it in the environemnt light shader
		{
			// NOTE(filip): SSAO is always enabled otherwise we need an if statement
			// in the deferred environment light shader
			//if (aScene.GetPostProcessData().mySSAOSettings.myEnabled)
			{
				GPU_ZONE_NAMED(ssaoZone, "SSAO");

				if (mySSAO == nullptr)
				{
					mySSAO = MakeOwned<SSAO>(
						myFullscreenTextureFactory,
						myWindowHandler);

					mySSAO->Init(*myFramework.GetDevice());
				}

				assert(mySSAO);

				SetSamplerState(0, SamplerState_TrilinearClamp);
				SetSamplerState(1, SamplerState_TrilinearWrap);

				myDepthTexture.SetAsResourceOnSlot(4);

				mySSAO->Render(
					myGBuffer,
					*myFramework.GetContext(),
					*myFullscreenRenderer);

				DxUtils::UnbindRenderTargetView(*myFramework.GetContext());

				if (aScene.GetRendererDebugOutput() == RendererSceneDebugOutput::SSAO)
				{
					aScene.GetDebugGBufferTexture().ClearTexture();

					aScene.GetDebugGBufferTexture().SetAsActiveTarget();

					mySSAO->GetResultTexture().SetAsResourceOnSlot(0);
					myFullscreenRenderer->Render(FullscreenRenderer::Shader::CopyR);
				}
				else if (aScene.GetRendererDebugOutput() == RendererSceneDebugOutput::SSAOBlurred)
				{
					aScene.GetDebugGBufferTexture().ClearTexture();

					aScene.GetDebugGBufferTexture().SetAsActiveTarget();

					mySSAO->GetResultTextureBlurred().SetAsResourceOnSlot(0);
					myFullscreenRenderer->Render(FullscreenRenderer::Shader::CopyR);
				}
			}
			// else
			// {
			// 	mySSAO = nullptr;
			// }
		}

		{
			GPU_ZONE_NAMED(lightsZone, "Deferred Lights");

			myGBuffer.SetAllAsResources();

			if (mySSAO)
			{
				mySSAO->GetResultTextureBlurred().SetAsResourceOnSlot(39);
			}

			myDeferredTexture.SetAsActiveTarget();

			SetBlendState(BlendState_Additive);

			SetSamplerState(0, SamplerState_TrilinearWrap);
			SetSamplerState(1, SamplerState_Point);
			SetSamplerState(2, SamplerState_ShadowSamplerPCF);

			// Convert the culled data to a raw pointlights vector 
			// because DeferredRenderer wants it like that atm, not best solution but whateever..
			std::vector<PointLightComponent*> pointLights;

			for (const auto& culledPointLightData : culledSceneData.frustumCulledPointLights)
			{
				pointLights.push_back(culledPointLightData.myPointLight);
			}

			std::vector<SpotLightComponent*> spotlights;

			for (const auto& culledSpotLightData : culledSceneData.frustumCulledSpotLights)
			{
				spotlights.push_back(culledSpotLightData.mySpotLight);
			}

			//SetRasterizerState(RasterizerState_NoCulling);

			stats.myDeferredRenderingStats = myDeferredRenderer->Render(
				environmentLight,
				pointLights,
				spotlights,
				&aCamera);

#if 1
			myFramework.GetContext()->OMSetRenderTargets(
				1,
				myDeferredTexture.GetRenderTargetTestPtr(),
				myReadOnlyDepthStencilView.Get());

			stats.myDeferredRenderingStats = stats.myDeferredRenderingStats + myDeferredRenderer->RenderPointLights(
				pointLights,
				aCamera,
				*this);

			stats.myDeferredRenderingStats = stats.myDeferredRenderingStats + myDeferredRenderer->RenderSpotLights(
				spotlights,
				aCamera,
				*this);

			// Reset states
			SetRasterizerState(RasterizerState_CW);
			SetBlendState(BlendState_Additive); // probably not needed
			SetDepthStencilState(DepthStencilState_Default);

			if (mySSAO)
			{
				// unbind the ssao render SRV
				DxUtils::UnbindShaderResourceView(*myFramework.GetContext(), 39);
			}

			myDeferredTexture.SetAsActiveTarget();
#endif

			//SetRasterizerState(RasterizerState_CW);
		}

		myGBuffer.UnbindAllResources();

		// Apply the fog to deferred stuff ONLY, Why?
		// Bcuz there is an issue when rendering transparent forward stuff
		// and applying the fog after that since transparent pixels also get written to depth buffer
		// therefore, fog would not apply properly to transparent objects.
		// Unity also only does Deferred Fog for deferred stuff only.
		if (!aUI)
		{
			GPU_ZONE_NAMED(deferredFogZone, "Deferred Fog");

			ZoneNamedN(zone3, "RenderManager::RenderScene Fog", true);

			// Set output texture
			myTempFogTexture.SetAsActiveTarget();

			myDeferredTexture.SetAsResourceOnSlot(0);
			myDepthTexture.SetAsResourceOnSlot(1);
			myGBuffer.SetAsResourceOnSlot(GBuffer::GBufferTexture::Position, 2);
			myFullscreenRenderer->Render(FullscreenRenderer::Shader::Fog);

			// unbind myTempFogTexture
			DxUtils::UnbindRenderTargetView(*myFramework.GetContext());

			// unbind myDeferredTexture
			DxUtils::UnbindShaderResourceView(*myFramework.GetContext(), 0);
			// unbind myIntermediateDepth
			DxUtils::UnbindShaderResourceView(*myFramework.GetContext(), 1);

			DxUtils::UnbindShaderResourceView(*myFramework.GetContext(), 2);

			myDeferredTexture.ClearTexture();

			myDeferredTexture.SetAsActiveTarget();
			myTempFogTexture.SetAsResourceOnSlot(0);
			myFullscreenRenderer->Render(FullscreenRenderer::Shader::/*GammaCorrection*/Copy);

			// NOTE(filip): Removed the below Gamma Correction because we
			// now use SRGB textures and backbuffer format to let the GPU handle it
			// A LOT nicer for me

			////// Apply the gamma correction as a post process effect on ALL of the deferred
			////// stuff. Why?
			////// The reason we need it as a post process effect is because if we do it
			////// for each pixel in each shader, the result differs from anything
			////// else that is renderered in the forward renderer
			////myDeferredTexture.SetAsActiveTarget();
			////myTempFogTexture.SetAsResourceOnSlot(0);
			////myFullscreenRenderer->Render(FullscreenRenderer::Shader::GammaCorrection);
			//////myFullscreenRenderer->Render(FullscreenRenderer::Shader::Copy);
		}

		//return {};

		// Render the opaque forward objects
		{
			ZoneNamedN(zone4, "RenderManager::RenderScene OpaqueForwardObjects", true);
			GPU_ZONE_NAMED(opaqueForwardZone, "Opaque Forward Objects");

			myDeferredTexture.SetAsActiveTarget(&myDepthTexture);

			SetDepthStencilState(DepthStencilState_Default);
			// no blending when drawing opaque
			SetBlendState(BlendState_AlphaBlend);
			SetSamplerState(0, SamplerState_TrilinearWrap);
			SetRasterizerState(RasterizerState_CW);

			std::vector<CulledMeshInstance> frontToBackForwardModelInstancesMeshes = culledSceneData.myForwardMeshes;

			// Sort the meshes from front-to-back
			std::sort(
				frontToBackForwardModelInstancesMeshes.begin(),
				frontToBackForwardModelInstancesMeshes.end(),
				[&aCamera](CulledMeshInstance& aLeft, CulledMeshInstance& aRight)
				{
					const float distSqToLeft =
						(aLeft.myMeshInstance->GetModelInstance()->GetTransformMatrix().GetTranslation() -
							aCamera.GetTransform().GetPosition()).LengthSqr();

					const float distSqToRight =
						(aRight.myMeshInstance->GetModelInstance()->GetTransformMatrix().GetTranslation() -
							aCamera.GetTransform().GetPosition()).LengthSqr();

					return distSqToLeft > distSqToRight;
				});

			stats.myForwardRenderingStats =
				stats.myForwardRenderingStats + myForwardRenderer->RenderModelInstances(
					environmentLight,
					&aScene,
					{},
					frontToBackForwardModelInstancesMeshes,
					&aCamera);
		}

		// Copy the depth buffer after all opaque stuff are rendered
		{
			// Copy the depth buffer because we need it when drawing the decals

			DxUtils::UnbindShaderResourceView(*myFramework.GetContext(), 38);
			myDepthBufferCopy2.SetAsActiveTarget();
			myDepthTexture.SetAsResourceOnSlot(0);
			myFullscreenRenderer->Render(FullscreenRenderer::Shader::CopyDepth);

			DxUtils::UnbindRenderTargetView(*myFramework.GetContext());
			// unbind depth
			DxUtils::UnbindShaderResourceView(*myFramework.GetContext(), 0);
		}

		// to use in particles
		myDepthBufferCopy2.SetAsResourceOnSlot(38);

		//// Render particles
		//{
		//	GPU_ZONE_NAMED(particlesZone, "Particles");

		//	SetBlendState(BlendState_Additive);
		//	SetSamplerState(0, SamplerState_TrilinearWrap);

		//	myDeferredTexture.SetAsActiveTarget(&myDepthTexture);

		//	SetDepthStencilState(DepthStencilState_ReadOnly);
		//	myParticleRenderer->Render(&aCamera, culledSceneData.emitters);

		//	// Reset the depth stencil state
		//	SetDepthStencilState(DepthStencilState_Default);
		//}

		// TODO: Sort all Opaque from Front-To-Back
		// Optimizatoin techqinues: https://realtimecollisiondetection.net/blog/?p=86

		std::vector<CulledMeshInstance> backToFrontTransparentMeshes = culledSceneData.myForwardTransparentMeshes;

		// TODO: We are currently sorting by the parent ModelInstance
		// if a ModelInstance has multiple transparent meshes, this 
		// will not work, I need to sort by the position offseted by local mesh position as well
		std::sort(
			backToFrontTransparentMeshes.begin(),
			backToFrontTransparentMeshes.end(),
			[&aCamera](CulledMeshInstance& aLeft, CulledMeshInstance& aRight)
			{
				const float distSqToLeft =
					(aLeft.myMeshInstance->GetModelInstance()->GetTransformMatrix().GetTranslation() -
						aCamera.GetTransform().GetPosition()).LengthSqr();

				const float distSqToRight =
					(aRight.myMeshInstance->GetModelInstance()->GetTransformMatrix().GetTranslation() -
						aCamera.GetTransform().GetPosition()).LengthSqr();

				return distSqToLeft > distSqToRight;
			});

		// Render the transparent objects
		{
			ZoneNamedN(zone5, "RenderManager::RenderScene TransparentObjects", true);

			GPU_ZONE_NAMED(transparentZone, "Transparent Objects");

			// Due to the fog being a post process effect, we cannot have transparent object
			// writing to the depth buffer, otherwise transparent objects would
			// seem visible and fog will think its a model that is close bcuz depth is e.g. 0.99
			// We also disable depth writing for the particles later on

			// TODO: Divide up forward and transparent so normal forward is not shit performance!
			// Normal forward should write to the depth buffer!

			/*
				I disable the transparent objects from writing to the depth
				because if we have a mesh that is intersecting itself (e.g. fire shader)
				the drawing order won't help and depth will cause of of the sides
				to not be drawn.

				Disabling depth like this, causes complex non-transparent models to draw incorrectly
				the solution is to enable depth on them (this is what unity does)
				Reference: https://docs.unity3d.com/2017.3/Documentation/Manual/SL-CullAndDepth.html

				// TODO: Should be able to in the effect to
				* enable/disable culling
				* enable/disable depth write
				* set blend state
			*/
			SetDepthStencilState(DepthStencilState_ReadOnly);

			SetSamplerState(0, SamplerState_TrilinearWrap);
			SetBlendState(BlendState_AlphaBlend);
			myDeferredTexture.SetAsActiveTarget(&myDepthTexture);

			// TODO: All forward objects are renderer twice, only do this for transparent objects!
			// The reason we want to render transparent objects twice is that we want to draw
			// the back part of the mesh first, so the front part can blend with the back

			// We render the back sides of the transparent objects first
			// because we want the front to blend with the back
			SetRasterizerState(RasterizerState_CCW);

			stats.myForwardRenderingStats =
				stats.myForwardRenderingStats + myForwardRenderer->RenderModelInstances(
					environmentLight,
					&aScene,
					{},
					backToFrontTransparentMeshes,
					&aCamera);

			SetRasterizerState(RasterizerState_CW);

			stats.myForwardRenderingStats =
				stats.myForwardRenderingStats + myForwardRenderer->RenderModelInstances(
					environmentLight,
					&aScene,
					{},
					backToFrontTransparentMeshes,
					&aCamera);


			// Reset the depth stencil state
			SetDepthStencilState(DepthStencilState_Default);
		}

		// Render particles
		{
			ZoneNamedN(zone5, "Particles", true);
			GPU_ZONE_NAMED(particlesZone, "Particles");

			SetBlendState(BlendState_Additive);
			SetSamplerState(0, SamplerState_TrilinearWrap);

			myDeferredTexture.SetAsActiveTarget(&myDepthTexture);

			SetDepthStencilState(DepthStencilState_ReadOnly);
			myParticleRenderer->Render(&aCamera, culledSceneData.emitters);

			// Reset the depth stencil state
			SetDepthStencilState(DepthStencilState_Default);
		}

		// Returns to default rasterizer
		SetRasterizerState(RasterizerState_CW);

		SetBlendState(BlendState_Disable);

		// Must not wrap, because otherwise bloom will bleed to other opposite side of window
		SetSamplerState(0, SamplerState_Trilinear);

		DxUtils::UnbindRenderTargetView(*myFramework.GetContext());

		myFrameBuffer->Bind();

		myFullscreenRenderer->RenderPostProcess(
			aScene.GetPostProcessData(),
			myDeferredTexture,
			myDepthTexture,
			myGBuffer,
			myDeferredTexture2);

		// Render 3d sprites
		{
			ZoneNamedN(zone5, "3D Sprites & Text", true);
			GPU_ZONE_NAMED(spriteZone3d, "3d sprites & text");

			std::vector<SpriteComponent*>& sprites3D = culledSceneData.my3DSprites;

			std::sort(sprites3D.begin(), sprites3D.end(), [&](const SpriteComponent* first, const SpriteComponent* second) {
				float firstDist = (aCamera.GetTransform().GetPosition() - first->GetGameObject()->GetTransform().GetPosition()).LengthSqr();
				float secondDist = (aCamera.GetTransform().GetPosition() - second->GetGameObject()->GetTransform().GetPosition()).LengthSqr();
				return secondDist < firstDist;
				});

			myDeferredTexture2.SetAsActiveTarget();

			SetBlendState(BlendState_AlphaBlend);
			// TODO: render UI to a separate target?
			//SetSamplerState(0, SamplerState_Point);
			SetSamplerState(0, SamplerState_Point);

			myDeferredTexture2.SetAsActiveTarget(&myDepthTexture);
			SetDepthStencilState(DepthStencilState_ReadOnly);
			mySpriteRenderer->Render(&aCamera, sprites3D);


			SetBlendState(BlendState_Disable);
			SetSamplerState(0, SamplerState_TrilinearWrap);
			stats.myTextRenderingStats = myTextRenderer->Render(&aCamera, culledSceneData.texts);
		}

		SetRasterizerState(RasterizerState_CW);

		{
			ZoneNamedN(zone5, "2D sprites & Text", true);
			GPU_ZONE_NAMED(spriteZone2d, "2d sprites & text");

			static std::map<int, std::vector<std::function<void()>>> sortedUIElements;
			{
				ZoneNamedN(zone6, "clear", true);
				for (auto& entry : sortedUIElements)
				{
					entry.second.clear();
				}
			}

			{
				ZoneNamedN(zone6, "sort text", true);
				for (auto& entry : culledSceneData.my2DTexts)
				{
					int order = entry->GetSortOrder();
					sortedUIElements[order].emplace_back([&]()
						{
							myTextRenderer->RenderSingle2D(entry);
						});
				}
			}
			{
				ZoneNamedN(zone6, "sort sprites", true);
				for (auto& entry : culledSceneData.spriteInstances)
				{
					int order = entry->GetSortOrder();
					sortedUIElements[order].emplace_back([&]()
						{
							mySpriteRenderer->RenderSingle2D(entry);
						});
				}
			}

			{
				ZoneNamedN(zone6, "render", true);
				SetSamplerState(0, SamplerState_TrilinearWrap);
				SetDepthStencilState(DepthStencilState_ReadOnly);
				SetBlendState(BlendState_AlphaBlend);
				for (const auto& entry : sortedUIElements)
				{
					for (const auto& element : entry.second)
					{
						element();
					}
				}
			}
			SetDepthStencilState(DepthStencilState_Default);
		}

		//// Render text
		//{
		//	GPU_ZONE_NAMED(textZone, "2d elements");
		//
		//	SetBlendState(BlendState_Disable);
		//	SetSamplerState(0, SamplerState_TrilinearWrap);
		//	stats.myTextRenderingStats = stats.myTextRenderingStats + myTextRenderer->Render2D(culledSceneData.my2DTexts);
		//
		//	SetSamplerState(0, SamplerState_Point);
		//	SetDepthStencilState(DepthStencilState_ReadOnly);
		//	mySpriteRenderer->Render(&aCamera, culledSceneData.spriteInstances);
		//	SetDepthStencilState(DepthStencilState_Default);
		//}


		// Render debug primitives
		{
			GPU_ZONE_NAMED(debugDrawZone, "Debug Draw");
			ZoneNamedN(zone5, "Debug Draws", true);

			myDeferredTexture2.SetAsActiveTarget(&myDepthTexture);

			SetBlendState(BlendState_Disable);
			myDebugRenderer->RenderDepthTested(&aCamera);

			myDeferredTexture2.SetAsActiveTarget();
			SetBlendState(BlendState_Disable);
			myDebugRenderer->Render(&aCamera);
		}

#if !defined(TRACY_ENABLE) && !defined(_RETAIL)
		{
			ZoneNamedN(zone5, "Selection Render", true);
			GPU_ZONE_NAMED(selectionZone, "Selection Render");
			mySelectionTexture.SetAsActiveTarget(&mySelectionDepthTexture);
			mySelectionTexture.ClearTexture();
			mySelectionDepthTexture.ClearDepth();

			std::vector<CulledMeshInstance> meshes = aCulledSceneData.myDeferredMeshes;
			meshes.insert(meshes.end(), aCulledSceneData.myForwardMeshes.begin(), aCulledSceneData.myForwardMeshes.end());
			meshes.insert(meshes.end(), aCulledSceneData.myForwardTransparentMeshes.begin(), aCulledSceneData.myForwardTransparentMeshes.end());

			mySelectionRenderer->RenderModelInstances(meshes, &aCamera);
		}
#endif

		return stats;
	}

	void RenderManager::DrawDebugMenu()
	{
		myFrameBuffer->DrawDebugMenu();
	}

	void RenderManager::SetBlendState(const BlendState aBlendState)
	{
		UINT mask = 0xffffffff;
		const FLOAT blendFactor[4] = { 0.f, 0.f, 0.f, 0.f };
		myFramework.GetContext()->OMSetBlendState(myBlendStates[aBlendState], blendFactor, mask);
	}

	void RenderManager::SetDepthStencilState(const DepthStencilState aDepthStencil)
	{
		myFramework.GetContext()->OMSetDepthStencilState(myDepthStencilStates[aDepthStencil], 0);
	}

	void RenderManager::SetSamplerState(const int aSlot, const SamplerState aSamplerState)
	{
		// more?
		myFramework.GetContext()->PSSetSamplers(aSlot, 1, &mySamplerStates[aSamplerState]);
		myFramework.GetContext()->VSSetSamplers(aSlot, 1, &mySamplerStates[aSamplerState]);
		myFramework.GetContext()->GSSetSamplers(aSlot, 1, &mySamplerStates[aSamplerState]);
	}

	void RenderManager::SetRasterizerState(const RasterizerState aRasterizeState)
	{
		myFramework.GetContext()->RSSetState(myRasterizerStates[static_cast<int>(aRasterizeState)]);
	}

	ID3D11DepthStencilState* RenderManager::GetDepthStencilState(const DepthStencilState aDepthStencilState)
	{
		return myDepthStencilStates[static_cast<int>(aDepthStencilState)];
	}

	ID3D11RasterizerState* RenderManager::GetRasterizerState(const RasterizerState aRasterizerState)
	{
		return myRasterizerStates[static_cast<int>(aRasterizerState)];
	}

	ID3D11BlendState* RenderManager::GetBlendState(const BlendState aBlendState)
	{
		return myBlendStates[static_cast<int>(aBlendState)];
	}

	const ForwardRenderer& RenderManager::GetForwardRenderer() const
	{
		return *myForwardRenderer;
	}

	const FullscreenRenderer& RenderManager::GetFullscreenRenderer() const
	{
		return *myFullscreenRenderer;
	}

	const DeferredRenderer& RenderManager::GetDeferredRenderer() const
	{
		return *myDeferredRenderer;
	}

	const ParticleRenderer& RenderManager::GetParticleRenderer() const
	{
		return *myParticleRenderer;
	}

	const SpriteRenderer& RenderManager::GetSpriteRenderer() const
	{
		return *mySpriteRenderer;
	}

	const TextRenderer& RenderManager::GetTextRenderer() const
	{
		return *myTextRenderer;
	}

	DebugRenderer& RenderManager::GetDebugRenderer()
	{
		return *myDebugRenderer;
	}

	void RenderManager::Receive(const EventType aEventType, const std::any& aValue)
	{
		if (aEventType == EventType::WindowResized)
		{
			OnResized(std::any_cast<WindowResizedData>(aValue));
		}
	}

	FrameBufferTempName& RenderManager::GetFrameBuffer()
	{
		return *myFrameBuffer;
	}

	bool RenderManager::InitBlendStates(Directx11Framework& aFramework)
	{
		HRESULT result = 0;

		{
			D3D11_BLEND_DESC alphaBlendDesc = {};
			{
				alphaBlendDesc.RenderTarget[0].BlendEnable = true;
				alphaBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
				alphaBlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
				alphaBlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
				alphaBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;

				alphaBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE; // original
				//alphaBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;

				alphaBlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_MAX; // original
				//alphaBlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

				alphaBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

				// NOTE: If I enabled this, the sprite rendering would 
				// completely disappear when alpha got below 0.5
				// alphaBlendDesc.AlphaToCoverageEnable = true;
			}

			result = aFramework.GetDevice()->CreateBlendState(&alphaBlendDesc, &myBlendStates[BlendState_AlphaBlend]);

			if (FAILED(result))
			{
				return false;
			}
		}

		{
			D3D11_BLEND_DESC alphaBlendAlphaCovDesc = {};
			{
				alphaBlendAlphaCovDesc.RenderTarget[0].BlendEnable = true;
				alphaBlendAlphaCovDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
				alphaBlendAlphaCovDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
				alphaBlendAlphaCovDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
				alphaBlendAlphaCovDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;

				alphaBlendAlphaCovDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE; // original
				//alphaBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;

				alphaBlendAlphaCovDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_MAX; // original
				//alphaBlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

				alphaBlendAlphaCovDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

				// NOTE: If I enabled this, the sprite rendering would 
				// completely disappear when alpha got below 0.5
				alphaBlendAlphaCovDesc.AlphaToCoverageEnable = TRUE;
			}

			result = aFramework.GetDevice()->CreateBlendState(&alphaBlendAlphaCovDesc, &myBlendStates[BlendState_AlphaBlend_AlphaToCoverage]);

			if (FAILED(result))
			{
				return false;
			}
		}

		{
			D3D11_BLEND_DESC additiveBlendDesc = {};
			{
				additiveBlendDesc.RenderTarget[0].BlendEnable = true;
				additiveBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
				additiveBlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
				additiveBlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
				additiveBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
				additiveBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
				additiveBlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_MAX;
				additiveBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			}

			result = aFramework.GetDevice()->CreateBlendState(&additiveBlendDesc, &myBlendStates[BlendState_Additive]);

			if (FAILED(result))
			{
				return false;
			}
		}

		myBlendStates[BlendState_Disable] = nullptr;

		return true;
	}

	bool RenderManager::InitDepthStencilStates(Directx11Framework& aFramework)
	{
		D3D11_DEPTH_STENCIL_DESC readOnlyDepthDesc = {};
		{
			readOnlyDepthDesc.DepthEnable = true;
			readOnlyDepthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			readOnlyDepthDesc.DepthFunc = D3D11_COMPARISON_LESS;
			readOnlyDepthDesc.StencilEnable = false;
		}

		HRESULT result = aFramework.GetDevice()->CreateDepthStencilState(
			&readOnlyDepthDesc,
			&myDepthStencilStates[DepthStencilState_ReadOnly]);

		if (FAILED(result))
		{
			return false;
		}

		// Create a stencil enables
		// DepthStencilState_StencilEnabled
		{
			D3D11_DEPTH_STENCIL_DESC desc = {};
			{
				desc.DepthEnable = TRUE;
				desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
				desc.DepthFunc = D3D11_COMPARISON_LESS;

				desc.StencilEnable = TRUE;
				desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
				desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

				desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
				desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP; // doesn't matter bcuz its always?
				desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP; // doesn't matter bcuz its always?
				desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;

				desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
				desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP; // doesn't matter bcuz its always?
				desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP; // doesn't matter bcuz its always?
				desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
			}

			result = aFramework.GetDevice()->CreateDepthStencilState(
				&desc,
				&myDepthStencilStates[DepthStencilState_StencilAlwaysWithDepthTest]);

			if (FAILED(result))
			{
				return false;
			}
		}

		{
			D3D11_DEPTH_STENCIL_DESC desc = {};
			{
				desc.DepthEnable = TRUE;
				desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
				desc.DepthFunc = D3D11_COMPARISON_LESS;

				desc.StencilEnable = TRUE;
				desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
				desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

				desc.FrontFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;
				desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
				desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
				desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;

				desc.BackFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;
				desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
				desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
				desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
			}

			result = aFramework.GetDevice()->CreateDepthStencilState(
				&desc,
				&myDepthStencilStates[DepthStencilState_StencilNotEqualWithDepthTest]);

			if (FAILED(result))
			{
				return false;
			}
		}

		{
			D3D11_DEPTH_STENCIL_DESC desc = {};
			{
				desc.DepthEnable = TRUE;

				// Z-Write is disabled
				desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
				desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

				desc.StencilEnable = TRUE;
				desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
				desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

				{
					desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
					desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
					desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;

					// Z-Fail writes non-zero value to stencil
					desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR_SAT;
				}

				{
					desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
					desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
					desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;

					// Z-Fail writes non-zero value to stencil
					desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR_SAT;
				}
			}

			result = aFramework.GetDevice()->CreateDepthStencilState(
				&desc,
				&myDepthStencilStates[DepthStencilState_PointlightPass1]);

			if (FAILED(result))
			{
				return false;
			}
		}

		{
			D3D11_DEPTH_STENCIL_DESC desc = {};
			{
				desc.DepthEnable = TRUE;

				// Z-Write is disabled
				desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
				desc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;

				desc.StencilEnable = TRUE;
				desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
				desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

				{
					desc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
					desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;

					desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_ZERO;
					desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_ZERO;
				}

				{
					desc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;
					desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;

					desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_ZERO;
					desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_ZERO;
				}
			}

			result = aFramework.GetDevice()->CreateDepthStencilState(
				&desc,
				&myDepthStencilStates[DepthStencilState_PointlightPass2]);

			if (FAILED(result))
			{
				return false;
			}
		}

		{
			D3D11_DEPTH_STENCIL_DESC skyboxDepthDesc = {};
			{
				skyboxDepthDesc.DepthEnable = true;
				skyboxDepthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
				skyboxDepthDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
			}

			HRESULT result = aFramework.GetDevice()->CreateDepthStencilState(
				&skyboxDepthDesc,
				&myDepthStencilStates[DepthStencilState_Skybox]);

			if (FAILED(result))
			{
				return false;
			}
		}

		myDepthStencilStates[DepthStencilState_Default] = nullptr;

		return true;
	}

	bool RenderManager::InitRasterizerStates(Directx11Framework& aFramework)
	{
		{
			D3D11_RASTERIZER_DESC wireframeRasterizerDesc = {};
			{
				wireframeRasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
				wireframeRasterizerDesc.CullMode = D3D11_CULL_BACK;
				wireframeRasterizerDesc.DepthClipEnable = true;
			}

			HRESULT result = aFramework.GetDevice()->CreateRasterizerState(
				&wireframeRasterizerDesc,
				&myRasterizerStates[RasterizerState_Wireframe]);

			if (FAILED(result))
			{
				return false;
			}
		}

		{
			D3D11_RASTERIZER_DESC desc = {};
			{
				desc.FillMode = D3D11_FILL_SOLID;
				desc.CullMode = D3D11_CULL_BACK;
				desc.DepthClipEnable = true;
			}

			desc.FrontCounterClockwise = false;

			HRESULT result = aFramework.GetDevice()->CreateRasterizerState(
				&desc,
				&myRasterizerStates[RasterizerState_CW]);

			if (FAILED(result))
			{
				return false;
			}

			desc.FrontCounterClockwise = true;

			result = aFramework.GetDevice()->CreateRasterizerState(
				&desc,
				&myRasterizerStates[RasterizerState_CCW]);

			if (FAILED(result))
			{
				return false;
			}
		}

		// 
		{
			D3D11_RASTERIZER_DESC desc = {};
			{
				desc.FillMode = D3D11_FILL_SOLID;
				desc.CullMode = D3D11_CULL_BACK;
				desc.DepthClipEnable = true;

				desc.DepthBias = 1000;

				//desc.DepthBiasClamp = 1.0f;
				desc.SlopeScaledDepthBias = 1.5f;

				//desc.SlopeScaledDepthBias = 0.1f;
			}

			desc.FrontCounterClockwise = false;

			HRESULT result = aFramework.GetDevice()->CreateRasterizerState(
				&desc,
				&myRasterizerStates[RasterizerState_ShadowCastersCWDepthBias]);

			if (FAILED(result))
			{
				return false;
			}
		}

		{
			D3D11_RASTERIZER_DESC plRastDesc = {};
			{
				plRastDesc.FillMode = D3D11_FILL_SOLID;
				plRastDesc.CullMode = D3D11_CULL_BACK;
				plRastDesc.DepthClipEnable = true;
			}

			HRESULT result = aFramework.GetDevice()->CreateRasterizerState(
				&plRastDesc,
				&myRasterizerStates[RasterizerState_PointlightPass1]);

			if (FAILED(result))
			{
				return false;
			}

			plRastDesc.CullMode = D3D11_CULL_FRONT;

			result = aFramework.GetDevice()->CreateRasterizerState(
				&plRastDesc,
				&myRasterizerStates[RasterizerState_PointlightPass2]);

			if (FAILED(result))
			{
				return false;
			}
		}

		{
			D3D11_RASTERIZER_DESC noCullingRasterizerDesc = {};
			{
				noCullingRasterizerDesc.FillMode = D3D11_FILL_SOLID;
				noCullingRasterizerDesc.CullMode = D3D11_CULL_NONE;
				noCullingRasterizerDesc.DepthClipEnable = true;
			}

			HRESULT result = aFramework.GetDevice()->CreateRasterizerState(
				&noCullingRasterizerDesc,
				&myRasterizerStates[RasterizerState_NoCulling]);

			if (FAILED(result))
			{
				return false;
			}
		}

		return true;
	}

	bool RenderManager::InitSamplerStates(Directx11Framework& aFramework)
	{
		HRESULT result = 0;

		{
			D3D11_SAMPLER_DESC pointSampleDesc = {};
			{
				pointSampleDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
				pointSampleDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
				pointSampleDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
				pointSampleDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
				pointSampleDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
				pointSampleDesc.MinLOD = -FLT_MAX;
				pointSampleDesc.MaxLOD = FLT_MAX;
			}

			result = aFramework.GetDevice()->CreateSamplerState(
				&pointSampleDesc,
				&mySamplerStates[SamplerState_Point]);

			if (FAILED(result))
			{
				return false;
			}
		}

		{
			D3D11_SAMPLER_DESC shadowPcfSampleDesc = {};
			{
				shadowPcfSampleDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
				shadowPcfSampleDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
				shadowPcfSampleDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
				shadowPcfSampleDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
				shadowPcfSampleDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
				shadowPcfSampleDesc.MinLOD = -FLT_MAX;
				shadowPcfSampleDesc.MaxLOD = FLT_MAX;
			}

			result = aFramework.GetDevice()->CreateSamplerState(
				&shadowPcfSampleDesc,
				&mySamplerStates[SamplerState_ShadowSamplerPCF]);

			if (FAILED(result))
			{
				return false;
			}
		}

		{
			D3D11_SAMPLER_DESC pointSampleDesc = {};
			{
				pointSampleDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
				pointSampleDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
				pointSampleDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
				pointSampleDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
				pointSampleDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
				pointSampleDesc.MinLOD = -FLT_MAX;
				pointSampleDesc.MaxLOD = FLT_MAX;
			}

			result = aFramework.GetDevice()->CreateSamplerState(
				&pointSampleDesc,
				&mySamplerStates[SamplerState_TrilinearClamp]);

			if (FAILED(result))
			{
				return false;
			}
		}

		D3D11_SAMPLER_DESC trilWrapSampleDesc = {};
		{
			trilWrapSampleDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			trilWrapSampleDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			trilWrapSampleDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			trilWrapSampleDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			trilWrapSampleDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
			trilWrapSampleDesc.MinLOD = -FLT_MAX;
			trilWrapSampleDesc.MaxLOD = FLT_MAX;
		}

		result = aFramework.GetDevice()->CreateSamplerState(
			&trilWrapSampleDesc,
			&mySamplerStates[SamplerState_TrilinearWrap]);

		if (FAILED(result))
		{
			return false;
		}

		mySamplerStates[SamplerState_Trilinear] = nullptr;

		return true;
	}

	void RenderManager::RenderToBackbuffer(FullscreenTexture& aTexture)
	{
		myFramework.GetBackBuffer().SetAsActiveTarget();
		aTexture.SetAsResourceOnSlot(0);
		myFullscreenRenderer->Render(FullscreenRenderer::Shader::Copy);
	}

	void RenderManager::OnResized(const WindowResizedData& aResizeData)
	{
		// We have to re-create the window size dependant resources
		CreateTextures();

		myFullscreenRenderer->OnResized();
	}

	void RenderManager::CreateTextures()
	{
		const auto renderSizeui = myWindowHandler.GetRenderingSize();
		const auto renderSizef = myWindowHandler.GetRenderingSize<float>();

		// Cannot bind DXGI_FORMAT_D32_FLOAT as shader resource
		// therefore I use a typeless format
		myDepthTexture = myFullscreenTextureFactory.CreateDepth(
			renderSizeui, /*DXGI_FORMAT::DXGI_FORMAT_R24G8_TYPELESS*/DXGI_FORMAT_R32G8X24_TYPELESS);
		CreateReadOnlyDepthStencilView(myDepthTexture);

		// myIntermediateTexture = myFullscreenTextureFactory.CreateTexture(
		// 	renderSizeui, DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT);

		myGBuffer = myFullscreenTextureFactory.CreateGBuffer(renderSizeui);

		myDeferredTexture = myFullscreenTextureFactory.CreateTexture(
			renderSizeui, DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT);

		myDeferredTexture2 = myFullscreenTextureFactory.CreateTexture(
			renderSizeui, DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT);

		myTempFogTexture = myFullscreenTextureFactory.CreateTexture(
			renderSizeui, DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT);

		myDebugResultTexture = myFullscreenTextureFactory.CreateTexture(
			renderSizeui, DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM);

		myDepthBufferCopy = myFullscreenTextureFactory.CreateTexture(
			renderSizeui, DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT);
		myDepthBufferCopy2 = myFullscreenTextureFactory.CreateTexture(
			renderSizeui, DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT);

		mySelectionTexture = myFullscreenTextureFactory.CreateTexture(
			renderSizeui, DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT);

		mySelectionDepthTexture = myFullscreenTextureFactory.CreateDepth(
			renderSizeui, DXGI_FORMAT::DXGI_FORMAT_R32G8X24_TYPELESS);

		if (mySSAO)
		{
			mySSAO->CreateTextures();
		}

		//myDeferredTexture2 = myFullscreenTextureFactory.CreateTexture(
		//	renderSizeui, DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM);

		// DXGI_FORMAT_R16G16B16A16_FLOAT
	}

	void RenderManager::RenderSceneToBackBuffer(Scene& aScene)
	{
		// We render the scene's result texture to the backbuffer
		// TODO: In future, we want the cameras to have textures

		if (!GetEngine().IsEditorEnabled() &&
			aScene.GetRendererScene().GetRendererDebugOutput() != RendererSceneDebugOutput::Nothing)
		{
			auto& resultTexture = aScene.GetRendererScene().GetDebugGBufferTexture();

			// Write the last result to backbuffer
			myFramework.GetBackBuffer2().SetAsActiveTarget();
			//myDeferredTexture.SetAsResourceOnSlot(0);
			resultTexture.SetAsResourceOnSlot(0);
			myFullscreenRenderer->Render(FullscreenRenderer::Shader::Copy);
			DxUtils::UnbindShaderResourceView(*myFramework.GetContext(), 0);
		}
		else
		{
			auto& resultTexture = aScene.GetRendererScene().GetTextureToRenderTo();

			// Write the last result to backbuffer
			myFramework.GetBackBuffer2().SetAsActiveTarget();
			//myDeferredTexture.SetAsResourceOnSlot(0);
			resultTexture.SetAsResourceOnSlot(0);
			myFullscreenRenderer->Render(FullscreenRenderer::Shader::Copy);
			DxUtils::UnbindShaderResourceView(*myFramework.GetContext(), 0);
		}
	}

	void RenderManager::RenderResultToBackBuffer(Scene& aScene)
	{
		auto& resultTexture = myFramework.GetBackBuffer2();

		// Write the last result to backbuffer
		myFramework.GetBackBuffer().SetAsActiveTarget();
		//myDeferredTexture.SetAsResourceOnSlot(0);
		resultTexture.SetAsResourceOnSlot(0);
		myFullscreenRenderer->Render(FullscreenRenderer::Shader::Copy);
		DxUtils::UnbindShaderResourceView(*myFramework.GetContext(), 0);
	}

	ID3D11DepthStencilView* RenderManager::GetReadOnlyDepthStencilView()
	{
		return myReadOnlyDepthStencilView.Get();
	}

	void RenderManager::DepthPrePass(
		const RenderCommandList& aRenderCommandChain,
		FullscreenTexture& aDepthTexture)
	{
		//auto& context = *myFramework.GetContext();
		//
		//for (const auto& [pass, entry] : aRenderCommandChain.myPasses)
		//{
		//	for (const auto& [_, modelGroup] : entry.myModelGroups)
		//	{
		//
		//	}
		//}
	}

	void RenderManager::CreateReadOnlyDepthStencilView(FullscreenTexture& aDepthTexture)
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(desc));

		desc.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = 0;
		desc.Flags = D3D11_DSV_READ_ONLY_DEPTH;

		auto hr = myFramework.GetDevice()->CreateDepthStencilView(
			aDepthTexture.GetTextureTest(),
			&desc,
			&myReadOnlyDepthStencilView);

		ThrowIfFailed(hr);
	}

}
