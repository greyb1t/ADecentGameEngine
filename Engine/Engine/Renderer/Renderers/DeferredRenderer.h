#pragma once

#include "../Statistics/RenderingStatistics.h"
#include "../ResourceManagement/ResourceRef.h"
#include "Engine/Renderer/Scene/CulledSceneData.h"
#include "Renderer.h"
#include "Engine/Renderer/ConstantBuffer.h"
#include "Engine/Renderer/Shaders/ShaderStructs.h"

namespace Engine
{
	class SpotLightComponent;
	class PointLightComponent;
	class ResourceManager;
	class ResourceReferences;
	class EnvironmentLightComponent;
}

namespace Engine
{
	class RenderCommandList;
	class RenderManager;
	struct CulledMeshComponent;
	class ModelInstance;
	class Directx11Framework;
	class Camera;
	class PointLight;
	class SpotLight;
	class ShaderRendererData;
	class FrameBufferTempName;
	class MeshInstance;
	class FullscreenTexture;
	struct CulledMeshInstance;

	class DeferredRenderer : public Renderer
	{
	public:
		DeferredRenderer(
			Directx11Framework& aFramework,
			ResourceManager& aResourceManager,
			ResourceReferences& aResourceReferences,
			FrameBufferTempName& aFrameBuffer,
			VertexShaderRef aFullscreenVertexShader,
			ShaderRendererData& aSharedRendererData);

		bool Init();

		DeferredGBufferStats GenerateGBuffer(
			const Camera* aCamera,
			const std::vector<CulledMeshInstance>& aMeshInstances,
			const std::vector<CulledMeshComponent>& aMeshComponents,
			const RenderCommandList& aCommandList);

		DeferredGBufferStats RenderDecals(
			const Camera* aCamera,
			const std::vector<CulledDecalComponent>& aDecalComponents);

		DeferredRenderingStatistics Render(
			EnvironmentLightComponent* aEnvironmentLight,
			const std::vector<PointLightComponent*> aPointLights,
			const std::vector<SpotLightComponent*> aSpotLights,
			const Camera* aCamera);

		DeferredRenderingStatistics RenderPointLights(
			const std::vector<PointLightComponent*> aPointLights,
			const Camera& aCamera,
			RenderManager& aRenderManager);

		DeferredRenderingStatistics RenderSpotLights(
			const std::vector<SpotLightComponent*> aSpotLights,
			const Camera& aCamera,
			RenderManager& aRenderManager);

		static const ModelRef& GetDecalCubeModel();

	private:
		bool CreateGBufferPixelShader();
		bool CreateEnvironmentLightPixelShader();
		bool CreatePointLightPixelShader();
		bool CreateSpotLightPixelShader();
		void CreateInstanceBuffer();

	private:
		ResourceManager& myResourceManager;
		ResourceReferences& myResourceReferences;

		FrameBufferTempName& myFrameBuffer;

		ID3D11DeviceContext& myContext;
		ID3D11Device& myDevice;
		ShaderRendererData& mySharedRendererData;

		ID3D11Buffer* myEnvironmentLightBuffer = nullptr;
		ID3D11Buffer* myPointLightBuffer = nullptr;
		// ID3D11Buffer* mySpotLightBuffer = nullptr;
		ConstantBuffer<SpotLightBufferData> mySpotlightBuffer;

		ConstantBuffer<ObjectBuferData_DeferredRenderer> myObjectBuffer;

		ID3D11Buffer* myDecalObjectBuffer = nullptr;

		ConstantBuffer<AnimationBufferData> myAnimationBuffer;

		ID3D11Buffer* myInstanceBuffer = nullptr;
		ID3D11ShaderResourceView* myInstanceBufferSRV = nullptr;

		PixelShaderRef myGBufferPixelShader;

		PixelShaderRef myEnvironmentLightShader;
		PixelShaderRef myPointLightShader;
		PixelShaderRef mySpotLightShader;

		VertexShaderRef myFullscreenShader;

		// For decals
		static inline ModelRef myCubeModel;

		struct InstanceBufferData
		{
			Mat4f myToWorld;
			// No HasBones here because cannot instance skinned models because
			// max size of a structured buffer stride is too small (2048)
		} myInstanceBufferData = {};

		struct DecalObjectBufferData
		{
			Mat4f myToInverseWorld;
			Mat4f myToWorld;

			Vec2f myUVScale;
			int32_t myHasAlbedoTexture = 0;
			int32_t myHasMaterialTexture = 0;

			int32_t myHasNormalTexture = 0;
			int32_t myHasEmissiveTexture = 0;
			Vec2f trashyBitch;
		} myDecalObjectBufferData = {};

		struct EnvironmentLightBufferData
		{
			std::array<Mat4f, 4> myLightView;
			std::array<Mat4f, 4> myLightProjection;
			C::Vector4f myToDirectionalLightDirection;
			C::Vector4f myDirectionalLightColorAndIntensity;
			//CU::Vector4f myDirectionalLightColor;
			// unsigned int myEnvironmentLightMipCount;
			// unsigned int myPadding[3];
		} myEnvironmentLightBufferData = {};

		struct PointLightBufferData
		{
			Mat4f myLightView[6];
			Mat4f myLightProjection;
			C::Vector4f myPosition;
			C::Vector4f myColorAndIntensity;
			float myRange = 0.f;
			Vec3f garbage;
			int myIsCastingShadows = 0;
			int myShit[3] = {};
		} myPointLightBufferData = {};
	};
}