#include "pch.h"
#include "SelectionRenderer.h"

#include "../Model/ModelInstance.h"
#include "../Directx11Framework.h"
#include "../Camera/Camera.h"
#include "../ResourceManagement/Resources/VertexShaderResource.h"
#include "Engine/Renderer/Shaders/VertexShader.h"
#include "Engine/Renderer/Shaders/PixelShader.h"
#include "../ResourceManagement/Resources/PixelShaderResource.h"
#include "../ResourceManagement/Resources/TextureResource.h"
#include "../ResourceManagement/Resources/ModelResource.h"
#include "Engine/Renderer/Texture/Texture2D.h"
#include "Engine/Renderer/Model/Model.h"
#include "../WindowHandler.h"
#include "../SharedRendererData.h"
#include "../Material/MeshMaterial.h"
#include "Engine/Renderer/Scene/RendererScene.h"
#include "../FrameConstantBuffer.h"
#include "../Utils/DxUtils.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/PointLightComponent.h"
#include "Engine/GameObject/Components/SpotLightComponent.h"
#include "Engine/Renderer/Material/EffectPass.h"
#include "Engine/Renderer/Scene/CulledSceneData.h"
#include "Engine/GameObject/Components/EnvironmentLightComponent.h"

namespace Engine
{
	SelectionRenderer::SelectionRenderer(Directx11Framework& aFramework, FrameBufferTempName& aFrameBuffer, ShaderRendererData& aSharedRendererData):
		myDevice(*aFramework.GetDevice()),
		myContext(*aFramework.GetContext()),
		myFramework(aFramework),
		myFrameBuffer(aFrameBuffer),
		mySharedRendererData(aSharedRendererData)
	{}

	bool SelectionRenderer::Init()
	{
		D3D11_BUFFER_DESC bufDesc{};
		bufDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		bufDesc.ByteWidth = sizeof(ObjectBuferData);

		HRESULT result = myDevice.CreateBuffer(&bufDesc, nullptr, &myObjectBuffer);

		if (FAILED(result))
		{
			LOG_ERROR(LogType::Renderer) << "SelectionRenderer fail: CreateBuffer failed to create object cbuffer";
			return false;
		}

		bufDesc.ByteWidth = sizeof(AnimationBufferData);

		result = myDevice.CreateBuffer(&bufDesc, nullptr, &myAnimationBuffer);

		if (FAILED(result))
		{
			LOG_ERROR(LogType::Renderer) << "SelectionRenderer fail: CreateBuffer failed to create shared cbuffer";
			return false;
		}

		mySelectionShader = GResourceManager->CreateRef<PixelShaderResource>("Assets/Shaders/Engine/SelectionShader.ps");
		mySelectionShader->RequestLoading();

		bufDesc.ByteWidth = sizeof(SelectionBufferData);

		result = myDevice.CreateBuffer(&bufDesc, nullptr, &mySelectionBuffer);

		if (FAILED(result))
		{
			LOG_ERROR(LogType::Renderer) << "SelectionRenderer fail: CreateBuffer failed to create selection cbuffer";
			return false;
		}

		return true;
	}

	void SelectionRenderer::RenderModelInstances(std::vector<CulledMeshInstance>& aMeshInstances, const Camera* aMainCamera)
	{
		myFrameBuffer.UpdateCamera(*aMainCamera);

		for(auto& culledMeshInstance : aMeshInstances)
		{
			RenderMeshInstance(aMainCamera, culledMeshInstance);
		}
	}

	void SelectionRenderer::RenderMeshInstance(const Camera* aCamera, CulledMeshInstance& aCulledMeshInstance)
	{
		ForwardRenderingStatistics stats;

		myFrameBuffer.UpdateCamera(*aCamera);

		HRESULT result = 0;

		{
			auto meshInstance = aCulledMeshInstance.myMeshInstance;
			ModelInstance* modelInstance = meshInstance->GetModelInstance();
			const auto& fbxResource = modelInstance->GetModel();

			if (!fbxResource->IsValid())
			{
				return;
			}

			// TODO: We can use polymorphism better here by moving the code into a function
			// called PrepareForRendering() into ModelInstance, then we don't need the if check here
			
			// I Removed this as its unnecessary? And it takes a lot of performance
			//myAnimationBufferData = { };

			if (fbxResource->Get().IsSkinned())
			{
				const auto& finalTransforms = modelInstance->GetFinalAnimTransforms();

				memcpy(
					myAnimationBufferData.myBones,
					finalTransforms.data(),
					sizeof(Mat4f) * finalTransforms.size());
			}

			// Map the bones to VS shader
			{
				D3D11_MAPPED_SUBRESOURCE bufferData2{};

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
			auto& meshDatas = fbxResource->Get().GetMeshDatas();

			// reset the data
			// TODO: is there a reason it is a member variable instead of local variable?
			// myObjectBufferData = ObjectBuferData {};

			myObjectBufferData.myToWorld = modelInstance->GetTransformMatrix();
			myObjectBufferData.myHasBones = modelInstance->HasBones();

			myObjectBufferData.myPointLightCount = 0;
			myObjectBufferData.mySpotLightCount = 0;

			D3D11_MAPPED_SUBRESOURCE bufferData2{};
			ZeroMemory(&bufferData2, sizeof(bufferData2));
			result = myContext.Map(myObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferData2);
			ThrowIfFailed(result);
			memcpy(bufferData2.pData, &myObjectBufferData, sizeof(myObjectBufferData));
			myContext.Unmap(myObjectBuffer, 0);

			assert(fbxResource->Get().myPrimitiveTopology != D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED);

			myContext.IASetPrimitiveTopology(fbxResource->Get().myPrimitiveTopology);
			myContext.IASetVertexBuffers(0, 1, &vertexBuffer, stride, offset);
			myContext.IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

			myContext.VSSetConstantBuffers(1, 1, &myObjectBuffer);

			// Start at slot 1 because environmentlight has slot 1
			myContext.PSSetConstantBuffers(1, 1, &myObjectBuffer);

			{
				const int meshIndex = meshInstance->GetMeshIndex();
				const auto& meshData = meshDatas[meshIndex];

				auto material = meshInstance->GetMaterialInstance().GetMaterial();

				// meshInstance->GetMaterialInstance().UpdateMaterial();

				EffectPass* pass = aCulledMeshInstance.myEffectPass;
				{
					if (!pass->GetVertexShader())
					{
						return;
					}

					if (!pass->GetPixelShader())
					{
						return;
					}

					if (!pass->GetVertexShader()->IsValid())
					{
						return;
					}

					if (!pass->GetPixelShader()->IsValid())
					{
						return;
					}

					EffectPassState state = pass->BindToPipeline(&myContext,
						BindEffectFlags_EffectSpecificStuff |
						// BindEffectFlags_PixelShader |
						BindEffectFlags_VertexShader);

					myContext.IASetInputLayout(pass->GetVertexShader()->Get().GetInputLayout());
					myContext.VSSetShader(pass->GetVertexShader()->Get().GetVertexShader(), nullptr, 0);
					myContext.PSSetShader(mySelectionShader->Get().GetPixelShader(), nullptr, 0);

					D3D11_MAPPED_SUBRESOURCE subresource{};
					result = myContext.Map(mySelectionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);

					if(FAILED(result))
					{
						std::cout << "Selection buffer could not be mapped" << std::endl;
					}
					
					*reinterpret_cast<SelectionBufferData*>(subresource.pData) = {static_cast<int32_t>(aCulledMeshInstance.myUUID)};

					myContext.Unmap(mySelectionBuffer, 0);

					myContext.PSSetConstantBuffers(3, 1, &mySelectionBuffer);

					myContext.DrawIndexed(
						meshData.myIndicesCount,
						meshData.myStartIndexLocation,
						meshData.myBaseVertexLocation
					);

					state.ResetState(myContext);

					stats.myMeshDrawCalls++;
				}
			}
		}

		return;
	}
}