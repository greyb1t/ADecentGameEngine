#include "pch.h"
#include "ForwardRenderer.h"
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
	ForwardRenderer::ForwardRenderer(Directx11Framework& aFramework, FrameBufferTempName& aFrameBuffer, ShaderRendererData& aSharedRendererData)
		: myContext(*aFramework.GetContext()),
		myDevice(*aFramework.GetDevice()),
		myFramework(aFramework),
		myFrameBuffer(aFrameBuffer),
		mySharedRendererData(aSharedRendererData)
	{
	}

	bool ForwardRenderer::Init()
	{
		D3D11_BUFFER_DESC bufDesc = {};
		{
			bufDesc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
			bufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		}

		if (!myObjectBuffer.InitDynamicWritable())
		{
			LOG_ERROR(LogType::Renderer) << "CreateBuffer failed to create object cbuffer";
			return false;
		}

		if (!myShadowDataBuffer.InitDynamicWritable())
		{
			LOG_ERROR(LogType::Renderer) << "CreateBuffer failed to create shadow data buffer.";
			return false;
		}

		/*
		bufDesc.ByteWidth = sizeof(ObjectBuferData);

		HRESULT result = myDevice.CreateBuffer(&bufDesc, nullptr, &myObjectBuffer);

		if (FAILED(result))
		{
			LOG_ERROR(LogType::Renderer) << "CreateBuffer failed to create object cbuffer";
			return false;
		}
		*/

		bufDesc.ByteWidth = sizeof(AnimationBufferData);

		HRESULT result = myDevice.CreateBuffer(&bufDesc, nullptr, &myAnimationBuffer);

		if (FAILED(result))
		{
			LOG_ERROR(LogType::Renderer) << "CreateBuffer failed to create shared cbuffer";
			return false;
		}

		return true;
	}

	ForwardRenderingStatistics ForwardRenderer::RenderModelInstances(
		EnvironmentLightComponent* aEnvironmentLight,
		RendererScene* aScene,
		std::vector<ModelInstance*> aModelInstances,
		std::vector<CulledMeshInstance>& aCulledMeshInstances,
		const Camera* aMainCamera)
	{
		ZoneNamedN(zone1, "ForwardRenderer::RenderModelInstances", true);

		ForwardRenderingStatistics stats;

		START_TIMER(ForwardRenderTimer); 

		myFrameBuffer.UpdateCamera(*aMainCamera);

		auto& bufferData = myShadowDataBuffer.Map(myContext);
		for (int cascadeIndex = 0; cascadeIndex < aEnvironmentLight->GetNumCascades(); ++cascadeIndex)
		{
			bufferData.myEnvLightViews[cascadeIndex] = Mat4f::GetFastInverse(aEnvironmentLight->GetCascadeView(cascadeIndex));
			bufferData.myEnvProjs[cascadeIndex] = aEnvironmentLight->GetCascadeProjection(cascadeIndex);
			aEnvironmentLight->GetCascadeTexture(cascadeIndex).SetAsResourceOnSlot(40 + cascadeIndex);
		}
		myShadowDataBuffer.Unmap(myContext);
		myShadowDataBuffer.BindPS(myContext, 3);

		HRESULT result = 0;

		if (auto cubemap1 = aEnvironmentLight->GetCubemap())
		{
			myContext.PSSetShaderResources(0, 1, cubemap1->Get().GetSRVPtr());
		}

		if (auto cubemap2 = aEnvironmentLight->GetCubemap2())
		{
			myContext.PSSetShaderResources(11, 1, cubemap2->Get().GetSRVPtr());
		}

		for (auto& culledMeshInstance : aCulledMeshInstances)
		{
			auto meshInstance = culledMeshInstance.myMeshInstance;

			// Cull the lights per mesh
			auto culledPointLights = aScene->CullPointLights(meshInstance->GetModelInstance());
			auto culledSpotLights = aScene->CullSpotLights(meshInstance->GetModelInstance());

			stats = stats + RenderMeshInstance(
				aEnvironmentLight,
				culledPointLights,
				culledSpotLights,
				aMainCamera,
				culledMeshInstance);
		}

		stats.myRenderTime = END_TIMER_GET_RESULT_MS(ForwardRenderTimer);

		return stats;
	}

	ForwardRenderingStatistics ForwardRenderer::RenderMeshInstance(
		EnvironmentLightComponent* aEnvironmentLight,
		const std::vector<PointLightComponent*> aPointLights,
		const std::vector<SpotLightComponent*> aSpotLights,
		const Camera* aCamera,
		CulledMeshInstance& aCulledMeshInstance)
	{
		ForwardRenderingStatistics stats;

		HRESULT result = 0;

		{
			auto meshInstance = aCulledMeshInstance.myMeshInstance;
			ModelInstance* modelInstance = meshInstance->GetModelInstance();
			const auto& fbxResource = modelInstance->GetModel();

			if (!fbxResource->IsValid())
			{
				return {};
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

			// model.GetModelData().myMaterial->BindBoundTextures(myContext);

			// TODO: We can use polymorphism better here by moving the code into a function
			// called PrepareForRendering() into ModelInstance, then we don't need the if check here
			myAnimationBufferData = {};

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
			auto& meshDatas = fbxResource->Get().GetMeshDatas();

			D3D11_MAPPED_SUBRESOURCE bufferData2;

			// reset the data
			// TODO: is there a reason it is a member variable instead of local variable?
			// myObjectBufferData = ObjectBuferData {};

			ObjectBuferData_ForwardRenderer myObjectBufferData;
			myObjectBufferData.myToWorld = modelInstance->GetTransformMatrix();
			myObjectBufferData.myHasBones = modelInstance->HasBones();

			myObjectBufferData.myAdditiveColor = meshInstance->GetMaterialInstance().GetFinalAdditiveColor();
			myObjectBufferData.myMainColor = meshInstance->GetMaterialInstance().GetMainColor();

			// const auto& [pointLightCount, pointLights] = aPointLights[modelInstanceIndex];

			// The point light data assumes that the model instances are in the same 
			// order as they've been added onto the its vector
			myObjectBufferData.myPointLightCount = aPointLights.size();
			assert(myObjectBufferData.myPointLightCount <= MaxPointLightCount);

			for (unsigned int pointLightIndex = 0; pointLightIndex < aPointLights.size(); ++pointLightIndex)
			{
				assert(pointLightIndex < MaxPointLightCount);

				PointLightComponent* pointLight = aPointLights[pointLightIndex];

				const auto pointLightPos = pointLight->GetGameObject()->GetTransform().GetPosition();

				myObjectBufferData.myPointLights[pointLightIndex].myPosition =
					C::Vector4f(
						pointLightPos.x,
						pointLightPos.y,
						pointLightPos.z,
						1.f);

				myObjectBufferData.myPointLights[pointLightIndex].myColorAndIntensity =
					C::Vector4f(
						pointLight->GetColor().x,
						pointLight->GetColor().y,
						pointLight->GetColor().z,
						pointLight->GetIntensity());

				myObjectBufferData.myPointLights[pointLightIndex].myRange = pointLight->GetRange();
			}



			// const auto& [spotLightCount, spotLights] = aSpotLights[modelInstanceIndex];

			// The spot light data assumes that the model instances are in the same 
			// order as they've been added onto the its vector
			myObjectBufferData.mySpotLightCount = aSpotLights.size();
			assert(myObjectBufferData.mySpotLightCount <= MaxSpotLightCount);

			for (unsigned int spotLightIndex = 0; spotLightIndex < aSpotLights.size(); ++spotLightIndex)
			{
				assert(spotLightIndex < MaxSpotLightCount);

				SpotLightComponent* spotLight = aSpotLights[spotLightIndex];

				const auto& spotLightPos = spotLight->GetGameObject()->GetTransform().GetPosition();

				myObjectBufferData.mySpotLights[spotLightIndex].myPosition =
					C::Vector4f(
						spotLightPos.x,
						spotLightPos.y,
						spotLightPos.z,
						1.f);

				const auto& dir = spotLight->GetGameObject()->GetTransform().GetMatrix().GetForward();
				myObjectBufferData.mySpotLights[spotLightIndex].myDirection = C::Vector4f(dir.x, dir.y, dir.z, 0.f);

				myObjectBufferData.mySpotLights[spotLightIndex].myColorAndIntensity =
					C::Vector4f(
						spotLight->GetColor().x,
						spotLight->GetColor().y,
						spotLight->GetColor().z,
						spotLight->GetIntensity());

				myObjectBufferData.mySpotLights[spotLightIndex].myRange = spotLight->GetRange();

				myObjectBufferData.mySpotLights[spotLightIndex].myInnerAngle = spotLight->GetInnerAngle();
				myObjectBufferData.mySpotLights[spotLightIndex].myOuterAngle = spotLight->GetOuterAngle();
			}

			auto& objectBufferData = myObjectBuffer.Map(myContext);
			objectBufferData = myObjectBufferData;
			myObjectBuffer.Unmap(myContext);

			// ZeroMemory(&bufferData2, sizeof(bufferData2));
			// result = myContext.Map(myObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferData2);
			// ThrowIfFailed(result);
			// memcpy(bufferData2.pData, &myObjectBufferData, sizeof(myObjectBufferData));
			// myContext.Unmap(myObjectBuffer, 0);

			assert(fbxResource->Get().myPrimitiveTopology != D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED);

			myContext.IASetPrimitiveTopology(fbxResource->Get().myPrimitiveTopology);
			myContext.IASetVertexBuffers(0, 1, &vertexBuffer, stride, offset);
			myContext.IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

			// myContext.VSSetConstantBuffers(1, 1, &myObjectBuffer);
			// // Start at slot 1 because environmentlight has slot 1
			// myContext.PSSetConstantBuffers(1, 1, &myObjectBuffer);

			myObjectBuffer.BindPS(myContext, 1);
			myObjectBuffer.BindVS(myContext, 1);

			// for (int meshIndex = 0; meshIndex < meshDatas.size(); ++meshIndex)
			{
				const int meshIndex = meshInstance->GetMeshIndex();
				const auto& meshData = meshDatas[meshIndex];

				auto material = meshInstance->GetMaterialInstance().GetMaterial();

				// 				if (!material->GetVertexShader())
				// 				{
				// 					continue;
				// 				}
				// 
				// 				if (!material->GetPixelShader())
				// 				{
				// 					continue;
				// 				}
				// 
				// 				if (!material->GetVertexShader()->IsValid())
				// 				{
				// 					continue;
				// 				}
				// 
				// 				if (!material->GetPixelShader()->IsValid())
				// 				{
				// 					continue;
				// 				}

				meshInstance->GetMaterialInstance().UpdateMaterialCustomValues();

				//for (auto& pass : material.GetEffect()->GetPasses())
				EffectPass* pass = aCulledMeshInstance.myEffectPass;

				pass->MapConstantBuffersDataToGPU(myContext);

				{
					if (!pass->GetVertexShader())
					{
						return {};
					}

					if (!pass->GetPixelShader())
					{
						return {};
					}

					if (!pass->GetVertexShader()->IsValid())
					{
						return {};
					}

					if (!pass->GetPixelShader()->IsValid())
					{
						return {};
					}

					//material->BindBoundTextures(&myContext);

					EffectPassState state = pass->BindToPipeline(&myContext,
						BindEffectFlags_EffectSpecificStuff |
						BindEffectFlags_PixelShader |
						BindEffectFlags_VertexShader);

					myContext.IASetInputLayout(pass->GetVertexShader()->Get().GetInputLayout());
					myContext.VSSetShader(pass->GetVertexShader()->Get().GetVertexShader(), nullptr, 0);
					myContext.PSSetShader(pass->GetPixelShader()->Get().GetPixelShader(), nullptr, 0);

					/*
					ID3D11ShaderResourceView* textures[] =
					{
						meshData.myTexture[0]->Get().GetSRV(),
						meshData.myTexture[1]->Get().GetSRV(),
						meshData.myTexture[2]->Get().GetSRV(),
					};
					myContext.PSSetShaderResources(1, static_cast<UINT>(meshData.myTexture.size()), &textures[0]);
					*/
					meshInstance->GetMaterialInstance().GetMaterial()->Bind(myContext);


					myContext.DrawIndexed(
						meshData.myIndicesCount,
						meshData.myStartIndexLocation,
						meshData.myBaseVertexLocation);

					state.ResetState(myContext);

					stats.myMeshDrawCalls++;
				}
			}
		}

		return stats;
	}

}