#include "pch.h"
#include "ShadowRenderer.h"
#include "Engine/Renderer/Camera/Camera.h"
#include "Engine/Renderer/Model/ModelInstance.h"
#include "Engine/Renderer/Directx11Framework.h"
#include "Engine/ResourceManagement/ResourceThreadContext.h"
#include "Engine/ResourceManagement/Resources/VertexShaderResource.h"
#include "Engine/ResourceManagement/Resources/ModelResource.h"
#include "Engine/ResourceManagement/Resources/PixelShaderResource.h"
#include "Engine/Renderer/Shaders/VertexShader.h"
#include "Engine/Renderer/Shaders/GeometryShader.h"
#include "Engine/Renderer/Model/Model.h"
#include "Engine/Renderer/Material/MeshMaterial.h"
#include "Engine/Renderer/FrameConstantBuffer.h"
#include "Engine/Renderer/Utils/DxUtils.h"
#include "Engine/Renderer/Material/EffectPass.h"
#include "Engine/Renderer/Scene/CulledSceneData.h"
#include "Engine/ResourceManagement/Resources/GeometryShaderResource.h"
#include "Engine/Renderer/Sorting/RenderCommandList.h"
#include "Engine/Renderer/Shaders/ShaderConstants.h"
#include "../GameObject/Components/EnvironmentLightComponent.h"
#include "../Statistics/RenderingStatistics.h"
#include "../TracyProfilingGPU.h"

namespace Engine
{
	ShadowRenderer::ShadowRenderer(Directx11Framework& aFramework, FrameBufferTempName& aFrameBuffer)
		: Renderer(*aFramework.GetContext()),
		myFrameBuffer(aFrameBuffer),
		myDevice(*aFramework.GetDevice()),
		myContext(*aFramework.GetContext())
	{

	}

	ShadowRenderer::~ShadowRenderer()
	{
	}

	bool ShadowRenderer::Init(ResourceManager& aResourceManager)
	{
		if (!CreateObjectBuffer())
		{
			return false;
		}

		if (!CreateAnimationBuffer())
		{
			return false;
		}

		if (!CreateShadowCubeBuffer())
		{
			return false;
		}

		myGeometryShader =
			aResourceManager.CreateRef<GeometryShaderResource>(
				"Assets/Shaders/Engine/Deferred/ShadowCubeGS");
		myGeometryShader->RequestLoading();

		myCascadeShader = aResourceManager.CreateRef<VertexShaderResource>("Assets/Shaders/Engine/Shadows/CascadeShadow.vs", DefaultVertex::ourInputElements, DefaultVertex::ourElementCount);

		// myCascadeShader->RequestLoading();
		// NOTE(filip): retail ful fix
		myCascadeShader->Load();


		myCascadeFrameBuffer.InitDynamicWritable();

		D3D11_RASTERIZER_DESC rasterizerDesc{};
		rasterizerDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerDesc.CullMode = D3D11_CULL_NONE;
		rasterizerDesc.DepthClipEnable = true;

		HRESULT result = myDevice.CreateRasterizerState(&rasterizerDesc, &myRasterizerState);
		assert(SUCCEEDED(result));

		return true;
	}

	ShadowRendererStatistics ShadowRenderer::Render(
		const Camera& aCamera,
		const std::vector<CulledMeshInstance>& aCulledMeshInstances)
	{
		ZoneNamedN(zone1, "ShadowRenderer::Render", true);

		ShadowRendererStatistics stats;

		HRESULT result = 0;

		START_TIMER(shadowRenderingTimer);

		myFrameBuffer.UpdateCamera(aCamera);

		// D3D11_MAPPED_SUBRESOURCE bufferData;

		for (const auto& culledMeshInstance : aCulledMeshInstances)
		{
			MeshInstance* meshInstance = culledMeshInstance.myMeshInstance;
			ModelInstance* modelInstance = meshInstance->GetModelInstance();

			const auto& fbxResource = modelInstance->GetModel();

			if (!fbxResource->IsValid())
			{
				continue;
			}

			//if (!model.GetModelData().myMaterial->GetVertexShader())
			//{
			//	continue;
			//}

			//if (!model.GetModelData().myMaterial->GetPixelShader())
			//{
			//	continue;
			//}

			//if (!model.GetModelData().myMaterial->GetVertexShader()->IsValid())
			//{
			//	continue;
			//}

			//if (!model.GetModelData().myMaterial->GetPixelShader()->IsValid())
			//{
			//	continue;
			//}

			// TODO: We can use polymorphism better here by moving the code into a function
			// called PrepareForRendering() into ModelInstance, then we don't need the if check here
			// myAnimationBufferData = {};

			if (fbxResource->Get().IsSkinned())
			{
				const auto& finalTransforms = modelInstance->GetFinalAnimTransforms();

				AnimationBufferData& animData = myAnimationBuffer.Map(myContext);

				memcpy(
					animData.myBones,
					&finalTransforms[0],
					sizeof(Mat4f) * finalTransforms.size());

				myAnimationBuffer.Unmap(myContext);


				//myAnimationBufferData.OB_HasBones = modelInstance->HasBones();

				//// Map the bones to VS shader
				//{
				//	D3D11_MAPPED_SUBRESOURCE bufferData2;

				//	ZeroMemory(&bufferData2, sizeof(bufferData2));
				//	result = myContext.Map(myAnimationBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferData2);
				//	if (FAILED(result))
				//	{
				//		ERROR_LOG("Context.Map() failed");
				//		return;
				//	}
				//	memcpy(bufferData2.pData, &myAnimationBufferData, sizeof(myAnimationBufferData));
				//	myContext.Unmap(myAnimationBuffer, 0);
				//}

				//// animation is on cbuffer slot 2
				//myContext.VSSetConstantBuffers(2, 1, &myAnimationBuffer);
			}

			/*
			// Map the bones to VS shader
			{
				D3D11_MAPPED_SUBRESOURCE bufferData2;

				ZeroMemory(&bufferData2, sizeof(bufferData2));
				result = myContext.Map(myAnimationBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferData2);
				ThrowIfFailed(result);
				memcpy(bufferData2.pData, &myAnimationBufferData, sizeof(myAnimationBufferData));
				myContext.Unmap(myAnimationBuffer, 0);
			}
			*/

			// animation is on cbuffer slot 2
			myContext.VSSetConstantBuffers(2, 1, myAnimationBuffer.GetAddressOf());


			auto vertexBuffer = fbxResource->Get().GetVertexBuffer();
			auto indexBuffer = fbxResource->Get().GetIndexBuffer();
			auto stride = fbxResource->Get().GetStride();
			auto offset = fbxResource->Get().GetOffset();

			// Update object buffer
			{
				/*
				myObjectBufferData.myToWorld = modelInstance->GetTransformMatrix();
				myObjectBufferData.myHasBones = modelInstance->HasBones();

				bufferData = {};

				result = myContext.Map(myObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferData);
				ThrowIfFailed(result);

				memcpy(bufferData.pData, &myObjectBufferData, sizeof(myObjectBufferData));
				myContext.Unmap(myObjectBuffer, 0);
				*/

				ObjectBuferData_DeferredRenderer& data = myObjectBuffer.Map(myContext);

				data = { };
				data.myToWorld = modelInstance->GetTransformMatrix();
				data.myHasBones = modelInstance->HasBones();

				myObjectBuffer.Unmap(myContext);
			}

			myContext.IASetPrimitiveTopology(fbxResource->Get().myPrimitiveTopology);

			myContext.IASetVertexBuffers(0, 1, &vertexBuffer, stride, offset);
			myContext.IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

			myContext.VSSetConstantBuffers(1, 1, myObjectBuffer.GetAddressOf());

			myContext.GSSetShader(nullptr, nullptr, 0);
			myContext.PSSetShader(nullptr, nullptr, 0);

			const auto& meshDatas = fbxResource->Get().GetMeshDatas();

			// for (int meshIndex = 0; meshIndex < meshDatas.size(); ++meshIndex)
			{
				const int meshIndex = meshInstance->GetMeshIndex();
				const auto& meshData = meshDatas[meshIndex];

				//for (auto& pass : material.GetEffect()->GetPasses())
				EffectPass* pass = culledMeshInstance.myEffectPass;
				{
					if (!pass->GetVertexShader())
					{
						continue;
					}

					if (!pass->GetPixelShader())
					{
						continue;
					}

					if (!pass->GetVertexShader()->IsValid())
					{
						continue;
					}

					if (!pass->GetPixelShader()->IsValid())
					{
						continue;
					}

					myContext.IASetInputLayout(pass->GetVertexShader()->Get().GetInputLayout());
					myContext.VSSetShader(pass->GetVertexShader()->Get().GetVertexShader(), nullptr, 0);

					EffectPassState state = pass->BindToPipeline(&myContext,
						BindEffectFlags_VertexShader);

					myContext.DrawIndexed(
						meshData.myIndicesCount,
						meshData.myStartIndexLocation,
						meshData.myBaseVertexLocation);

					state.ResetState(myContext);

					stats.myDrawCalls++;
				}
			}
		}

		stats.myRenderTime = END_TIMER_GET_RESULT_MS(shadowRenderingTimer);

		return stats;
	}


	void ShadowRenderer::Render(
		const Camera& aCamera,
		const std::vector<MeshCommandEntry>& aMeshCommands)
	{
		ZoneNamedN(zone1, "ShadowRenderer::Render Newone", true);

		ShadowRendererStatistics stats;

		HRESULT result = 0;

		START_TIMER(shadowRenderingTimer);

		myFrameBuffer.UpdateCamera(aCamera);

		ResetInternalBindState();

		// No gs or ps shaders will be used when rendering shadows, only geometry
		myContext.GSSetShader(nullptr, nullptr, 0);
		myContext.PSSetShader(nullptr, nullptr, 0);

#if 1
		const size_t cmdListSize = aMeshCommands.size();

		for (size_t i = 0; i < cmdListSize;)
		{
			const auto& command = aMeshCommands[i];
			const auto& meshData = command.myMeshData;

			BindEffectPass(*meshData.myPass, BindEffectFlags_VertexShader);

			if (meshData.myPass->IsInstanced())
			{
				assert(false && "unimplemented");
			}
			else
			{
				BindModelMesh(
					*meshData.myModel,
					command.mySortKey.myModelID,
					meshData.myMeshIndex,
					BindModelFlags_None);

				ObjectBuferData_DeferredRenderer& data = myObjectBuffer.Map(myContext);

				data.myToWorld = meshData.myTransform;
				data.myHasBones = meshData.myHasSkeleton ? 1 : 0;

				myObjectBuffer.Unmap(myContext);

				BindConstantBufferVS(
					ShaderConstants::ObjectConstantBufferSlot, 
					myObjectBuffer);

				if (meshData.myHasSkeleton)
				{
					AnimationBufferData& animData = myAnimationBuffer.Map(myContext);

					memcpy(
						animData.myBones,
						&meshData.myAnimTransforms[0],
						sizeof(animData.myBones));

					myAnimationBuffer.Unmap(myContext);
					BindConstantBufferVS(
						ShaderConstants::AnimationConstantBufferSlot,
						myAnimationBuffer);
				}

				const auto& modelMesh = meshData.myModel->GetMeshDatas()[meshData.myMeshIndex];

				myContext.DrawIndexed(
					modelMesh.myIndicesCount,
					modelMesh.myStartIndexLocation,
					modelMesh.myBaseVertexLocation);

				++i;
			}
		}
#endif
	}

	ShadowRendererStatistics ShadowRenderer::Render(EnvironmentLightComponent* aLight, const std::vector<std::vector<CulledMeshInstance>>& aMeshInstances)
	{
		ZoneNamedN(zone1, "ShadowRenderer::EnvironmentlightCascades", true);
		ShadowRendererStatistics stats;

		HRESULT result = 0;

		START_TIMER(shadowRenderingTimer);

		myContext.GSSetShader(nullptr, nullptr, 0);
		myContext.PSSetShader(nullptr, nullptr, 0);
		myContext.VSSetShader(myCascadeShader->Get().GetVertexShader(), nullptr, 0);

		for (int cascadeIndex = 0; cascadeIndex < aLight->GetNumCascades(); ++cascadeIndex)
		{
			ZoneNamedN(zone2, "Cascade", true);
			GPU_ZONE_NAMED(envLightShadowZone2, "Cascade");
			auto& shadowMap = aLight->GetCascadeTexture(cascadeIndex);
			shadowMap.ClearDepth();
			shadowMap.SetAsActiveDepth();

			auto& buffer = myCascadeFrameBuffer.Map(myContext);
			buffer.myToCascadeView = Mat4f::GetFastInverse(aLight->GetCascadeView(cascadeIndex));
			buffer.myToCascadeProjection = aLight->GetCascadeProjection(cascadeIndex);
			myCascadeFrameBuffer.Unmap(myContext);
			myContext.VSSetConstantBuffers(0, 1, myCascadeFrameBuffer.GetAddressOf());
			
			for (const auto& culledMeshInstance : aMeshInstances[cascadeIndex])
			{
				MeshInstance* meshInstance = culledMeshInstance.myMeshInstance;
				ModelInstance* modelInstance = meshInstance->GetModelInstance();

				const auto& fbxResource = modelInstance->GetModel();
				if (!fbxResource->IsValid())
					continue;

				if (fbxResource->Get().IsSkinned())
				{
					const auto& finalTransforms = modelInstance->GetFinalAnimTransforms();

					AnimationBufferData& animData = myAnimationBuffer.Map(myContext);
					memcpy(animData.myBones, &finalTransforms[0], sizeof(Mat4f) * finalTransforms.size());
					myAnimationBuffer.Unmap(myContext);
				}
				myContext.VSSetConstantBuffers(2, 1, myAnimationBuffer.GetAddressOf());
				
				auto vertexBuffer = fbxResource->Get().GetVertexBuffer();
				auto indexBuffer = fbxResource->Get().GetIndexBuffer();
				auto stride = fbxResource->Get().GetStride();
				auto offset = fbxResource->Get().GetOffset();

				ObjectBuferData_DeferredRenderer& data = myObjectBuffer.Map(myContext);
				data = { };
				data.myToWorld = modelInstance->GetTransformMatrix();
				data.myHasBones = modelInstance->HasBones();
				myObjectBuffer.Unmap(myContext);

				myContext.IASetPrimitiveTopology(fbxResource->Get().myPrimitiveTopology);
				myContext.IASetVertexBuffers(0, 1, &vertexBuffer, stride, offset);
				myContext.IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

				myContext.VSSetConstantBuffers(1, 1, myObjectBuffer.GetAddressOf());

				const auto& meshDatas = fbxResource->Get().GetMeshDatas();

				const int meshIndex = meshInstance->GetMeshIndex();
				const auto& meshData = meshDatas[meshIndex];

				//EffectPass* pass = culledMeshInstance.myEffectPass;
				//{
				//	if (!pass->GetVertexShader())
				//	{
				//		continue;
				//	}
				//
				//	if (!pass->GetPixelShader())
				//	{
				//		continue;
				//	}
				//
				//	if (!pass->GetVertexShader()->IsValid())
				//	{
				//		continue;
				//	}
				//
				//	if (!pass->GetPixelShader()->IsValid())
				//	{
				//		continue;
				//	}
				//
				//	myContext.IASetInputLayout(pass->GetVertexShader()->Get().GetInputLayout());
				//
				//	// Cant use this as we got our custom cascade vertex shader. This will most likely be a problem in the future but idgaf
				//	//myContext.VSSetShader(pass->GetVertexShader()->Get().GetVertexShader(), nullptr, 0);
				//
				//	EffectPassState state = pass->BindToPipeline(&myContext, BindEffectFlags_None);
					myContext.IASetInputLayout(myCascadeShader->Get().GetInputLayout());

					myContext.RSSetState(myRasterizerState);

					myContext.DrawIndexed(
						meshData.myIndicesCount,
						meshData.myStartIndexLocation,
						meshData.myBaseVertexLocation);

				///	state.ResetState(myContext);

					stats.myDrawCalls++;
				//}
			}
		}

		stats.myRenderTime = END_TIMER_GET_RESULT_MS(shadowRenderingTimer);
		return stats;
	}

	// Removed bcuz I removed class PointLight, but still need code to point light optimization in 1 draw call
#if 0
	Engine::ShadowRendererStatistics ShadowRenderer::Render2(
		PointLight& aPointLight,
		const std::vector<CulledMeshInstance>& aMeshInstances)
	{
		ShadowRendererStatistics stats;
		HRESULT result = 0;

		START_TIMER(shadowRenderingTimer);

		// We only needto update with the first camera, because the projection matrix
		// for all sides are the same
		myFrameBuffer.UpdateCamera(aPointLight.GetCameras()[0]);

		// Update the shadow cube buffer
		{
			for (int i = 0; i < 6; ++i)
			{
				const auto temp = aPointLight.GetCameras()[i].GetTransform().ToMatrix();

				myShadowCubeBufferData.myToCamera[i] =
					Mat4f::GetFastInverse(
						temp);
			}

			D3D11_MAPPED_SUBRESOURCE bufferData2;

			ZeroMemory(&bufferData2, sizeof(bufferData2));
			result = myContext.Map(myShadowCubeBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferData2);
			ThrowIfFailed(result);
			memcpy(bufferData2.pData, &myShadowCubeBufferData, sizeof(myShadowCubeBufferData));
			myContext.Unmap(myShadowCubeBuffer, 0);
		}

		D3D11_MAPPED_SUBRESOURCE bufferData;

		for (const auto& culledMeshInstance : aMeshInstances)
		{
			MeshInstance* meshInstance = culledMeshInstance.myMeshInstance;
			ModelInstance* modelInstance = meshInstance->GetModelInstance();

			const auto& fbxResource = modelInstance->GetModel();

			if (!fbxResource->IsValid())
			{
				continue;
			}

			//if (!model.GetModelData().myMaterial->GetVertexShader())
			//{
			//	continue;
			//}

			//if (!model.GetModelData().myMaterial->GetPixelShader())
			//{
			//	continue;
			//}

			//if (!model.GetModelData().myMaterial->GetVertexShader()->IsValid())
			//{
			//	continue;
			//}

			//if (!model.GetModelData().myMaterial->GetPixelShader()->IsValid())
			//{
			//	continue;
			//}

			// TODO: We can use polymorphism better here by moving the code into a function
			// called PrepareForRendering() into ModelInstance, then we don't need the if check here
			myAnimationBufferData = { };

			if (fbxResource->Get().IsSkinned())
			{
				const auto& finalTransforms = modelInstance->GetFinalAnimTransforms();

				memcpy(
					myAnimationBufferData.myBones,
					finalTransforms.data(),
					sizeof(Mat4f) * finalTransforms.size());

				// myAnimationBufferData.OB_HasBones = modelInstance->HasBones();

				//// Map the bones to VS shader
				//{
				//	D3D11_MAPPED_SUBRESOURCE bufferData2;

				//	ZeroMemory(&bufferData2, sizeof(bufferData2));
				//	result = myContext.Map(myAnimationBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferData2);
				//	if (FAILED(result))
				//	{
				//		ERROR_LOG("Context.Map() failed");
				//		return;
				//	}
				//	memcpy(bufferData2.pData, &myAnimationBufferData, sizeof(myAnimationBufferData));
				//	myContext.Unmap(myAnimationBuffer, 0);
				//}

				//// animation is on cbuffer slot 2
				//myContext.VSSetConstantBuffers(2, 1, &myAnimationBuffer);
			}

			// Map the bones to VS shader
			{
				D3D11_MAPPED_SUBRESOURCE bufferData2;

				ZeroMemory(&bufferData2, sizeof(bufferData2));
				result = myContext.Map(myAnimationBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferData2);
				ThrowIfFailed(result);
				memcpy(bufferData2.pData, &myAnimationBufferData, sizeof(myAnimationBufferData));
				myContext.Unmap(myAnimationBuffer, 0);
			}

			// animation is on cbuffer slot 2
			myContext.VSSetConstantBuffers(2, 1, &myAnimationBuffer);


			auto vertexBuffer = fbxResource->Get().GetVertexBuffer();
			auto indexBuffer = fbxResource->Get().GetIndexBuffer();
			auto stride = fbxResource->Get().GetStride();
			auto offset = fbxResource->Get().GetOffset();

			// Update object buffer
			{
				myObjectBufferData.myToWorld = modelInstance->GetTransformMatrix();
				myObjectBufferData.myHasBones = modelInstance->HasBones();

				bufferData = { };

				result = myContext.Map(myObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferData);
				ThrowIfFailed(result);

				memcpy(bufferData.pData, &myObjectBufferData, sizeof(myObjectBufferData));
				myContext.Unmap(myObjectBuffer, 0);
			}

			myContext.IASetPrimitiveTopology(fbxResource->Get().myPrimitiveTopology);

			myContext.IASetVertexBuffers(0, 1, &vertexBuffer, stride, offset);
			myContext.IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

			myContext.VSSetConstantBuffers(1, 1, &myObjectBuffer);

			myContext.GSSetConstantBuffers(2, 1, &myShadowCubeBuffer);

			myContext.GSSetShader(myGeometryShader->Get().GetGeometryShader(), nullptr, 0);

			myContext.PSSetShader(nullptr, nullptr, 0);

			const auto& meshDatas = fbxResource->Get().GetMeshDatas();

			// for (int meshIndex = 0; meshIndex < meshDatas.size(); ++meshIndex)
			{
				const int meshIndex = meshInstance->GetMeshIndex();
				const auto& meshData = meshDatas[meshIndex];

				//for (auto& pass : material.GetEffect()->GetPasses())
				EffectPass* pass = culledMeshInstance.myEffectPass;
				{
					if (!pass->GetVertexShader())
					{
						continue;
					}

					if (!pass->GetPixelShader())
					{
						continue;
					}

					if (!pass->GetVertexShader()->IsValid())
					{
						continue;
					}

					if (!pass->GetPixelShader()->IsValid())
					{
						continue;
					}

					myContext.IASetInputLayout(pass->GetVertexShader()->Get().GetInputLayout());
					myContext.VSSetShader(pass->GetVertexShader()->Get().GetVertexShader(), nullptr, 0);

					myContext.DrawIndexed(
						meshData.myIndicesCount,
						meshData.myStartIndexLocation,
						meshData.myBaseVertexLocation);

					stats.myDrawCalls++;
				}
			}
		}

		stats.myRenderTime = END_TIMER_GET_RESULT_MS(shadowRenderingTimer);

		myContext.GSSetShader(nullptr, nullptr, 0);

		return stats;
	}
#endif

	bool ShadowRenderer::CreateObjectBuffer()
	{
		return myObjectBuffer.InitDynamicWritable();
		/*
		D3D11_BUFFER_DESC bufDesc = {};
		{
			bufDesc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
			bufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			bufDesc.ByteWidth = sizeof(ObjectBuferData);
		}

		HRESULT result = myDevice.CreateBuffer(&bufDesc, nullptr, &myObjectBuffer);

		if (FAILED(result))
		{
			LOG_ERROR(LogType::Renderer) << "CreateBuffer failed to create frame cbuffer";
			return false;
		}

		return true;
		*/
	}

	bool ShadowRenderer::CreateAnimationBuffer()
	{
		return myAnimationBuffer.InitDynamicWritable();

		/*
		D3D11_BUFFER_DESC bufDesc = {};
		{
			bufDesc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
			bufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			bufDesc.ByteWidth = sizeof(AnimationBufferData);
		}

		HRESULT result = myDevice.CreateBuffer(&bufDesc, nullptr, &myAnimationBuffer);

		if (FAILED(result))
		{
			LOG_ERROR(LogType::Renderer) << "CreateBuffer failed to create animation cbuffer";
			return false;
		}

		return true;
		*/
	}

	bool ShadowRenderer::CreateShadowCubeBuffer()
	{
		D3D11_BUFFER_DESC bufDesc = { };
		{
			bufDesc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
			bufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			bufDesc.ByteWidth = sizeof(ShadowCubeBufferData);
		}

		HRESULT result = myDevice.CreateBuffer(&bufDesc, nullptr, &myShadowCubeBuffer);

		if (FAILED(result))
		{
			LOG_ERROR(LogType::Renderer) << "CreateBuffer failed to create shadow cube cbuffer";
			return false;
		}

		return true;
	}
}