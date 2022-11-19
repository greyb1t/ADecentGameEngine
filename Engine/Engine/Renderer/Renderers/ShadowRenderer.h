#pragma once

#include "..\Statistics\RenderingStatistics.h"
#include "Engine/ResourceManagement\ResourceRef.h"
#include "Renderer.h"
#include "Engine\Renderer\Shaders\ShaderStructs.h"
#include "Engine\Renderer\ConstantBuffer.h"

namespace Engine
{
	class ResourceManager;
}

namespace Engine
{
	class Directx11Framework;
	class ModelInstance;
	class FrameBufferTempName;
	class Camera;
	class MeshInstance;
	struct CulledMeshInstance;
	struct MeshCommandEntry;
	class EnvironmentLightComponent;

	class ShadowRenderer : public Renderer
	{
	public:
		ShadowRenderer(Directx11Framework& aFramework, FrameBufferTempName& aFrameBuffer);
		~ShadowRenderer();

		bool Init(ResourceManager& aResourceManager);

		ShadowRendererStatistics Render(
			const Camera& aCamera,
			const std::vector<CulledMeshInstance>& aMeshInstances);

		void Render(
			const Camera& aCamera,
			const std::vector<MeshCommandEntry>& aMeshCommands);

		ShadowRendererStatistics Render(EnvironmentLightComponent* aLight, const std::vector<std::vector<CulledMeshInstance>>& aMeshInstances);

#if 0
		ShadowRendererStatistics Render2(
			PointLight& aPointLight,
			const std::vector<CulledMeshInstance>& aMeshInstances);
#endif

	private:
		bool CreateObjectBuffer();
		bool CreateAnimationBuffer();
		bool CreateShadowCubeBuffer();

	private:

		struct ShadowCubeBufferData
		{
			// // All of the cameras have the same projection, therefore we only need one
			// CU::Matrix4f myProjection;

			// A ToCamera matrix for each cubemap side for the pointlight shadow casting
			Mat4f myToCamera[6];
		} myShadowCubeBufferData = {};

		struct CascadeFrameBuffer
		{
			Mat4f myToCascadeView{};
			Mat4f myToCascadeProjection{};
		};

		ID3D11Device& myDevice;
		ID3D11DeviceContext& myContext;
		FrameBufferTempName& myFrameBuffer;

		ConstantBuffer<ObjectBuferData_DeferredRenderer> myObjectBuffer;
		ConstantBuffer<AnimationBufferData> myAnimationBuffer;
		ConstantBuffer<CascadeFrameBuffer> myCascadeFrameBuffer;

		ID3D11Buffer* myShadowCubeBuffer = nullptr;

		ID3D11RasterizerState* myRasterizerState = nullptr;

		GeometryShaderRef myGeometryShader;
		VertexShaderRef myCascadeShader;
	};
}