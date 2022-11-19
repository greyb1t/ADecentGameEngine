#pragma once

#include "Engine/Renderer/Lights/LightConstants.h"
#include "Engine\Renderer\RendererType.h"
#include "CulledSceneData.h"
#include "../Utils/Observer.h"
#include "../Texture/FullscreenTexture.h"
#include "Engine/DebugManager/DebugDrawer.h"
#include "Engine/Renderer/PostProcessing/PostProcessData.h"
#include "Engine/Renderer/Sorting/RenderCommandList.h"
#include "Engine/Renderer/RendererSceneDebugOutput.h"
#include "Engine/Renderer/Statistics/RenderingStatistics.h"

namespace Engine
{
	class SpriteComponent;
	class SpotLightComponent;
	class PointLightComponent;
	class MeshComponent;
	class ModelComponent;
	class DecalComponent;
	class EnvironmentLightComponent;
	class ParticleEmitterComponent;
	class ReflectionCaptureComponent;
}

namespace Engine
{
	struct MeshRenderCommand;
	class ModelInstance;
	class Camera;
	class PointLight;
	class SpotLight;
	class ParticleEmitterInstance;
	class Text;
	struct CulledMeshInstance;
	class CubemapTexture;

	struct Wrapped
	{
		Vec3f myPosition;
		//C::Sphere<float> myBoundingSphere;
		float myBoundingSphereRadius;
		ModelComponent* myModelComponent;
	};

	struct ReflectionCapture
	{
		Vec3f myPosition;
		FullscreenTextureCube* myCubemapTexture = nullptr;
		CubemapTexture* myCubemapTextureSmooth = nullptr;

		struct CameraSide
		{
			int mySideIndex = -1;
			Camera* myCamera = nullptr;
			CulledSceneData myCulledSceneData;
		};

		std::array<CameraSide, 6> myCameraSides;
	};

	class RendererScene : public Observer
	{
	public:
		RendererScene() = default;
		~RendererScene();
		RendererScene(const RendererScene&) = default;

		bool Init();

		void ClearRenderCommands();

		void LateUpdate(const float aDeltaTime);

		void PrepareRendering(Camera& aMainCamera, EnvironmentLightComponent& aEnvironmentLight);

		void RenderModelInstance(ModelComponent& aModelComponent);
		void RenderMeshInstance(MeshComponent& aMeshComponent);
		void RenderPointLight(PointLightComponent& aPointLightComponent);
		void RenderSpotLight(SpotLightComponent& aSpotLight);
		void RenderParticleEmitter(ParticleEmitterComponent& aParticleEmitterComponent);
		void RenderSprite(SpriteComponent& aSprite);
		void RenderText(Text& aText);
		void RenderDecal(DecalComponent& aDecal);
		void RenderReflectionCapture(ReflectionCaptureComponent& aReflectionCapture);

		void CullForwardModelsMeshes(
			const Camera& aCamera,
			const bool aMustCastShadows,
			std::vector<CulledMeshInstance>& aDeferredMeshesOut,
			std::vector<CulledMeshInstance>& aForwardMeshesOut,
			std::vector<CulledMeshInstance>& aForwardTransparentMeshesOut) const;

		void CullCascadeMeshes(EnvironmentLightComponent* aLight, std::vector<std::vector<CulledMeshInstance>>& outMeshes) const;

		void CullMeshComponents(
			const Camera& aCamera,
			const bool aMustCastShadows,
			std::vector<CulledMeshComponent>& aDeferredMeshesOut,
			std::vector<CulledMeshComponent>& aForwardMeshesOut,
			std::vector<CulledMeshComponent>& aForwardTransparentMeshesOut) const;

		void CullForwardModelsMeshesByDistance(
			const PointLightComponent* aPointLight,
			const bool aMustCastShadows,
			std::vector<CulledMeshInstance>& aDeferredMeshesOut,
			std::vector<CulledMeshInstance>& aForwardMeshesOut) const;

		void CullDecals(
			const Camera& aCamera,
			std::vector<CulledDecalComponent>& aCulledDecals) const;

		std::vector<SpotLightComponent*> FrustumCullSpotLights(const Camera* aCamera) const;
		std::vector<PointLightComponent*> FrustumCullPointLights(const Camera* aCamera) const;

		std::vector<PointLightComponent*> CullPointLights(const ModelInstance* aModelInstance) const;
		std::vector<SpotLightComponent*> CullSpotLights(const ModelInstance* aModelInstance) const;
		std::vector<ParticleEmitterComponent*> CullEmitters(const Camera* aCamera) const;
		std::vector<SpriteComponent*> CullSpriteInstances(const Camera* aCamera) const;
		std::vector<Text*> CullTexts(const Camera* aCamera) const;
		std::vector<ReflectionCaptureComponent*> CullReflectionCaptures(const Camera* aCamera) const;

		CulledSceneData CullEverything(const Camera& aCamera) const;
		void CreateRenderCommandChain(const CulledSceneData& aSceneData, RenderCommandList& aResult) const;

		Camera* GetMainCamera() const;

		EnvironmentLightComponent* GetEnvironmentalLight() const;
		void SetEnvironmentLightComponent(EnvironmentLightComponent* aEnvironmentLightComponent);

		bool CanRender() const;

		std::vector<ModelInstance*> CullModelsByDistanceFromSpotLight(
			const SpotLightComponent* aSpotLight,
			const std::vector<ModelInstance*> aModelInstances) const;

		// std::vector<ModelInstance*> CullModelsByDistanceFromPointLight(
		// 	const PointLight* aPointLight,
		// 	const std::vector<ModelInstance*> aModelInstances) const;

		// TODO: These use the whole model position and not each mesh positino
		// can therefore give bad results
		std::vector<CulledMeshInstance> CullModelsByDistanceFromSpotLightMeshes(
			const SpotLightComponent* aSpotLight,
			const std::vector<CulledMeshInstance>& aMeshInstances) const;

		std::vector<CulledMeshInstance> CullModelsByDistanceFromSpotLightMeshes2(
			const std::vector<Wrapped>& aWrappeds,
			const SpotLightComponent* aSpotLight) const;

		std::vector<CulledMeshInstance> CullMeshInstancesByFrustum(
			const Camera& aCamera,
			const std::vector<CulledMeshInstance>& aMeshInstances) const;

		std::vector<CulledMeshInstance> CullModelsByDistanceFromPointLightMeshes(
			const PointLightComponent* aPointLight,
			const std::vector<CulledMeshInstance>& aMeshInstances) const;

		PostProcessData& GetPostProcessData();

		float GetShadowStrength() const;
		void SetShadowStrength(const float aShadowStrength);

		const CulledSceneData& GetCulledSceneData() const;
		const RenderCommandList& GetRenderCommandChain() const;

		FullscreenTexture& GetResultOutputTexture();
		FullscreenTexture& GetTextureToRenderTo();

		FullscreenTexture& GetDebugGBufferTexture();

		FullscreenTexture& GetSelectionTexture();
		UUID GetUUIDFromSelectionTexture(const Vec2f& aNormalizedCoordinates);

		RendererSceneDebugOutput GetRendererDebugOutput() const;
		void SetRendererDebugOutput(const RendererSceneDebugOutput aRendererSceneDebugOutput);

		void SwapRenderTargets();

		DebugDrawer& GetDebugDrawer();

		const RenderingStatistics& GetStats() const;
		void SetStats(const RenderingStatistics& aStats);

		const std::vector<ReflectionCapture>& GetReflectionCaptures() const;

		void Submit(const std::function<void()>& aFunction);
		const std::vector<std::function<void()>>& GetSubmittedDXCalls() const;

		const Vec3f &GetPlayerPos() const;
		void SetPlayerPos(const Vec3f& aPlayerPos);

	private:
		void Receive(const EventType aEventType, const std::any& aValue) override;
		void OnResized(const WindowResizedData& aResizeData);
		void CreateTextures();

	private:
		Camera* myMainCamera = nullptr;

		std::vector<std::function<void()>> mySubmittedDXCalls;

		//FogData myFogData;

		PostProcessData myPostProcessData;

		// 0 is no shadow, 1 is full shadow
		float myShadowStrength = 1.f;

		EnvironmentLightComponent* myEnvironmentalLight = nullptr;

		std::vector<ModelComponent*> myModelInstancess;
		std::vector<MeshComponent*> myMeshComponents;

		std::vector<PointLightComponent*> myPointLights;
		std::vector<SpotLightComponent*> mySpotLights;

		std::vector<ParticleEmitterComponent*> myParticleEmitters;

		std::vector<SpriteComponent*> mySpriteInstances;
		std::vector<SpriteComponent*> my3DSprites;

		std::vector<Text*> myTexts;
		std::vector<Text*> my2DTexts;

		std::vector<DecalComponent*> myDecals;
		std::vector<ReflectionCaptureComponent*> myReflectionCaptureComponents;

		CulledSceneData myCulledSceneData;
		RenderCommandList myCommandList;

		std::vector<ReflectionCapture> myReflectionCaptures;

		int myRenderToTextureIndex = 0;
		int myAlreadyRendererTextureIndex = 1;

		std::array<FullscreenTexture, 2> myOutputTextures;

		RendererSceneDebugOutput myDebugOutput = RendererSceneDebugOutput::Nothing;
		FullscreenTexture myDebugGBufferResultTexture;

		FullscreenTexture mySelectionResultTexture;
		ID3D11Texture2D* mySelectionDestinationTexture = nullptr;

		DebugDrawer myDebugDrawer;

		RenderingStatistics myStats;

		Vec3f myPlayerPos;
	};
}