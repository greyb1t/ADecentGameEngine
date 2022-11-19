#pragma once

#include "Engine/Renderer/Lights/LightConstants.h"
#include "../Statistics/RenderingStatistics.h"
#include "Engine/Renderer/ConstantBuffer.h"
#include "Engine/Renderer/Shaders/ShaderStructs.h"

namespace Engine
{
	class SpotLightComponent;
	class PointLightComponent;
	class EnvironmentLightComponent;
}

namespace Engine
{
	class ModelInstance;
	class Directx11Framework;
	class Camera;
	class PointLight;
	class SpotLight;
	struct Times;
	class ShaderRendererData;
	class RendererScene;
	class FrameBufferTempName;
	class MeshInstance;
	struct CulledMeshInstance;

	class ForwardRenderer
	{
	public:
		ForwardRenderer(
			Directx11Framework& aFramework, 
			FrameBufferTempName& aFrameBuffer, 
			ShaderRendererData& aSharedRendererData);

		bool Init();

		ForwardRenderingStatistics RenderModelInstances(
			EnvironmentLightComponent* aEnvironmentLight,
			RendererScene* aScene,
			std::vector<ModelInstance*> aModelInstances,
			std::vector<CulledMeshInstance>& aMeshInstances,
			const Camera* aMainCamera);

		ForwardRenderingStatistics RenderMeshInstance(
			EnvironmentLightComponent* aEnvironmentLight,
			const std::vector<PointLightComponent*> aPointLights,
			const std::vector<SpotLightComponent*> aSpotLights,
			const Camera* aCamera,
			CulledMeshInstance& aCulledMeshInstance);

	private:
		Directx11Framework& myFramework;
		ShaderRendererData& mySharedRendererData;
		FrameBufferTempName& myFrameBuffer;

		ID3D11DeviceContext& myContext;
		ID3D11Device& myDevice;

		// ID3D11Buffer* myObjectBuffer = nullptr;
		ConstantBuffer<ObjectBuferData_ForwardRenderer> myObjectBuffer;
		ID3D11Buffer* myAnimationBuffer = nullptr;
		ConstantBuffer<ShadowData_ForwardRenderer> myShadowDataBuffer;

		struct AnimationBufferData
		{
			Mat4f myBones[ShaderConstants::MaxBoneCount];
		} myAnimationBufferData;
	};
}
