#pragma once

#include "Engine/ResourceManagement/ResourceRef.h"
#include "Engine/Renderer/Lights/LightConstants.h"
#include "Engine/Renderer/Shaders/ShaderConstants.h"

namespace Engine
{
	class SpotLightComponent;
	class PointLightComponent;
	class EnvironmentLightComponent;
	class ModelInstance;
	class Directx11Framework;
	class Camera;
	class PointLight;
	class SpotLight;
	class ShaderRendererData;
	class RendererScene;
	class FrameBufferTempName;
	class MeshInstance;

	struct Times;
	struct CulledMeshInstance;

	class SelectionRenderer
	{
	public:
		SelectionRenderer(Directx11Framework& aFramework, FrameBufferTempName& aFrameBuffer, ShaderRendererData& aSharedRendererData);

		bool Init();
		void RenderModelInstances(std::vector<CulledMeshInstance>& aMeshInstances, const Camera* aMainCamera);
		void RenderMeshInstance(const Camera* aCamera, CulledMeshInstance& aCulledMeshInstance);

	private:
		Directx11Framework& myFramework;
		ShaderRendererData& mySharedRendererData;
		FrameBufferTempName& myFrameBuffer;

		ID3D11DeviceContext& myContext;
		ID3D11Device& myDevice;

		ID3D11Buffer* myObjectBuffer = nullptr;
		ID3D11Buffer* myAnimationBuffer = nullptr;

		PixelShaderRef mySelectionShader;
		ID3D11Buffer* mySelectionBuffer = nullptr;

		struct SelectionBufferData
		{
			int32_t myID;
			uint32_t padding1;
			uint32_t padding2;
			uint32_t padding3;
		};

		struct ObjectBuferData
		{
			Mat4f myToWorld;

			int myHasBones;
			int myTrash1;

			unsigned int myPointLightCount = 0;
			unsigned int mySpotLightCount = 0;

			struct PointLightData
			{
				C::Vector4f myPosition;
				C::Vector4f myColorAndIntensity;
				float myRange = 0.f;
				Vec3f garbage;
			} myPointLights[MaxPointLightCount] = {};

			struct SpotLightData
			{
				C::Vector4f myPosition;
				C::Vector4f myDirection;
				C::Vector4f myColorAndIntensity;
				float myRange = 0.f;
				float myInnerAngle = 0.f;
				float myOuterAngle = 0.f;
				float trash;
			} mySpotLights[MaxSpotLightCount] = {};
		} myObjectBufferData = {};

		struct AnimationBufferData
		{
			Mat4f myBones[ShaderConstants::MaxBoneCount];
		} myAnimationBufferData;
	};
}