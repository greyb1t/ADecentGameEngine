#include "pch.h"
#include "RendererScene.h"

#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/MeshComponent.h"
#include "Engine/GameObject/Components/PointLightComponent.h"
#include "Engine/GameObject/Components/SpotLightComponent.h"
#include "Engine/Renderer/Model/ModelInstance.h"
#include "Engine/Renderer/Camera/Camera.h"
#include "Engine/Renderer/Culling/ViewFrustum.h"
#include "Engine/ResourceManagement/Resources/ModelResource.h"
#include "Engine/Renderer/Model/ModelInstance.h"
#include "Engine/Renderer/Material/MaterialInstance.h"
#include "Engine/Renderer/Material/Material.h"
#include "Engine/Renderer/Scene/CulledSceneData.h"
#include "Engine/Renderer/Material/EffectPass.h"
#include "Engine/Renderer/Model/Model.h"
#include "../GraphicsEngine.h"
#include "../Texture/FullscreenTextureFactory.h"
#include "Engine/GameObject/Components/DecalComponent.h"
#include "Engine/ResourceManagement/Resources/TextureResource.h"
#include "Engine/GameObject/Components/EnvironmentLightComponent.h"
#include "Engine/GameObject/Components/ModelComponent.h"
#include "Engine/GameObject/Components/ReflectionCaptureComponent.h"
#include "Engine/Renderer/TracyProfilingGPU.h"
#include "Engine/Renderer/Model/MeshInstance.h"
#include "Engine/Renderer/Renderers/DeferredRenderer.h"
#include "Engine/ResourceManagement/Resources/MaterialResource.h"
#include "Engine/Renderer/Text/Text.h"
#include "Engine/GameObject/Components/SpriteComponent.h"
#include "Engine/Renderer/Material/MaterialFactory.h"
#include "Engine/GameObject/Components/ModelComponent.h"

namespace Engine
{
	RendererScene::~RendererScene()
	{
		GetEngine().GetGraphicsEngine().GetWindowHandler().RemoveObserver(EventType::WindowResized, this);
	}

	bool RendererScene::Init()
	{
		GetEngine().GetGraphicsEngine().GetWindowHandler().AddObserver(EventType::WindowResized, this);

		CreateTextures();

		return true;
	}

	void RendererScene::ClearRenderCommands()
	{
		myModelInstancess.clear();
		myMeshComponents.clear();
		myPointLights.clear();
		mySpotLights.clear();
		mySpriteInstances.clear();
		my3DSprites.clear();
		myTexts.clear();
		my2DTexts.clear();
		myParticleEmitters.clear();
		myDecals.clear();
		myReflectionCaptureComponents.clear();
	}

	void RendererScene::LateUpdate(const float aDeltaTime)
	{
	}

	void RendererScene::PrepareRendering(Camera& aMainCamera, EnvironmentLightComponent& aEnvironmentLight)
	{
		ZoneScopedN("Scene::PrepareRendering");

		// Must be cleared here
		myReflectionCaptures.clear();

		myMainCamera = &aMainCamera;
		myEnvironmentalLight = &aEnvironmentLight;

		myCulledSceneData = CullEverything(*myMainCamera);
		CreateRenderCommandChain(myCulledSceneData, myCommandList);

		// REFLECTION CAPTURES
#if 1
		// Cull the rendererd scene for each reflection capture
		{
			const auto& reflectionCaptures = myCulledSceneData.myReflectionCaptureComponents;

			for (const auto& reflectionCapture : reflectionCaptures)
			{
				ReflectionCapture capture;

				capture.myCubemapTexture = &reflectionCapture->GetCubemapTexture();
				capture.myPosition = reflectionCapture->GetTransform().GetPosition();
				capture.myCubemapTextureSmooth = &reflectionCapture->GetCubemapTextureSmooth();

				for (int i = 0; i < 6; ++i)
				{
					auto& cam = reflectionCapture->GetCamera(i);

					auto& camSide = capture.myCameraSides[i];

					camSide.myCamera = &cam;
					camSide.myCulledSceneData = CullEverything(cam);
					camSide.mySideIndex = i;
				}

				myReflectionCaptures.push_back(capture);
			}
		}
#endif

		ClearRenderCommands();
	}

	void RendererScene::RenderModelInstance(ModelComponent& aModelComponent)
	{
		//aModelComponent.GetMeshMaterialInstanceByIndex(0).GetMaterialRef()->

		myModelInstancess.push_back(&aModelComponent);
	}

	void RendererScene::RenderMeshInstance(MeshComponent& aMeshComponent)
	{
		myMeshComponents.push_back(&aMeshComponent);
	}

	void RendererScene::RenderPointLight(PointLightComponent& aPointLightComponent)
	{
		myPointLights.push_back(&aPointLightComponent);
	}

	void RendererScene::RenderSpotLight(SpotLightComponent& aSpotLight)
	{
		mySpotLights.push_back(&aSpotLight);
	}

	void RendererScene::RenderParticleEmitter(ParticleEmitterComponent& aParticleEmitterComponent)
	{
		myParticleEmitters.push_back(&aParticleEmitterComponent);
	}

	void RendererScene::CullForwardModelsMeshes(const Camera& aCamera,
		const bool aMustCastShadows,
		std::vector<CulledMeshInstance>& aDeferredMeshesOut,
		std::vector<CulledMeshInstance>& aForwardMeshesOut,
		std::vector<CulledMeshInstance>& aForwardTransparentMeshesOut) const
	{
		CPU_ZONE_NAMED(zone1, "Cull Forward Renderer");

		aDeferredMeshesOut.clear();
		aForwardMeshesOut.clear();
		aForwardTransparentMeshesOut.clear();

		for (const auto& modelComponent : myModelInstancess)
		{
			if (!modelComponent->GetModel()->IsValid())
			{
				continue;
			}

			if (modelComponent->IsCullingDisabled() ||
				aCamera.GetViewFrustum().IsSphereInside(modelComponent->CalculateWorldBoundingSphere()))
			{
				// HACK: Since I did not have the meshcount initially
				const int meshCount = modelComponent->GetModel()->Get().GetMeshDatas().size();

				auto& meshInstanceList = modelComponent->GetMeshInstances();

				for (int i = 0; i < meshCount; ++i)
				{
					if (i
						>= meshInstanceList
						.size()) //Fixes krash where meshCount was bigger than meshInstanceList.size()
						break;

					auto& meshInstance = meshInstanceList[i];

					if (!meshInstance.GetMaterialInstance().IsValid())
					{
						continue;
					}

					if (aMustCastShadows)
					{
						// Skip the non-shadow casting meshes
						if (!meshInstance.IsCastingShadows())
						{
							continue;
						}
					}

					for (auto& effectPass :
						meshInstance.GetMaterialInstance().GetMaterial()->GetPasses())
					{
						if (effectPass->GetRendererType() == RendererType::Forward)
						{
							CulledMeshInstance culledMeshInstance;
							culledMeshInstance.myEffectPass = effectPass.get();
							culledMeshInstance.myMeshInstance = &meshInstance;
							culledMeshInstance.myUUID = modelComponent->GetGameObject()->GetUUID();

							// TODO: consider moving isTransparent from Material to EffectPass instead?
							if (meshInstance.GetMaterialInstance().GetMaterial()->IsTransparent())
							{
								aForwardTransparentMeshesOut.push_back(culledMeshInstance);
							}
							else
							{
								aForwardMeshesOut.push_back(culledMeshInstance);
							}
						}
						else if (effectPass->GetRendererType() == RendererType::Deferred)
						{
							CulledMeshInstance culledMeshInstance;
							culledMeshInstance.myEffectPass = effectPass.get();
							culledMeshInstance.myMeshInstance = &meshInstance;
							culledMeshInstance.myUUID = modelComponent->GetGameObject()->GetUUID();

							aDeferredMeshesOut.push_back(culledMeshInstance);
						}
						else
						{
							assert(false);
						}
					}
				}

				// modelInstance->SetPassedRenderCheck(true);
			}
		}
	}

	void RendererScene::CullCascadeMeshes(EnvironmentLightComponent* aLight, std::vector<std::vector<CulledMeshInstance>>& outMeshes) const
	{
		CPU_ZONE_NAMED(zone1, "Cull cascades");
		if (outMeshes.size() < aLight->GetNumCascades())
			outMeshes.resize(aLight->GetNumCascades());
		
		for (int cascadeIndex = 0; cascadeIndex < aLight->GetNumCascades(); ++cascadeIndex)
		{
			ViewFrustum frustum;
			frustum.InitOrthographic(aLight->GetCascadeView(cascadeIndex), aLight->GetCascadeProjection(cascadeIndex));
		
			std::vector<CulledMeshInstance>& cascadeVector = outMeshes[cascadeIndex];
			cascadeVector.clear();
		
			for (const auto& modelComponent : myModelInstancess)
			{
				if (!modelComponent->GetModel()->IsValid())
					continue;
		
				if (modelComponent->IsCullingDisabled() || frustum.IsSphereInside(modelComponent->CalculateWorldBoundingSphere()))
				{
					const int meshCount = modelComponent->GetModel()->Get().GetMeshDatas().size();
		
					auto& meshInstanceList = modelComponent->GetMeshInstances();
		
					for (int i = 0; i < meshCount; ++i)
					{
						if (i >= meshInstanceList.size())
							break;
		
						auto& meshInstance = meshInstanceList[i];
		
						if (!meshInstance.GetMaterialInstance().IsValid())
							continue;
		
						if (!meshInstance.IsCastingShadows())
							continue;
		
						for (auto& effectPass : meshInstance.GetMaterialInstance().GetMaterial()->GetPasses())
						{
							CulledMeshInstance culledMeshInstance;
							culledMeshInstance.myEffectPass = effectPass.get();
							culledMeshInstance.myMeshInstance = &meshInstance;
							culledMeshInstance.myUUID = modelComponent->GetGameObject()->GetUUID();
		
							cascadeVector.emplace_back(culledMeshInstance);
						}
					}
				}
			}
		}
	}

	void RendererScene::CullMeshComponents(const Camera& aCamera,
		const bool aMustCastShadows,
		std::vector<CulledMeshComponent>& aDeferredMeshesOut,
		std::vector<CulledMeshComponent>& aForwardMeshesOut,
		std::vector<CulledMeshComponent>& aForwardTransparentMeshesOut) const
	{
		CPU_ZONE("RendererScene::CullMeshComponents");

		aDeferredMeshesOut.clear();
		aForwardMeshesOut.clear();
		aForwardTransparentMeshesOut.clear();

		for (const auto& meshComponent : myMeshComponents)
		{
			if (!meshComponent->GetModel()->IsValid())
			{
				continue;
			}

			if (!meshComponent->GetMaterialInstance().IsValid())
			{
				continue;
			}

			if (meshComponent->IsCullingDisabled() ||
				aCamera.GetViewFrustum().IsSphereInside(meshComponent->CalculateWorldBoundingSphere()))
			{
				{
					if (aMustCastShadows)
					{
						// Skip the non-shadow casting meshes
						if (!meshComponent->IsCastingShadows())
						{
							continue;
						}
					}

					for (auto& effectPass :
						meshComponent->GetMaterialInstance().GetMaterial()->GetPasses())
					{
						if (effectPass->GetRendererType() == RendererType::Forward)
						{
							CulledMeshComponent culledMeshComponent;
							culledMeshComponent.myEffectPass = effectPass.get();
							culledMeshComponent.myMeshComponent = meshComponent;
							culledMeshComponent.myUUID = meshComponent->GetGameObject()->GetUUID();

							// TODO: consider moving isTransparent from Material to EffectPass instead?
							if (meshComponent->GetMaterialInstance().GetMaterial()->IsTransparent())
							{
								aForwardTransparentMeshesOut.push_back(culledMeshComponent);
							}
							else
							{
								aForwardMeshesOut.push_back(culledMeshComponent);
							}
						}
						else if (effectPass->GetRendererType() == RendererType::Deferred)
						{
							CulledMeshComponent culledMeshComponent;
							culledMeshComponent.myEffectPass = effectPass.get();
							culledMeshComponent.myMeshComponent = meshComponent;
							culledMeshComponent.myUUID = meshComponent->GetGameObject()->GetUUID();

							aDeferredMeshesOut.push_back(culledMeshComponent);
						}
						else
						{
							assert(false);
						}
					}
				}

				// modelInstance->SetPassedRenderCheck(true);
			}
		}
	}

	void RendererScene::CullForwardModelsMeshesByDistance(const PointLightComponent* aPointLight,
		const bool aMustCastShadows,
		std::vector<CulledMeshInstance>& aDeferredMeshesOut,
		std::vector<CulledMeshInstance>& aForwardMeshesOut) const
	{
		CPU_ZONE("RendererScene::CullForwardModelsMeshesByDistance");

		aDeferredMeshesOut.clear();
		aForwardMeshesOut.clear();

		for (const auto& modelInstance : myModelInstancess)
		{
			if (!modelInstance->GetModel()->IsValid())
			{
				continue;
			}

			const float distanceSq = (modelInstance->GetTransformMatrix().GetTranslation()
				- aPointLight->GetGameObject()->GetTransform().GetPosition())
				.LengthSqr();
			const float rangeSq = aPointLight->GetRange() * aPointLight->GetRange();

			if (distanceSq > rangeSq)
			{
				continue;
			}

			{
				// HACK: Since I did not have the meshcount initially
				const int meshCount = modelInstance->GetModel()->Get().GetMeshDatas().size();

				auto& meshInstanceList = modelInstance->GetMeshInstances();

				for (int i = 0; i < meshCount; ++i)
				{
					if (i
						>= meshInstanceList
						.size()) //Fixes krash where meshCount was bigger than meshInstanceList.size()
						break;

					auto& meshInstance = meshInstanceList[i];

					if (!meshInstance.GetMaterialInstance().IsValid())
					{
						continue;
					}

					if (aMustCastShadows)
					{
						// Skip the non-shadow casting meshes
						if (!meshInstance.IsCastingShadows())
						{
							continue;
						}
					}

					for (auto& effectPass :
						meshInstance.GetMaterialInstance().GetMaterial()->GetPasses())
					{
						if (effectPass->GetRendererType() == RendererType::Forward)
						{
							CulledMeshInstance culledMeshInstance;
							culledMeshInstance.myEffectPass = effectPass.get();
							culledMeshInstance.myMeshInstance = &meshInstance;

							aForwardMeshesOut.push_back(culledMeshInstance);
						}
						else if (effectPass->GetRendererType() == RendererType::Deferred)
						{
							CulledMeshInstance culledMeshInstance;
							culledMeshInstance.myEffectPass = effectPass.get();
							culledMeshInstance.myMeshInstance = &meshInstance;

							aDeferredMeshesOut.push_back(culledMeshInstance);
						}
						else
						{
							assert(false);
						}
					}
				}

				// modelInstance->SetPassedRenderCheck(true);
			}
		}
	}

	void RendererScene::CullDecals(
		const Camera& aCamera,
		std::vector<CulledDecalComponent>& aCulledDecals) const
	{
		CPU_ZONE_NAMED(decalsZone, "RendererScene::CullDecals");

		const ModelRef& decalCube = DeferredRenderer::GetDecalCubeModel();

		const auto& boundingSphere = decalCube->Get().GetBoundingSphere();

		for (const auto& decal : myDecals)
		{
			if (!decal->GetMaterialInstance().IsValid())
			{
				continue;
			}

			const auto scale = decal->GetTransform().GetScale();

			float biggestAxisScale = 1.f;
			biggestAxisScale = std::max(biggestAxisScale, scale.x);
			biggestAxisScale = std::max(biggestAxisScale, scale.y);
			biggestAxisScale = std::max(biggestAxisScale, scale.z);

			const auto worldSpaceBoundingSphere
				= C::Sphere<float>(decal->GetTransform().GetPosition() + boundingSphere.GetCenter(),
					boundingSphere.GetRadius() * biggestAxisScale);

			if (!aCamera.GetViewFrustum().IsSphereInside(worldSpaceBoundingSphere))
			{
				continue;
			}

			CulledDecalComponent culledDecal;
			culledDecal.myDecalComponent = decal;

			culledDecal.myEffectPass = decal->GetMaterialInstance().GetMaterial()->GetPasses().front().get();

			if (decal->GetMaterialInstance().GetMaterial()->GetPasses().size() > 1)
			{
				LOG_WARNING(LogType::Renderer) << "Decal materials only supports 1 effect pass at the moment, only first pass rendered";
			}

			aCulledDecals.push_back(culledDecal);
		}
	}

	std::vector<PointLightComponent*> RendererScene::CullPointLights(
		const ModelInstance* aModelInstance) const
	{
		std::vector<PointLightComponent*> culledLights;

		for (const auto& culledPointLight : myCulledSceneData.frustumCulledPointLights)
		{
			PointLightComponent* pointLight = culledPointLight.myPointLight;

			const float distanceSq = (aModelInstance->GetTransformMatrix().GetTranslation()
				- pointLight->GetGameObject()->GetTransform().GetPosition())
				.LengthSqr();
			const float rangeSq = pointLight->GetRange() * pointLight->GetRange();

			// TODO(filip): To fix the bug where a light did not affect
			// a model, we have to include the bounding sphere as well
			// and not just the center position
			// however, this is lower performance
			// DECIDE
			float modelBoundingRadiusSq = aModelInstance->CalculateWorldBoundingSphere().GetRadius() * aModelInstance->CalculateWorldBoundingSphere().GetRadius();

			// disabled for now
			modelBoundingRadiusSq = 0.f;

			if ((distanceSq - modelBoundingRadiusSq) < rangeSq)
			{
				culledLights.push_back(pointLight);
			}
		}

		const Vec3f modelPos = aModelInstance->GetTransformMatrix().GetTranslation();

		// If we have more lights than allowed to interact with the model
		// we must cut off the lights
		// Its important that we first sort the lights based on distance to the object
		// Otherwise, if we have 20 lights with HUGE range
		// Then a light right beside the model might not be rendererd
		// within those 8 lights allowed because some other lights further away
		// are being rendered with it
		// Therefore sort them based on distance and we only choose
		// the 8 closest lights
		if (culledLights.size() > MaxPointLightCount)
		{
			// Sort by closest to model instance first
			std::sort(culledLights.begin(), culledLights.end(),
				[&modelPos](PointLightComponent* aLeft, PointLightComponent* aRight)
				{
					const float distSqToLeft =
						(aLeft->GetGameObject()->GetTransform().GetPosition() -
							modelPos).LengthSqr();

					const float distSqToRight =
						(aRight->GetGameObject()->GetTransform().GetPosition() -
							modelPos).LengthSqr();

					return distSqToLeft < distSqToRight;
				});

			// Cut off rest of lights
			culledLights.resize(MaxPointLightCount);
		}

		assert(culledLights.size() <= MaxPointLightCount);

		return culledLights;
	}

	std::vector<PointLightComponent*> RendererScene::FrustumCullPointLights(const Camera* aCamera) const
	{
		CPU_ZONE("RendererScene::FrustumCullPointLights");

		std::vector<PointLightComponent*> culledLights;

		for (const auto& instance : myPointLights)
		{
			C::Sphere<float> sphere;
			sphere.InitWithCenterAndRadius(
				instance->GetGameObject()->GetTransform().GetPosition(), instance->GetRange());

			if (aCamera->GetViewFrustum().IsSphereInside(sphere))
			{
				culledLights.push_back(instance);
			}
		}

		return culledLights;
	}

	std::vector<SpotLightComponent*> RendererScene::CullSpotLights(
		const ModelInstance* aModelInstance) const
	{
		std::vector<SpotLightComponent*> culledLights;

		for (const auto& culledSpotLight : /*mySpotLights*/myCulledSceneData.frustumCulledSpotLights)
		{
			SpotLightComponent* spotLight = culledSpotLight.mySpotLight;

			const float distanceSq = (aModelInstance->GetTransformMatrix().GetTranslation()
				- spotLight->GetGameObject()->GetTransform().GetPosition())
				.LengthSqr();
			const float rangeSq = spotLight->GetRange() * spotLight->GetRange();

			if (distanceSq < rangeSq)
			{
				// culledLights[lightCount++] = spotLight;
				culledLights.push_back(spotLight);
			}
		}

		const Vec3f modelPos = aModelInstance->GetTransformMatrix().GetTranslation();

		// If we have more lights than allowed to interact with the model
		// we must cut off the lights
		// Its important that we first sort the lights based on distance to the object
		// Otherwise, if we have 20 lights with HUGE range
		// Then a light right beside the model might not be rendererd
		// within those 8 lights allowed because some other lights further away
		// are being rendered with it
		// Therefore sort them based on distance and we only choose
		// the 8 closest lights
		if (culledLights.size() > MaxSpotLightCount)
		{
			// Sort by closest to model instance first
			std::sort(culledLights.begin(), culledLights.end(),
				[&modelPos](SpotLightComponent* aLeft, SpotLightComponent* aRight)
				{
					const float distSqToLeft =
						(aLeft->GetGameObject()->GetTransform().GetPosition() -
							modelPos).LengthSqr();

					const float distSqToRight =
						(aRight->GetGameObject()->GetTransform().GetPosition() -
							modelPos).LengthSqr();

					return distSqToLeft < distSqToRight;
				});

			// Cut off rest of lights
			culledLights.resize(MaxSpotLightCount);
		}

		assert(culledLights.size() <= MaxSpotLightCount);

		return culledLights;
	}

	std::vector<SpotLightComponent*> RendererScene::FrustumCullSpotLights(const Camera* aCamera) const
	{
		CPU_ZONE("RendererScene::FrustumCullSpotLights");

		std::vector<SpotLightComponent*> culledLights;

		for (const auto& instance : mySpotLights)
		{
			C::Sphere<float> sphere;
			sphere.InitWithCenterAndRadius(
				instance->GetGameObject()->GetTransform().GetPosition(), instance->GetRange());

			if (aCamera->GetViewFrustum().IsSphereInside(sphere))
			{
				culledLights.push_back(instance);
			}
		}

		return culledLights;
	}

	std::vector<ParticleEmitterComponent*> RendererScene::CullEmitters(
		const Camera* aCamera) const
	{
		CPU_ZONE("RendererScene::CullEmitters");

		aCamera;
		// TODO: Properly cull them
		return myParticleEmitters;
	}

	std::vector<SpriteComponent*> RendererScene::CullSpriteInstances(const Camera* aCamera) const
	{
		CPU_ZONE("RendererScene::CullSpriteInstances");
		// TODO: properly cull them? if needed, I dont know
		return mySpriteInstances;
	}

	std::vector<Text*> RendererScene::CullTexts(const Camera* aCamera) const
	{
		CPU_ZONE("RendererScene::CullTexts");
		// TODO: cull
		return myTexts;
	}

	std::vector<ReflectionCaptureComponent*> RendererScene::CullReflectionCaptures(const Camera* aCamera) const
	{
		CPU_ZONE("RendererScene::CullReflectionCaptures");
		return myReflectionCaptureComponents;
	}

	CulledSceneData RendererScene::CullEverything(const Camera& aCamera) const
	{
		CPU_ZONE_NAMED(mainZone, "Scene::CullEverything");

		// TODO: Parallelize the culling now that its separates :)

		// Reset the render check pass value for the models
		{
			CPU_ZONE_NAMED(zone1, "Reset render check");
			for (const auto& instance : myModelInstancess)
			{
				instance->SetPassedRenderCheck(false);
			}
		}

		CulledSceneData result;

		result.emitters = CullEmitters(&aCamera);
		result.spriteInstances = CullSpriteInstances(&aCamera);
		result.my3DSprites = my3DSprites;
		result.texts = CullTexts(&aCamera);
		result.my2DTexts = my2DTexts;
		result.myReflectionCaptureComponents = CullReflectionCaptures(&aCamera);

		CullForwardModelsMeshes(aCamera,
			false,
			result.myDeferredMeshes,
			result.myForwardMeshes,
			result.myForwardTransparentMeshes);

		CullMeshComponents(aCamera,
			false,
			result.myDeferredMeshComponents,
			result.myForwardMeshComponents,
			result.myForwardTransparentMeshComponents);

		CullDecals(aCamera, result.myDeferredDecalComponents);

		// Cull spotlights
		{
			CPU_ZONE_NAMED(spotlightsZone, "Cull Spotlights");

			/*
			std::vector<ModelComponent*> modelComponentsSorted = myModelInstancess;

			std::sort(
				modelComponentsSorted.begin(),
				modelComponentsSorted.end(),
				[](ModelComponent* aLeft, ModelComponent* aRight)
				{
					const float distanceSq = (myMeshInstance->GetModelInstance()
						->GetTransformMatrix()
						.GetTranslation()
						- aSpotLight->GetGameObject()->GetTransform().GetPosition())
						.LengthSqr();

					const float distance = Vec3f::DistanceSq(a);
				});
			*/



			std::vector<Wrapped> wrappeds;

			wrappeds.reserve(myModelInstancess.size());

			for (const auto& modelInstance : myModelInstancess)
			{
				Wrapped wrapped;
				wrapped.myPosition = modelInstance->GetTransformMatrix().GetTranslation();
				wrapped.myBoundingSphereRadius = modelInstance->CalculateWorldBoundingSphere().GetRadius();
				wrapped.myModelComponent = modelInstance;

				wrappeds.push_back(wrapped);
			}

			const auto culledSpotLights = FrustumCullSpotLights(&aCamera);

			for (const auto& spotLight : culledSpotLights)
			{
				CulledSceneData::CulledSpotLightData culledSpotLightData;

				culledSpotLightData.mySpotLight = spotLight;

				// Cull the shadow casters that the spot light does not affect
				if (spotLight->IsCastingShadows())
				{
					const auto& shadowCamera = spotLight->GetCamera();

#if 1
					auto shadowCastersNew = CullModelsByDistanceFromSpotLightMeshes2(
						wrappeds,
						spotLight);

					auto shadowCasters2New = CullMeshInstancesByFrustum(spotLight->GetCamera(), shadowCastersNew);

					culledSpotLightData.mySpotLightShadowCastersMeshes = std::move(shadowCasters2New);
#else
					std::vector<CulledMeshInstance> betterShadowCasters;

					std::vector<CulledMeshInstance> betterShadowCastersDeferred;
					std::vector<CulledMeshInstance> betterShadowCastersForward;
					std::vector<CulledMeshInstance> betterShadowCastersForwardTransparent;
					CullForwardModelsMeshes(shadowCamera,
						true,
						betterShadowCastersDeferred,
						betterShadowCastersForward,
						betterShadowCastersForwardTransparent);

					// Combine them
					betterShadowCastersDeferred.insert(betterShadowCastersDeferred.end(),
						betterShadowCastersForward.begin(),
						betterShadowCastersForward.end());
					betterShadowCastersDeferred.insert(betterShadowCastersDeferred.end(),
						betterShadowCastersForwardTransparent.begin(),
						betterShadowCastersForwardTransparent.end());

					// TODO: Cull by distance before culling by frustum, its faster!
					std::vector<CulledMeshInstance> evenBetterShadowCasters
						= CullModelsByDistanceFromSpotLightMeshes(
							spotLight, betterShadowCastersDeferred);

					assert(culledSpotLightData.mySpotLightShadowCastersMeshes.size() == evenBetterShadowCasters.size());

					culledSpotLightData.mySpotLightShadowCastersMeshes.insert(
						culledSpotLightData.mySpotLightShadowCastersMeshes.end(),
						evenBetterShadowCasters.begin(),
						evenBetterShadowCasters.end());
#endif
				}

				result.frustumCulledSpotLights.push_back(culledSpotLightData);
			}
		}

		// Cull pointslight
		{
			CPU_ZONE_NAMED(pointLightZone, "Cull PointLights");

			const auto culledPointLights = FrustumCullPointLights(&aCamera);

			for (const auto& pointLight : culledPointLights)
			{
				CulledSceneData::CulledPointLightData culledPointLightData;

				culledPointLightData.myPointLight = pointLight;

				// Cull the shadow casters that the point light does not affect
				if (pointLight->IsCastingShadows())
				{
					// NEW
					{

						std::vector<CulledMeshInstance> betterShadowCastersDeferred;
						std::vector<CulledMeshInstance> betterShadowCastersForward;

						CullForwardModelsMeshesByDistance(
							pointLight, true, betterShadowCastersDeferred, betterShadowCastersForward);

						// Combine them
						betterShadowCastersDeferred.insert(betterShadowCastersDeferred.end(),
							betterShadowCastersForward.begin(),
							betterShadowCastersForward.end());

						culledPointLightData.myNewWholeSceneAllDirectionsShadowCastersMeshes
							= betterShadowCastersDeferred;
					}

					// For each of the cube's sides
					for (int i = 0; i < 6; ++i)
					{
						const auto& shadowCamera = pointLight->GetCameras()[i];

						std::vector<CulledMeshInstance> betterShadowCastersDeferred;
						std::vector<CulledMeshInstance> betterShadowCastersForward;
						std::vector<CulledMeshInstance> betterShadowCastersForwardTransparent;
						CullForwardModelsMeshes(shadowCamera,
							true,
							betterShadowCastersDeferred,
							betterShadowCastersForward,
							betterShadowCastersForwardTransparent);

						// Combine them
						betterShadowCastersDeferred.insert(betterShadowCastersDeferred.end(),
							betterShadowCastersForward.begin(),
							betterShadowCastersForward.end());
						betterShadowCastersDeferred.insert(betterShadowCastersDeferred.end(),
							betterShadowCastersForwardTransparent.begin(),
							betterShadowCastersForwardTransparent.end());

						// TODO: Cull by distance before culling by frustum, its faster!
						std::vector<CulledMeshInstance> evenBetterShadowCasters
							= CullModelsByDistanceFromPointLightMeshes(
								pointLight, betterShadowCastersDeferred);

						culledPointLightData.myPointLightShadowCastersMeshes[i].insert(
							culledPointLightData.myPointLightShadowCastersMeshes[i].end(),
							evenBetterShadowCasters.begin(),
							evenBetterShadowCasters.end());
					}
				}

				result.frustumCulledPointLights.push_back(culledPointLightData);
			}
		}

		// OLD ENV LIGHT CULLING
		//std::vector<CulledMeshInstance> envLightShadowCastersDeferred;
		//std::vector<CulledMeshInstance> envLightShadowCastersForward;
		//std::vector<CulledMeshInstance> envLightShadowCastersForwardTransparent;
		//CullForwardModelsMeshes(myEnvironmentalLight->GetCamera(),
		//	true,
		//	envLightShadowCastersDeferred,
		//	envLightShadowCastersForward,
		//	envLightShadowCastersForwardTransparent);
		//
		//// Combine them
		//envLightShadowCastersDeferred.insert(envLightShadowCastersDeferred.end(),
		//	envLightShadowCastersForward.begin(),
		//	envLightShadowCastersForward.end());
		//envLightShadowCastersDeferred.insert(envLightShadowCastersDeferred.end(),
		//	envLightShadowCastersForwardTransparent.begin(),
		//	envLightShadowCastersForwardTransparent.end());
		// result.myEnvironmentLightShadowCastersMeshes = envLightShadowCastersDeferred;

		CullCascadeMeshes(myEnvironmentalLight, result.myEnvironmentLightShadowCastersMeshes);


		// All the culled stuff, set them as passed the render check
		// TODO: Do for particle emitters and more
		{
			CPU_ZONE_NAMED(passedRenderCheckZone, "PassedRenderCheck true");

			for (auto& mesh : result.myDeferredMeshes)
			{
				mesh.myMeshInstance->GetModelInstance()->SetPassedRenderCheck(true);
			}
			for (auto& mesh : result.myForwardMeshes)
			{
				mesh.myMeshInstance->GetModelInstance()->SetPassedRenderCheck(true);
			}
			for (auto& mesh : result.myForwardTransparentMeshes)
			{
				mesh.myMeshInstance->GetModelInstance()->SetPassedRenderCheck(true);
			}
		}

		return result;
	}

	uint64_t GenerateSortKey()
	{
		return 0;
	}

	void RendererScene::CreateRenderCommandChain(
		const CulledSceneData& aSceneData, RenderCommandList& aResult) const
	{
		ZoneScopedN("Scene::CreateRenderCommandChain");

		aResult.Reset();

#if 0
		aResult.myMeshCommands.resize(aSceneData.myDeferredMeshes.size());

		auto& tp = GetEngine().GetParallelizationThreadPool();
		const int threadCount = tp.GetThreadCount();

		struct Range
		{
			int myStartIndex = 0;
			int myEndIndex = 0;
		};

		std::vector<std::future<void>> resultFutures;

		if (aSceneData.myDeferredMeshes.size() > 0)
		{
			int test = 0;
		}

		for (int i = 0; i < threadCount; ++i)
		{
			const int batch = aSceneData.myDeferredMeshes.size() / threadCount;

			int lastObjectsCount = 0;

			if (i == (threadCount - 1))
			{
				lastObjectsCount = aSceneData.myDeferredMeshes.size() - (batch * threadCount);
			}

			Range range;
			range.myStartIndex = i * batch;
			range.myEndIndex = (i + 1) * batch + lastObjectsCount;

			auto fut = tp.AddTask([range, &aSceneData, &aResult]()
				{
					// std::vector<MeshCommandEntry> result;

					for (int j = range.myStartIndex; j < range.myEndIndex; ++j)
					{
						const auto& mesh = aSceneData.myDeferredMeshes[j];

						const auto& model = mesh.myMeshInstance->GetModelInstance()->GetModel();

						const uint16_t modelID = model->GetID();

						const uint16_t effectPassID = mesh.myEffectPass->GetID();

						const uint16_t meshIndex = mesh.myMeshInstance->GetMeshIndex();

						MeshCommandEntry entry;

						entry.mySortKey.myEffectPassID = effectPassID;
						entry.mySortKey.myMeshIndex = meshIndex;
						entry.mySortKey.myModelID = modelID;

						entry.myMeshData = SingleMeshData2(mesh);

						aResult.myMeshCommands[j] = std::move(entry);
					}
				});

			resultFutures.push_back(std::move(fut));
		}

		for (auto& fut : resultFutures)
		{
			fut.wait();

			//const auto& result = fut.get();
			//
			//aResult.myMeshCommands.insert(
			//	aResult.myMeshCommands.begin(),
			//	result.begin(),
			//	result.end());
		}
#endif

#if 1
		size_t totalMeshCommands = 0;
		totalMeshCommands += aSceneData.myDeferredMeshComponents.size();
		totalMeshCommands += aSceneData.myDeferredMeshes.size();

		aResult.myMeshCommands.reserve(totalMeshCommands);

		// NOTE(filip): IMPORTANT, DO NOT COPY RESOURCE REFS
		// VERY POOR PERFORMANCE

		// Create commands from Deferred ModelInstances
		for (size_t i = 0; i < aSceneData.myDeferredMeshes.size(); ++i)
		{
			const auto& mesh = aSceneData.myDeferredMeshes[i];

			const auto& model = mesh.myMeshInstance->GetModelInstance()->GetModel();

			const uint16_t modelID = model->GetID();

			const uint16_t effectPassID = mesh.myEffectPass->GetID();

			const uint16_t meshIndex = mesh.myMeshInstance->GetMeshIndex();

			MeshCommandEntry entry;

			entry.mySortKey.myEffectPassID = effectPassID;
			entry.mySortKey.myMeshIndex = meshIndex;
			entry.mySortKey.myModelID = modelID;
			entry.mySortKey.myMaterialID = mesh.myMeshInstance->GetMaterialInstance().GetMaterialRef()->GetID();

			entry.myMeshData = SingleMeshData2(mesh);

			aResult.myMeshCommands.push_back(std::move(entry));
		}

		// Create mesh commands from Deferred Mesh Components
		for (size_t i = 0; i < aSceneData.myDeferredMeshComponents.size(); ++i)
		{
			const auto& mesh = aSceneData.myDeferredMeshComponents[i];

			const auto& model = mesh.myMeshComponent->GetModel();

			const uint16_t modelID = model->GetID();

			const uint16_t effectPassID = mesh.myEffectPass->GetID();

			const uint16_t meshIndex = mesh.myMeshComponent->GetMeshIndex();

			MeshCommandEntry entry;

			entry.mySortKey.myEffectPassID = effectPassID;
			entry.mySortKey.myMeshIndex = meshIndex;
			entry.mySortKey.myModelID = modelID;
			entry.mySortKey.myMaterialID = mesh.myMeshComponent->GetMaterialInstance().GetMaterialRef()->GetID();

			entry.myMeshData = SingleMeshData2(mesh);

			aResult.myMeshCommands.push_back(std::move(entry));
		}

		aResult.mySpotlightCommands.reserve(aSceneData.frustumCulledSpotLights.size());

		// Create spotlight commands
		for (size_t i = 0; i < aSceneData.frustumCulledSpotLights.size(); ++i)
		{
			const auto& culledSpot = aSceneData.frustumCulledSpotLights[i];

			SpotlightCommandEntry spotEntry;

			spotEntry.mySpotLight = culledSpot.mySpotLight;

			spotEntry.myShadowCasters.reserve(culledSpot.mySpotLightShadowCastersMeshes.size());

			// Add shadow casters
			for (size_t j = 0; j < culledSpot.mySpotLightShadowCastersMeshes.size(); ++j)
			{
				const auto& mesh = culledSpot.mySpotLightShadowCastersMeshes[j];

				const auto& model = mesh.myMeshInstance->GetModelInstance()->GetModel();

				const uint16_t modelID = model->GetID();

				const uint16_t effectPassID = mesh.myEffectPass->GetID();

				const uint16_t meshIndex = mesh.myMeshInstance->GetMeshIndex();

				MeshCommandEntry meshEntry;

				meshEntry.mySortKey.myEffectPassID = effectPassID;
				meshEntry.mySortKey.myMeshIndex = meshIndex;
				meshEntry.mySortKey.myModelID = modelID;

				meshEntry.myMeshData = SingleMeshData2(mesh);

				spotEntry.myShadowCasters.push_back(std::move(meshEntry));
			}

			std::sort(
				spotEntry.myShadowCasters.begin(),
				spotEntry.myShadowCasters.end(),
				MeshCommandEntry());

			aResult.mySpotlightCommands.push_back(std::move(spotEntry));
		}

		// Create pointlight commands
		for (size_t i = 0; i < aSceneData.frustumCulledPointLights.size(); ++i)
		{
			const auto& culledPoint = aSceneData.frustumCulledPointLights[i];

			PointLightCommandEntry pointEntry;

			pointEntry.myPointLight = culledPoint.myPointLight;

			for (int side = 0; side < 6; ++side)
			{
				pointEntry.myShadowCasters[side].reserve(culledPoint.myPointLightShadowCastersMeshes[side].size());

				// Add shadow casters
				for (size_t j = 0; j < culledPoint.myPointLightShadowCastersMeshes[side].size(); ++j)
				{
					const auto& mesh = culledPoint.myPointLightShadowCastersMeshes[side][j];

					const auto& model = mesh.myMeshInstance->GetModelInstance()->GetModel();

					const uint16_t modelID = model->GetID();

					const uint16_t effectPassID = mesh.myEffectPass->GetID();

					const uint16_t meshIndex = mesh.myMeshInstance->GetMeshIndex();

					MeshCommandEntry meshEntry;

					meshEntry.mySortKey.myEffectPassID = effectPassID;
					meshEntry.mySortKey.myMeshIndex = meshIndex;
					meshEntry.mySortKey.myModelID = modelID;

					meshEntry.myMeshData = SingleMeshData2(mesh);

					pointEntry.myShadowCasters[side].push_back(std::move(meshEntry));
				}

				std::sort(
					pointEntry.myShadowCasters[side].begin(),
					pointEntry.myShadowCasters[side].end(),
					MeshCommandEntry());
			}

			aResult.myPointLightCommands.push_back(std::move(pointEntry));
		}
#endif

		std::sort(
			aResult.myMeshCommands.begin(),
			aResult.myMeshCommands.end(),
			MeshCommandEntry());
	}

	Camera* RendererScene::GetMainCamera() const
	{
		return myMainCamera;
	}

	EnvironmentLightComponent* RendererScene::GetEnvironmentalLight() const
	{
		return myEnvironmentalLight;
	}

	void RendererScene::SetEnvironmentLightComponent(EnvironmentLightComponent* aEnvironmentLightComponent)
	{
		myEnvironmentalLight = aEnvironmentLightComponent;
	}

	void RendererScene::RenderSprite(SpriteComponent& aSprite)
	{
		if (aSprite.GetIs3D())
			my3DSprites.push_back(&aSprite);
		else
			mySpriteInstances.push_back(&aSprite);
	}

	void RendererScene::RenderText(Text& aText)
	{
		if (!aText.GetIs2D())
			myTexts.push_back(&aText);
		else
			my2DTexts.push_back(&aText);
	}

	void RendererScene::RenderDecal(DecalComponent& aDecal)
	{
		myDecals.push_back(&aDecal);
	}

	void RendererScene::RenderReflectionCapture(ReflectionCaptureComponent& aReflectionCapture)
	{
		myReflectionCaptureComponents.push_back(&aReflectionCapture);
	}

	bool RendererScene::CanRender() const
	{
		return myMainCamera &&
			myEnvironmentalLight &&
			myEnvironmentalLight->GetCubemap() &&
			myEnvironmentalLight->GetCubemap()->IsValid();
	}

	std::vector<ModelInstance*> RendererScene::CullModelsByDistanceFromSpotLight(
		const SpotLightComponent* aSpotLight,
		const std::vector<ModelInstance*> aModelInstances) const
	{
		std::vector<ModelInstance*> culledModelInstances;

		for (const auto& modelInstance : aModelInstances)
		{
			const float distanceSq = (modelInstance->GetTransformMatrix().GetTranslation()
				- aSpotLight->GetGameObject()->GetTransform().GetPosition())
				.LengthSqr();
			const float rangeSq = aSpotLight->GetRange() * aSpotLight->GetRange();

			if (distanceSq < rangeSq)
			{
				culledModelInstances.push_back(modelInstance);
			}
		}

		return culledModelInstances;
	}

	/*std::vector<ModelInstance*> RendererScene::CullModelsByDistanceFromPointLight(
		const PointLight* aPointLight, const std::vector<ModelInstance*> aModelInstances) const
	{
		std::vector<ModelInstance*> culledModelInstances;

		for (const auto& modelInstance : aModelInstances)
		{
			const float distanceSq
				= (modelInstance->GetTransformMatrix().GetTranslation() - aPointLight->GetPosition())
				.LengthSqr();
			const float rangeSq = aPointLight->GetRange() * aPointLight->GetRange();

			if (distanceSq < rangeSq)
			{
				culledModelInstances.push_back(modelInstance);
			}
		}

		return culledModelInstances;
	}*/

	std::vector<CulledMeshInstance> RendererScene::CullModelsByDistanceFromSpotLightMeshes(
		const SpotLightComponent* aSpotLight,
		const std::vector<CulledMeshInstance>& aMeshInstances) const
	{
		CPU_ZONE("RendererScene::CullModelsByDistanceFromSpotLightMeshes");

		std::vector<CulledMeshInstance> culledMeshInstances;

		for (const auto& culledMeshInstance : aMeshInstances)
		{
			const float distanceSq = (culledMeshInstance.myMeshInstance->GetModelInstance()
				->GetTransformMatrix()
				.GetTranslation()
				- aSpotLight->GetGameObject()->GetTransform().GetPosition())
				.LengthSqr();
			const float rangeSq = aSpotLight->GetRange() * aSpotLight->GetRange();

			if ((distanceSq - culledMeshInstance.myMeshInstance->GetModelInstance()->CalculateWorldBoundingSphere().GetRadius()) < rangeSq)
			{
				culledMeshInstances.push_back(culledMeshInstance);
			}
		}

		return culledMeshInstances;
	}

	std::vector<CulledMeshInstance> RendererScene::CullModelsByDistanceFromSpotLightMeshes2(
		const std::vector<Wrapped>& aWrappeds,
		const SpotLightComponent* aSpotLight) const
	{
		CPU_ZONE("RendererScene::CullModelsByDistanceFromSpotLightMeshes2");

		std::vector<CulledMeshInstance> culledMeshInstances;

		// NOTE(filip): Caching these values locally here gives an extreme performance boost
		const Vec3f spotLightPos = aSpotLight->GetGameObject()->GetTransform().GetPosition();
		const float spotLightRangeSq = aSpotLight->GetRange() * aSpotLight->GetRange();

#if 1
		for (const auto& wrapped : aWrappeds)
		{
			const float distanceSq = (wrapped.myPosition - spotLightPos).LengthSqr();

			if ((distanceSq - wrapped.myBoundingSphereRadius) > spotLightRangeSq)
			{
				continue;
			}

			const int meshCount = wrapped.myModelComponent->GetModelInstance().GetModel()->Get().GetMeshDatas().size();

			auto& meshInstanceList = wrapped.myModelComponent->GetModelInstance().GetMeshInstances();

			for (int i = 0; i < meshCount; ++i)
			{
				if (i
					>= meshInstanceList
					.size()) //Fixes krash where meshCount was bigger than meshInstanceList.size()
					break;

				auto& meshInstance = meshInstanceList[i];

				if (!meshInstance.GetMaterialInstance().IsValid())
				{
					continue;
				}

				for (auto& effectPass :
					meshInstance.GetMaterialInstance().GetMaterial()->GetPasses())
				{
					if (effectPass->GetRendererType() == RendererType::Forward)
					{
						CulledMeshInstance culledMeshInstance;
						culledMeshInstance.myEffectPass = effectPass.get();
						culledMeshInstance.myMeshInstance = &meshInstance;

						culledMeshInstances.push_back(culledMeshInstance);
					}
					else if (effectPass->GetRendererType() == RendererType::Deferred)
					{
						CulledMeshInstance culledMeshInstance;
						culledMeshInstance.myEffectPass = effectPass.get();
						culledMeshInstance.myMeshInstance = &meshInstance;

						culledMeshInstances.push_back(culledMeshInstance);
					}
					else
					{
						assert(false);
					}
				}
			}
		}
#else
		for (const auto& modelInstance : myModelInstancess)
		{
			if (!modelInstance->GetModel()->IsValid())
			{
				continue;
			}

			const float distanceSq = (modelInstance->GetTransformMatrix().GetTranslation()
				- aSpotLight->GetGameObject()->GetTransform().GetPosition())
				.LengthSqr();
			const float rangeSq = aSpotLight->GetRange() * aSpotLight->GetRange();

			if ((distanceSq - modelInstance->CalculateWorldBoundingSphere().GetRadius()) > rangeSq)
			{
				continue;
			}

			{
				// HACK: Since I did not have the meshcount initially
				const int meshCount = modelInstance->GetModel()->Get().GetMeshDatas().size();

				auto& meshInstanceList = modelInstance->GetMeshInstances();

				for (int i = 0; i < meshCount; ++i)
				{
					if (i
						>= meshInstanceList
						.size()) //Fixes krash where meshCount was bigger than meshInstanceList.size()
						break;

					auto& meshInstance = meshInstanceList[i];

					if (!meshInstance.GetMaterialInstance().IsValid())
					{
						continue;
					}

					for (auto& effectPass :
						meshInstance.GetMaterialInstance().GetMaterial()->GetPasses())
					{
						if (effectPass->GetRendererType() == RendererType::Forward)
						{
							CulledMeshInstance culledMeshInstance;
							culledMeshInstance.myEffectPass = effectPass.get();
							culledMeshInstance.myMeshInstance = &meshInstance;

							culledMeshInstances.push_back(culledMeshInstance);
						}
						else if (effectPass->GetRendererType() == RendererType::Deferred)
						{
							CulledMeshInstance culledMeshInstance;
							culledMeshInstance.myEffectPass = effectPass.get();
							culledMeshInstance.myMeshInstance = &meshInstance;

							culledMeshInstances.push_back(culledMeshInstance);
						}
						else
						{
							assert(false);
						}
					}
				}

				// modelInstance->SetPassedRenderCheck(true);
			}
		}
#endif

		return culledMeshInstances;

		//std::vector<CulledMeshInstance> culledMeshInstances;

		//for (const auto& culledMeshInstance : myModelInstancess)
		//{
		//	const float distanceSq = (culledMeshInstance.myMeshInstance->GetModelInstance()
		//		->GetTransformMatrix()
		//		.GetTranslation()
		//		- aSpotLight->GetGameObject()->GetTransform().GetPosition())
		//		.LengthSqr();
		//	const float rangeSq = aSpotLight->GetRange() * aSpotLight->GetRange();

		//	if ((distanceSq - culledMeshInstance.myMeshInstance->GetModelInstance()->CalculateWorldBoundingSphere().GetRadius()) < rangeSq)
		//	{
		//		culledMeshInstances.push_back(culledMeshInstance);
		//	}
		//}

		//return culledMeshInstances;
	}

	std::vector<CulledMeshInstance> RendererScene::CullMeshInstancesByFrustum(
		const Camera& aCamera,
		const std::vector<CulledMeshInstance>& aMeshInstances) const
	{
		std::vector<CulledMeshInstance> culledMeshInstances;

		for (const auto& meshInstance : aMeshInstances)
		{
			// TODO: must use IsCullingDisabled() as well
			if (/*meshInstance.myMeshInstance->GetModelInstance()->IsCullingDisabled() ||*/
				aCamera.GetViewFrustum().IsSphereInside(meshInstance.myMeshInstance->GetModelInstance()->CalculateWorldBoundingSphere()))
			{
				culledMeshInstances.push_back(meshInstance);
			}
		}

		return culledMeshInstances;
	}

	std::vector<CulledMeshInstance> RendererScene::CullModelsByDistanceFromPointLightMeshes(
		const PointLightComponent* aPointLight,
		const std::vector<CulledMeshInstance>& aMeshInstances) const
	{
		CPU_ZONE("RendererScene::CullModelsByDistanceFromPointLightMeshes");

		std::vector<CulledMeshInstance> culledMeshInstances;

		for (const auto& culledMeshInstance : aMeshInstances)
		{
			const float distanceSq = (culledMeshInstance.myMeshInstance->GetModelInstance()
				->GetTransformMatrix()
				.GetTranslation()
				- aPointLight->GetGameObject()->GetTransform().GetPosition())
				.LengthSqr();
			const float rangeSq = aPointLight->GetRange() * aPointLight->GetRange();

			if (distanceSq < rangeSq)
			{
				culledMeshInstances.push_back(culledMeshInstance);
			}
		}

		return culledMeshInstances;
	}

	PostProcessData& RendererScene::GetPostProcessData()
	{
		return myPostProcessData;
	}

	float RendererScene::GetShadowStrength() const
	{
		return myShadowStrength;
	}

	void RendererScene::SetShadowStrength(const float aShadowStrength)
	{
		myShadowStrength = aShadowStrength;
	}

	const CulledSceneData& RendererScene::GetCulledSceneData() const
	{
		return myCulledSceneData;
	}

	const RenderCommandList& RendererScene::GetRenderCommandChain() const
	{
		return myCommandList;
	}

	FullscreenTexture& RendererScene::GetResultOutputTexture()
	{
		if (!GetEngine().IsEditorEnabled())
		{
			if (myDebugOutput != RendererSceneDebugOutput::Nothing)
			{
				return myDebugGBufferResultTexture;
			}
		}

		return myOutputTextures[myAlreadyRendererTextureIndex];
	}

	FullscreenTexture& RendererScene::GetTextureToRenderTo()
	{
		return myOutputTextures[myRenderToTextureIndex];
	}

	FullscreenTexture& RendererScene::GetDebugGBufferTexture()
	{
		if (myDebugGBufferResultTexture.GetSRV() == nullptr)
		{
			const auto renderSizeui = GetEngine().GetGraphicsEngine().GetWindowHandler().GetRenderingSize();
			myDebugGBufferResultTexture = GetEngine().GetGraphicsEngine().GetFullscreenTextureFactory().CreateTexture(
				renderSizeui, DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM);
		}

		return myDebugGBufferResultTexture;
	}

	FullscreenTexture& RendererScene::GetSelectionTexture()
	{
		if (mySelectionResultTexture.GetSRV() == nullptr)
		{
			const auto renderSizeui = GetEngine().GetGraphicsEngine().GetWindowHandler().GetRenderingSize();
			mySelectionResultTexture = GetEngine().GetGraphicsEngine().GetFullscreenTextureFactory().CreateTexture(
				renderSizeui, DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT);
		}

		return mySelectionResultTexture;
	}

	UUID RendererScene::GetUUIDFromSelectionTexture(const Vec2f& aNormalizedCoordinates)
	{
		if (!mySelectionDestinationTexture)
		{
			std::cout << "Selection destination texture is nullptr" << std::endl;
		}

		ID3D11DeviceContext& context = *GetEngine().GetGraphicsEngine().GetDxFramework().GetContext();
		FullscreenTexture& texture = GetSelectionTexture();

		D3D11_TEXTURE2D_DESC textureDesc{};
		texture.GetTextureTest()->GetDesc(&textureDesc);

		const Vec2ui renderSizeui{ textureDesc.Width, textureDesc.Height };

		D3D11_BOX box{};
		box.left = static_cast<uint32_t>(aNormalizedCoordinates.x * renderSizeui.x);
		box.right = static_cast<uint32_t>(aNormalizedCoordinates.x * renderSizeui.x) + 1;
		box.top = static_cast<uint32_t>(aNormalizedCoordinates.y * renderSizeui.y);
		box.bottom = static_cast<uint32_t>(aNormalizedCoordinates.y * renderSizeui.y) + 1;
		box.back = 1;

		context.CopySubresourceRegion(mySelectionDestinationTexture, 0, 0, 0, 0, texture.GetTextureTest(), 0, &box);

		HRESULT result = 0;

		D3D11_MAPPED_SUBRESOURCE subresource{};
		result = context.Map(mySelectionDestinationTexture, 0, D3D11_MAP_READ, 0, &subresource);

		if (FAILED(result))
		{
			std::cout << "Failed to map selection destination texture" << std::endl;
		}

		UUID id = *reinterpret_cast<uint32_t*>(subresource.pData);

		context.Unmap(mySelectionDestinationTexture, 0);

		return id;
	}

	RendererSceneDebugOutput RendererScene::GetRendererDebugOutput() const
	{
		return myDebugOutput;
	}

	void RendererScene::SetRendererDebugOutput(const RendererSceneDebugOutput aRendererSceneDebugOutput)
	{
		myDebugOutput = aRendererSceneDebugOutput;
	}

	void RendererScene::Receive(const EventType aEventType, const std::any& aValue)
	{
		if (aEventType == EventType::WindowResized)
		{
			OnResized(std::any_cast<WindowResizedData>(aValue));
		}
	}

	void RendererScene::OnResized(const WindowResizedData& aResizeData)
	{
		CreateTextures();
	}

	void RendererScene::CreateTextures()
	{
		const auto renderSizeui = GetEngine().GetGraphicsEngine().GetWindowHandler().GetRenderingSize();

		// The following textures are directly used later on either
		// by drawing to backbuffer or drawing image in imgui
		// therefore tonemapping and shit like that has occured before
		// and we can use the 8-bit texture, no need for precision here

		myOutputTextures[0]
			= GetEngine().GetGraphicsEngine().GetFullscreenTextureFactory().CreateTexture(
				renderSizeui, DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM);

		myOutputTextures[1]
			= GetEngine().GetGraphicsEngine().GetFullscreenTextureFactory().CreateTexture(
				renderSizeui, DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM);

		D3D11_TEXTURE2D_DESC desc;
		myOutputTextures[0].GetTextureTest()->GetDesc(&desc);

		D3D11_TEXTURE2D_DESC desc2;
		myOutputTextures[1].GetTextureTest()->GetDesc(&desc2);

		D3D11_TEXTURE2D_DESC selectionDestinationTextureDesc{};
		selectionDestinationTextureDesc.Width = 1;
		selectionDestinationTextureDesc.Height = 1;
		selectionDestinationTextureDesc.MipLevels = 1;
		selectionDestinationTextureDesc.ArraySize = 1;
		selectionDestinationTextureDesc.Format = DXGI_FORMAT_R32_UINT;
		selectionDestinationTextureDesc.SampleDesc.Count = 1;
		selectionDestinationTextureDesc.Usage = D3D11_USAGE_STAGING;
		selectionDestinationTextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

		ID3D11Device& device = GetEngine().GetGraphicsEngine().GetDxFramework().GetDevice2();

		HRESULT result = 0;
		result = device.CreateTexture2D(&selectionDestinationTextureDesc, nullptr, &mySelectionDestinationTexture);

		if (FAILED(result))
		{
			std::cout << "Selection destination texture creation failed" << std::endl;
		}

		// mySelectionDestinationTexture
		// 	= GetEngine().GetGraphicsEngine().GetFullscreenTextureFactory().CreateTexture(
		// 			{1, 1}, DXGI_FORMAT_R32_UINT);

		int test = 0;

		// todo: continue with swapping the texture indexes after writing to one of them.
		// then write the MainScene texture to backbuffer, and the scenes within the Editor scene to imgui textures.
	}

	void RendererScene::SwapRenderTargets()
	{
		// God damn this is horrible

		if (myRenderToTextureIndex == 0)
		{
			myRenderToTextureIndex = 1;
		}
		else
		{
			myRenderToTextureIndex = 0;
		}

		if (myAlreadyRendererTextureIndex == 0)
		{
			myAlreadyRendererTextureIndex = 1;
		}
		else
		{
			myAlreadyRendererTextureIndex = 0;
		}
	}

	DebugDrawer& RendererScene::GetDebugDrawer()
	{
		return myDebugDrawer;
	}

	const RenderingStatistics& RendererScene::GetStats() const
	{
		return myStats;
	}

	void RendererScene::SetStats(const RenderingStatistics& aStats)
	{
		myStats = aStats;
	}

	const std::vector<ReflectionCapture>& RendererScene::GetReflectionCaptures() const
	{
		return myReflectionCaptures;
	}

	void RendererScene::Submit(const std::function<void()>& aFunction)
	{
		mySubmittedDXCalls.push_back(aFunction);
	}

	const std::vector<std::function<void()>>& RendererScene::GetSubmittedDXCalls() const
	{
		return mySubmittedDXCalls;
	}

	const Vec3f& RendererScene::GetPlayerPos() const
	{
		return myPlayerPos;
	}

	void RendererScene::SetPlayerPos(const Vec3f& aPlayerPos)
	{
		myPlayerPos = aPlayerPos;
	}

}
