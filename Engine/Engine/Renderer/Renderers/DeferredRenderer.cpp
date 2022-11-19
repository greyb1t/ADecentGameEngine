#include "pch.h"
#include "DeferredRenderer.h"
#include "../Directx11Framework.h"
#include "../Model/ModelInstance.h"
#include "../ResourceManagement/Resources/VertexShaderResource.h"
#include "../ResourceManagement/Resources/TextureResource.h"
#include "../ResourceManagement/Resources/ModelResource.h"
#include "Engine/Renderer/Shaders/VertexShader.h"
#include "Engine/Renderer/Shaders/PixelShader.h"
#include "Engine/Renderer/Texture/Texture2D.h"
#include "Engine/ResourceManagement/Resources/PixelShaderResource.h"
#include "Engine/Renderer/Model/Model.h"
#include "../Material/MeshMaterial.h"
#include "../SharedRendererData.h"
#include "../ResourceManagement/ResourceManager.h"
#include "../Utils/DxUtils.h"
#include "../FrameConstantBuffer.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/MeshComponent.h"
#include "Engine/GameObject/Components/PointLightComponent.h"
#include "Engine/GameObject/Components/SpotLightComponent.h"
#include "Engine/Renderer/Material/EffectPass.h"
#include "Engine/Renderer/Model/MeshInstance.h"
#include "Engine/Renderer/Scene/CulledSceneData.h"
#include "Engine/GameObject/Components/DecalComponent.h"
#include "Engine/Renderer/Animation/AssimpUtils.h"
#include "Engine/Renderer/Material/DecalMaterial.h"
#include "Engine/GameObject/Components/EnvironmentLightComponent.h"
#include "Engine/Renderer/Shaders/ShaderConstants.h"
#include "Engine/Renderer/Sorting/MeshCommandIterator.h"
#include "Engine/Renderer/TracyProfilingGPU.h"
#include "Engine/Renderer/RenderManager.h"
#include "Engine/Renderer/Material/MaterialInstance.h"

namespace Engine
{
	DeferredRenderer::DeferredRenderer(
		Directx11Framework& aFramework,
		ResourceManager& aResourceManager,
		ResourceReferences& aResourceReferences,
		FrameBufferTempName& aFrameBuffer,
		VertexShaderRef aFullscreenVertexShader,
		ShaderRendererData& aSharedRendererData)
		: Renderer(*aFramework.GetContext())
		, myDevice(*aFramework.GetDevice()),
		myContext(*aFramework.GetContext()),
		myResourceManager(aResourceManager),
		myResourceReferences(aResourceReferences),
		myFrameBuffer(aFrameBuffer),
		mySharedRendererData(aSharedRendererData),
		myFullscreenShader(aFullscreenVertexShader)
	{
	}

	bool DeferredRenderer::Init()
	{
		D3D11_BUFFER_DESC bufDesc = {};
		{
			bufDesc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
			bufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		}

		bufDesc.ByteWidth = sizeof(EnvironmentLightBufferData);

		HRESULT result = myDevice.CreateBuffer(&bufDesc, nullptr, &myEnvironmentLightBuffer);

		if (FAILED(result))
		{
			LOG_ERROR(LogType::Renderer) << "CreateBuffer failed to create frame cbuffer";
			return false;
		}

		//bufDesc.ByteWidth = sizeof(ObjectBuferData_DeferredRenderer);

		if (!myObjectBuffer.InitDynamicWritable())
		{
			LOG_ERROR(LogType::Renderer) << "CreateBuffer failed to create object cbuffer";
			return false;
		}

		bufDesc.ByteWidth = sizeof(DecalObjectBufferData);

		result = myDevice.CreateBuffer(&bufDesc, nullptr, &myDecalObjectBuffer);

		if (FAILED(result))
		{
			LOG_ERROR(LogType::Renderer) << "CreateBuffer failed to create decal object cbuffer";
			return false;
		}

		bufDesc.ByteWidth = sizeof(PointLightBufferData);

		result = myDevice.CreateBuffer(&bufDesc, nullptr, &myPointLightBuffer);

		if (FAILED(result))
		{
			LOG_ERROR(LogType::Renderer) << "CreateBuffer failed to create frame cbuffer";
			return false;
		}

		if (!mySpotlightBuffer.InitDynamicWritable())
		{
			LOG_ERROR(LogType::Renderer) << "CreateBuffer failed to create frame cbuffer";
			return false;
		}

		//bufDesc.ByteWidth = sizeof(SpotLightBufferData);

		//result = myDevice.CreateBuffer(&bufDesc, nullptr, &mySpotLightBuffer);

		//if (FAILED(result))
		//{
		//	LOG_ERROR(LogType::Renderer) << "CreateBuffer failed to create frame cbuffer";
		//	return false;
		//}

		if (!myAnimationBuffer.InitDynamicWritable())
		{
			LOG_ERROR(LogType::Renderer) << "CreateBuffer failed to create animation cbuffer";
			return false;
		}

		CreateGBufferPixelShader();
		CreateEnvironmentLightPixelShader();
		CreatePointLightPixelShader();
		CreateSpotLightPixelShader();

		// for deferred decals drawing
		myCubeModel =
			myResourceManager.CreateRef<ModelResource>(
				"Assets\\Engine\\Meshes\\Cube1CM.model");
		myCubeModel->RequestLoading();

		CreateInstanceBuffer();

		return true;
	}

	DeferredRenderingStatistics DeferredRenderer::Render(
		EnvironmentLightComponent* aEnvironmentLight,
		const std::vector<PointLightComponent*> aPointLights,
		const std::vector<SpotLightComponent*> aSpotLights,
		const Camera* aCamera)
	{
		ZoneNamedN(zone1, "DeferredRenderer::Render", true);

		DeferredRenderingStatistics stats;

		myFrameBuffer.UpdateCamera(*aCamera);



		START_TIMER(DeferredRenderTimer);

		assert(aEnvironmentLight && "no environment light");

		// Running environment shader onto the texture for the environmentlight
		{
			GPU_ZONE_NAMED(envLightZone, "Environment Light");

			// Update the environment constant buffer
			{
				//myEnvironmentLightBufferData.myLightView = Mat4f::GetFastInverse(aEnvironmentLight->GetGameObject()->GetTransform().GetMatrix()); // the transform issue again
				//myEnvironmentLightBufferData.myLightProjection = aEnvironmentLight->GetCamera().GetProjectionMatrix();

				// Working cascades ------------
				for (int i = 0; i < aEnvironmentLight->GetNumCascades(); ++i)
				{
					myEnvironmentLightBufferData.myLightView[i] = Mat4f::GetFastInverse(aEnvironmentLight->GetCascadeView(i));
					myEnvironmentLightBufferData.myLightProjection[i] = aEnvironmentLight->GetCascadeProjection(i);

					//std::cout << "Cascade index: " << i << "-------------------" << std::endl;
					//std::cout << myEnvironmentLightBufferData.myLightView[i](1, 1) << " - " << myEnvironmentLightBufferData.myLightView[i](1, 2) << " - " << myEnvironmentLightBufferData.myLightView[i](1, 3) << " - " << myEnvironmentLightBufferData.myLightView[i](1, 4) << std::endl;
					//std::cout << myEnvironmentLightBufferData.myLightView[i](2, 1) << " - " << myEnvironmentLightBufferData.myLightView[i](2, 2) << " - " << myEnvironmentLightBufferData.myLightView[i](2, 3) << " - " << myEnvironmentLightBufferData.myLightView[i](2, 4) << std::endl;
					//std::cout << myEnvironmentLightBufferData.myLightView[i](3, 1) << " - " << myEnvironmentLightBufferData.myLightView[i](3, 2) << " - " << myEnvironmentLightBufferData.myLightView[i](3, 3) << " - " << myEnvironmentLightBufferData.myLightView[i](3, 4) << std::endl;
					//std::cout << myEnvironmentLightBufferData.myLightView[i](4, 1) << " - " << myEnvironmentLightBufferData.myLightView[i](4, 2) << " - " << myEnvironmentLightBufferData.myLightView[i](4, 3) << " - " << myEnvironmentLightBufferData.myLightView[i](4, 4) << std::endl;
					//std::cout << "View: " << std::endl;
					//std::cout << myEnvironmentLightBufferData.myLightProjection[i](1, 1) << " - " << myEnvironmentLightBufferData.myLightProjection[i](1, 2) << " - " << myEnvironmentLightBufferData.myLightProjection[i](1, 3) << " - " << myEnvironmentLightBufferData.myLightProjection[i](1, 4) << std::endl;
					//std::cout << myEnvironmentLightBufferData.myLightProjection[i](2, 1) << " - " << myEnvironmentLightBufferData.myLightProjection[i](2, 2) << " - " << myEnvironmentLightBufferData.myLightProjection[i](2, 3) << " - " << myEnvironmentLightBufferData.myLightProjection[i](2, 4) << std::endl;
					//std::cout << myEnvironmentLightBufferData.myLightProjection[i](3, 1) << " - " << myEnvironmentLightBufferData.myLightProjection[i](3, 2) << " - " << myEnvironmentLightBufferData.myLightProjection[i](3, 3) << " - " << myEnvironmentLightBufferData.myLightProjection[i](3, 4) << std::endl;
					//std::cout << myEnvironmentLightBufferData.myLightProjection[i](4, 1) << " - " << myEnvironmentLightBufferData.myLightProjection[i](4, 2) << " - " << myEnvironmentLightBufferData.myLightProjection[i](4, 3) << " - " << myEnvironmentLightBufferData.myLightView[i](4, 4) << std::endl;
				}

				// Since W might be intensity, dont use it in the normalization
				const auto invertedDirection = aEnvironmentLight->GetDirection() * -1.f;
				const auto direction = Vec3f(invertedDirection.x, invertedDirection.y, invertedDirection.z).GetNormalized();

				myEnvironmentLightBufferData.myToDirectionalLightDirection =
					C::Vector4f(direction.x, direction.y, direction.z,
						aEnvironmentLight->GetDirection().w);

				myEnvironmentLightBufferData.myDirectionalLightColorAndIntensity =
					C::Vector4f(
						aEnvironmentLight->GetColor().x,
						aEnvironmentLight->GetColor().y,
						aEnvironmentLight->GetColor().z,
						aEnvironmentLight->GetColorIntensity());

				D3D11_MAPPED_SUBRESOURCE bufferData;
				ZeroMemory(&bufferData, sizeof(bufferData));

				HRESULT result = myContext.Map(myEnvironmentLightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferData);
				ThrowIfFailed(result);

				memcpy(bufferData.pData, &myEnvironmentLightBufferData, sizeof(myEnvironmentLightBufferData));

				myContext.Unmap(myEnvironmentLightBuffer, 0);

				// myContext.VSSetConstantBuffers(1, 1, &myEnvironmentLightBuffer);
				myContext.PSSetConstantBuffers(1, 1, &myEnvironmentLightBuffer);
			}

			if (auto cubemap1 = aEnvironmentLight->GetCubemap())
			{
				myContext.PSSetShaderResources(0, 1, cubemap1->Get().GetSRVPtr());
			}

			if (auto cubemap2 = aEnvironmentLight->GetCubemap2())
			{
				myContext.PSSetShaderResources(11, 1, cubemap2->Get().GetSRVPtr());
			}

			// Binding cascade views.
			for (int i = 0; i < aEnvironmentLight->GetNumCascades(); ++i)
			{
				aEnvironmentLight->GetCascadeTexture(i).SetAsResourceOnSlot(9 + i);
			}
			//aEnvironmentLight->GetShadowMap().SetAsResourceOnSlot(9);

			myContext.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			myContext.IASetInputLayout(nullptr);
			myContext.IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
			myContext.IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);

			myContext.VSSetShader(myFullscreenShader->Get().GetVertexShader(), nullptr, 0);

			myContext.PSSetShader(myEnvironmentLightShader->Get().GetPixelShader(), nullptr, 0);
			myContext.Draw(3, 0);

			for (int i = 0; i < aEnvironmentLight->GetNumCascades(); ++i)
			{
				DxUtils::UnbindShaderResourceView(myContext, 9 + i);
			}

			stats.myEnvironmentLightDrawCalls++;
		}

		DxUtils::UnbindShaderResourceView(myContext, 9);

		//#if 0
		//		{
		//			GPU_ZONE_NAMED(pointLightZone, "Point Lights");
		//
		//			// point lights deferred
		//			for (auto& pointLight : aPointLights)
		//			{
		//				if (pointLight->IsCastingShadows())
		//				{
		//					for (int i = 0; i < 6; ++i)
		//					{
		//						myPointLightBufferData.myLightView[i] =
		//							Mat4f::GetFastInverse(pointLight->GetCameras()[i].GetTransform().ToMatrix());
		//					}
		//
		//					// all the cameras have the same projection matrix
		//					myPointLightBufferData.myLightProjection = pointLight->GetCameras()[0].GetProjectionMatrix();
		//					myPointLightBufferData.myIsCastingShadows = 1;
		//				}
		//				else
		//				{
		//					myPointLightBufferData.myIsCastingShadows = 0;
		//				}
		//
		//				const auto& pointLightPos = pointLight->GetGameObject()->GetTransform().GetPosition();
		//
		//				myPointLightBufferData.myPosition =
		//					C::Vector4f(
		//						pointLightPos.x,
		//						pointLightPos.y,
		//						pointLightPos.z,
		//						1.f);
		//
		//				myPointLightBufferData.myColorAndIntensity =
		//					C::Vector4f(
		//						pointLight->GetColor().x,
		//						pointLight->GetColor().y,
		//						pointLight->GetColor().z,
		//						pointLight->GetIntensity());
		//
		//				myPointLightBufferData.myRange = pointLight->GetRange();
		//
		//				D3D11_MAPPED_SUBRESOURCE bufferData;
		//				ZeroMemory(&bufferData, sizeof(bufferData));
		//
		//				HRESULT result = myContext.Map(myPointLightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferData);
		//				ThrowIfFailed(result);
		//
		//				memcpy(bufferData.pData, &myPointLightBufferData, sizeof(myPointLightBufferData));
		//
		//				myContext.Unmap(myPointLightBuffer, 0);
		//
		//				if (pointLight->IsCastingShadows())
		//				{
		//					// slot 10 is for the TextureCube in DeferredStruct.hlsli
		//					pointLight->GetShadowMap()->SetCubeAsResourceOnSlot(10);
		//				}
		//
		//				myContext.PSSetConstantBuffers(1, 1, &myPointLightBuffer);
		//				myContext.PSSetShader(myPointLightShader->Get().GetPixelShader(), nullptr, 0);
		//				myContext.Draw(3, 0);
		//
		//				if (pointLight->IsCastingShadows())
		//				{
		//					// slot 10 is for the TextureCube in DeferredStruct.hlsli
		//					DxUtils::UnbindShaderResourceView(myContext, 10);
		//				}
		//
		//				stats.myPointLightDrawCalls++;
		//			}
		//		}
		//#endif
		//
		//#if 0
		//		{
		//			GPU_ZONE_NAMED(spotLightZone, "Spot Lights");
		//
		//			// spot lights deferred
		//			for (auto& spotLight : aSpotLights)
		//			{
		//				if (spotLight->IsCastingShadows())
		//				{
		//					mySpotLightBufferData.myLightView = Mat4f::GetFastInverse(spotLight->GetGameObject()->GetTransform().GetMatrix());
		//					mySpotLightBufferData.myLightProjection = spotLight->GetCamera().GetProjectionMatrix();
		//					mySpotLightBufferData.myIsCastingShadows = 1;
		//				}
		//				else
		//				{
		//					mySpotLightBufferData.myIsCastingShadows = 0;
		//				}
		//
		//				const auto& spotLightPos = spotLight->GetGameObject()->GetTransform().GetPosition();
		//
		//				mySpotLightBufferData.myPosition =
		//					C::Vector4f(
		//						spotLightPos.x,
		//						spotLightPos.y,
		//						spotLightPos.z,
		//						1.f);
		//
		//				const auto& dir = spotLight->GetGameObject()->GetTransform().GetMatrix().GetForward();
		//				mySpotLightBufferData.myDirection = C::Vector4f(dir.x, dir.y, dir.z, 0.f);
		//
		//				mySpotLightBufferData.myColorAndIntensity =
		//					C::Vector4f(
		//						spotLight->GetColor().x,
		//						spotLight->GetColor().y,
		//						spotLight->GetColor().z,
		//						spotLight->GetIntensity());
		//
		//				mySpotLightBufferData.myRange = spotLight->GetRange();
		//
		//				mySpotLightBufferData.myInnerAngle = spotLight->GetInnerAngle();
		//				mySpotLightBufferData.myOuterAngle = spotLight->GetOuterAngle();
		//
		//				D3D11_MAPPED_SUBRESOURCE bufferData;
		//				ZeroMemory(&bufferData, sizeof(bufferData));
		//
		//				HRESULT result = myContext.Map(mySpotLightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferData);
		//				ThrowIfFailed(result);
		//
		//				memcpy(bufferData.pData, &mySpotLightBufferData, sizeof(mySpotLightBufferData));
		//
		//				myContext.Unmap(mySpotLightBuffer, 0);
		//
		//				if (spotLight->IsCastingShadows())
		//				{
		//					spotLight->GetShadowMap().SetAsResourceOnSlot(9);
		//				}
		//
		//				myContext.PSSetConstantBuffers(1, 1, &mySpotLightBuffer);
		//				myContext.PSSetShader(mySpotLightShader->Get().GetPixelShader(), nullptr, 0);
		//				myContext.Draw(3, 0);
		//
		//				if (spotLight->IsCastingShadows())
		//				{
		//					// slot 10 is for the TextureCube in DeferredStruct.hlsli
		//					DxUtils::UnbindShaderResourceView(myContext, 9);
		//				}
		//
		//				stats.mySpotLightDrawCalls++;
		//			}
		//		}
		//#endif

		stats.myRenderTime = END_TIMER_GET_RESULT_MS(DeferredRenderTimer);

		return stats;
	}

	DeferredRenderingStatistics DeferredRenderer::RenderPointLights(
		const std::vector<PointLightComponent*> aPointLights,
		const Camera& aCamera,
		RenderManager& aRenderManager)
	{
		DeferredRenderingStatistics stats;
		myFrameBuffer.UpdateCamera(aCamera);

		auto sphere = GResourceManager->CreateRef<ModelResource>("Assets\\Engine\\Meshes\\Sphere2cm.model");
		sphere->Load();
		assert(sphere->IsValid());

		{
			GPU_ZONE_NAMED(pointLightZone, "Point Lights");

			// point lights deferred
			for (auto& pointLight : aPointLights)
			{
				// clear stencil to 0
				myContext.ClearDepthStencilView(
					aRenderManager.GetReadOnlyDepthStencilView(),
					D3D11_CLEAR_STENCIL, 0.f, 0);

				if (pointLight->IsCastingShadows())
				{
					for (int i = 0; i < 6; ++i)
					{
						myPointLightBufferData.myLightView[i] =
							Mat4f::GetFastInverse(pointLight->GetCameras()[i].GetTransform().ToMatrix());
					}

					// all the cameras have the same projection matrix
					myPointLightBufferData.myLightProjection = pointLight->GetCameras()[0].GetProjectionMatrix();
					myPointLightBufferData.myIsCastingShadows = 1;
				}
				else
				{
					myPointLightBufferData.myIsCastingShadows = 0;
				}

				const auto& pointLightPos = pointLight->GetGameObject()->GetTransform().GetPosition();

				myPointLightBufferData.myPosition =
					C::Vector4f(
						pointLightPos.x,
						pointLightPos.y,
						pointLightPos.z,
						1.f);

				myPointLightBufferData.myColorAndIntensity =
					C::Vector4f(
						pointLight->GetColor().x,
						pointLight->GetColor().y,
						pointLight->GetColor().z,
						pointLight->GetIntensity());

				myPointLightBufferData.myRange = pointLight->GetRange();

				D3D11_MAPPED_SUBRESOURCE bufferData;
				ZeroMemory(&bufferData, sizeof(bufferData));

				HRESULT result = myContext.Map(myPointLightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferData);
				ThrowIfFailed(result);

				memcpy(bufferData.pData, &myPointLightBufferData, sizeof(myPointLightBufferData));

				myContext.Unmap(myPointLightBuffer, 0);

				if (pointLight->IsCastingShadows())
				{
					// slot 10 is for the TextureCube in DeferredStruct.hlsli
					pointLight->GetShadowMap()->SetCubeAsResourceOnSlot(10);
				}

				myContext.PSSetConstantBuffers(1, 1, &myPointLightBuffer);
				myContext.PSSetShader(myPointLightShader->Get().GetPixelShader(), nullptr, 0);

				{
					ObjectBuferData_DeferredRenderer& data = myObjectBuffer.Map(myContext);

					LiteTransform liteTransform;
					liteTransform.SetPositionLocal(pointLight->GetTransform().GetPosition());
					// rotation not needed? its a sphere lol
					liteTransform.SetRotationLocal(pointLight->GetTransform().GetRotation());

					// NOTE(filip): We scale up the sphere a little bit because it does not have
					// many vertices, so the biggest it is, the less "sphere-like" is becomes
					// therefore must scale it up slightly to avoid issues with light cutting
					// off due to not being a perfect square
					const float scaleMultiplier = 1.1f;
					liteTransform.SetScaleLocal(pointLight->GetRange() * scaleMultiplier);

					//data->myToWorld = pointLight->GetTransform().GetMatrix();
					data.myToWorld = liteTransform.GetMatrix();

					data.myAdditiveColor = {};
					data.myMainColor = Vec4f(1.f, 1.f, 1.f, 1.f);

					data.myHasBones = 0;

					// myContext.Unmap(myObjectBuffer, 0);
					myObjectBuffer.Unmap(myContext);

					// myContext.PSSetConstantBuffers(1, 1, &myObjectBuffer);
					// only VS should have object buffer, the PS has its own PointLightBuffer
					myContext.VSSetConstantBuffers(1, 1, myObjectBuffer.GetAddressOf());
				}

				auto vs =
					GResourceManager->CreateRef<VertexShaderResource>(
						"Assets/Shaders/Engine/VertexShaderLightVolume",
						DefaultVertex::ourInputElements,
						DefaultVertex::ourElementCount);
				vs->Load();
				assert(vs->IsValid());
				myContext.VSSetShader(vs->Get().GetVertexShader(), nullptr, 0);

				auto vertexBuffer = sphere->Get().GetVertexBuffer();
				auto indexBuffer = sphere->Get().GetIndexBuffer();
				auto stride = sphere->Get().GetStride();
				auto offset = sphere->Get().GetOffset();

				myContext.IASetVertexBuffers(0, 1, &vertexBuffer, stride, offset);

				myContext.IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

				myContext.IASetPrimitiveTopology(sphere->Get().myPrimitiveTopology);

				myContext.IASetInputLayout(
					vs->Get().GetInputLayout());

				// Reference:
				// https://kayru.org/articles/deferred-stencil/
				// Dead link, use web archive: 
				// https://web.archive.org/web/20210506193112/https://kayru.org/articles/deferred-stencil/

				// PASS 1
				// Render the sphere front faces with a stencil to create the
				// stencil mask for areas of the light that are not occluded by geometry
				// All pixels of the sphere that are occluded (fails depth test) get
				// stencil ref 1, meaning all with stencil ref 0, will possibly contribute
				// to lighting (if the do not fail the 2nd pass test)
				{
					// front faces
					aRenderManager.SetRasterizerState(RasterizerState_PointlightPass1);

					aRenderManager.SetBlendState(BlendState_Disable);

					// depth readonly
					myContext.OMSetDepthStencilState(
						aRenderManager.GetDepthStencilState(
							DepthStencilState_PointlightPass1), 1); // does this ref 1 matter?

					// aRenderManager.SetDepthStencilState();

					// no color write
					myContext.PSSetShader(nullptr, nullptr, 0);

					// assumes sphere has 1 mesh
					myContext.DrawIndexed(
						sphere->Get().myMeshDatas[0].myIndicesCount,
						sphere->Get().myMeshDatas[0].myStartIndexLocation,
						sphere->Get().myMeshDatas[0].myBaseVertexLocation);
				}

				// PASS 2
				// Render the sphere back faces
				// Every pixel that passes the Z (for back faces) and stencil test goes 
				// through the light pixel shader
				{
					aRenderManager.SetRasterizerState(RasterizerState_PointlightPass2);

					aRenderManager.SetBlendState(BlendState_Additive);

					// Compare against ref 0, if ref is 0, then the pixel passed the test
					// if ref is 1, then it did not pass bcuz it was occluded by something
					myContext.OMSetDepthStencilState(
						aRenderManager.GetDepthStencilState(
							DepthStencilState_PointlightPass2), 0);

					myContext.PSSetShader(myPointLightShader->Get().GetPixelShader(), nullptr, 0);

					myContext.DrawIndexed(
						sphere->Get().myMeshDatas[0].myIndicesCount,
						sphere->Get().myMeshDatas[0].myStartIndexLocation,
						sphere->Get().myMeshDatas[0].myBaseVertexLocation);
				}

				//myContext.Draw(3, 0);

				if (pointLight->IsCastingShadows())
				{
					// slot 10 is for the TextureCube in DeferredStruct.hlsli
					DxUtils::UnbindShaderResourceView(myContext, 10);
				}

				++stats.myPointLightDrawCalls;
			}
		}

		return stats;
	}

	DeferredRenderingStatistics DeferredRenderer::RenderSpotLights(
		const std::vector<SpotLightComponent*> aSpotLights,
		const Camera& aCamera,
		RenderManager& aRenderManager)
	{
		DeferredRenderingStatistics stats;

		myFrameBuffer.UpdateCamera(aCamera);

		auto cone = GResourceManager->CreateRef<ModelResource>("Assets\\Engine\\Meshes\\Cone1cm.model");
		cone->Load();
		assert(cone->IsValid());

		auto vs =
			GResourceManager->CreateRef<VertexShaderResource>(
				"Assets/Shaders/Engine/VertexShaderLightVolume",
				DefaultVertex::ourInputElements,
				DefaultVertex::ourElementCount);
		vs->Load();
		assert(vs->IsValid());

		// Reference:
		// https://kayru.org/articles/deferred-stencil/
		// Dead link, use web archive: 
		// https://web.archive.org/web/20210506193112/https://kayru.org/articles/deferred-stencil/

		GPU_ZONE_NAMED(spotLightZone, "Spot Lights");

		ResetInternalBindState();

		myContext.VSSetShader(vs->Get().GetVertexShader(), nullptr, 0);
		myContext.IASetInputLayout(
			vs->Get().GetInputLayout());

		BindModelMesh(cone->Get(), cone->GetID(), 0, BindModelFlags_None);

		// spot lights deferred
		for (auto& spotLight : aSpotLights)
		{
			// clear stencil to 0
			myContext.ClearDepthStencilView(
				aRenderManager.GetReadOnlyDepthStencilView(),
				D3D11_CLEAR_STENCIL, 0.f, 0);

			SpotLightBufferData mySpotLightBufferData;

			if (spotLight->IsCastingShadows())
			{
				mySpotLightBufferData.myLightView = Mat4f::GetFastInverse(spotLight->GetGameObject()->GetTransform().GetMatrix());
				mySpotLightBufferData.myLightProjection = spotLight->GetCamera().GetProjectionMatrix();
				mySpotLightBufferData.myIsCastingShadows = 1;
			}
			else
			{
				mySpotLightBufferData.myIsCastingShadows = 0;
			}

			const auto& spotLightPos = spotLight->GetGameObject()->GetTransform().GetPosition();

			mySpotLightBufferData.myPosition =
				C::Vector4f(
					spotLightPos.x,
					spotLightPos.y,
					spotLightPos.z,
					1.f);

			const auto& dir = spotLight->GetGameObject()->GetTransform().GetMatrix().GetForward();
			mySpotLightBufferData.myDirection = C::Vector4f(dir.x, dir.y, dir.z, 0.f);

			mySpotLightBufferData.myColorAndIntensity =
				C::Vector4f(
					spotLight->GetColor().x,
					spotLight->GetColor().y,
					spotLight->GetColor().z,
					spotLight->GetIntensity());

			mySpotLightBufferData.myRange = spotLight->GetRange();

			mySpotLightBufferData.myInnerAngle = spotLight->GetInnerAngle();
			mySpotLightBufferData.myOuterAngle = spotLight->GetOuterAngle();

			mySpotLightBufferData.myShadowMapResolution = spotLight->GetShadowMapResolution();

			SpotLightBufferData& spotData = mySpotlightBuffer.Map(myContext);
			spotData = mySpotLightBufferData;
			mySpotlightBuffer.Unmap(myContext);

			BindConstantBufferPS(
				ShaderConstants::SpotLightConstantBufferSlot,
				mySpotlightBuffer);

			if (spotLight->IsCastingShadows())
			{
				spotLight->GetShadowMap().SetAsResourceOnSlot(9);
			}

			{
				ObjectBuferData_DeferredRenderer objectBufferData;

				LiteTransform liteTransform;
				liteTransform.SetPositionLocal(spotLight->GetTransform().GetPosition());
				// rotation not needed? its a sphere lol

				// NOTE(filip): the cone model is has the Z axis opposite, therefore
				// must manually rotate it, ask Alex to change the cone +Z
				Quatf t;
				t.InitWithAxisAndRotation(Vec3f(0.f, 1.f, 0.f), Math::PI);
				liteTransform.SetRotationLocal(spotLight->GetTransform().GetRotation() * t);

				// NOTE(filip): We scale up the sphere a little bit because it does not have
				// many vertices, so the biggest it is, the less "sphere-like" is becomes
				// therefore must scale it up slightly to avoid issues with light cutting
				// off due to not being a perfect square
				const float scaleMultiplier = 1.1f;

				float cosOuterAngle = cos(spotLight->GetOuterAngle());
				float cosInnerAngle = cos(spotLight->GetInnerAngle());

				float maxAngle = std::max(spotLight->GetInnerAngle(), spotLight->GetOuterAngle());

				// NOTE(filip): this is an approximation, not 100% correct
				// but good enough
				maxAngle *= Math::PI;
				maxAngle = maxAngle * maxAngle;

				// X and Y (the base part of the cone 
				// must be bigger when the inner and outer angle changes)
				Vec3f scaleMultiplierVec3 = Vec3f(
					spotLight->GetRange() * maxAngle,
					spotLight->GetRange() * maxAngle,
					spotLight->GetRange());
				liteTransform.SetScaleLocal(scaleMultiplierVec3 * scaleMultiplier);

				//data->myToWorld = pointLight->GetTransform().GetMatrix();
				objectBufferData.myToWorld = liteTransform.GetMatrix();
				objectBufferData.myAdditiveColor = {};
				objectBufferData.myMainColor = Vec4f(1.f, 1.f, 1.f, 1.f);
				objectBufferData.myHasBones = 0;

				ObjectBuferData_DeferredRenderer& data = myObjectBuffer.Map(myContext);
				data = objectBufferData;
				myObjectBuffer.Unmap(myContext);

				BindConstantBufferVS(
					ShaderConstants::ObjectConstantBufferSlot,
					myObjectBuffer);
			}

			// PASS 1
			// Render the sphere front faces with a stencil to create the
			// stencil mask for areas of the light that are not occluded by geometry
			// All pixels of the sphere that are occluded (fails depth test) get
			// stencil ref 1, meaning all with stencil ref 0, will possibly contribute
			// to lighting (if the do not fail the 2nd pass test)
			{
				// front faces
				aRenderManager.SetRasterizerState(RasterizerState_PointlightPass1);

				aRenderManager.SetBlendState(BlendState_Disable);

				// depth readonly
				myContext.OMSetDepthStencilState(
					aRenderManager.GetDepthStencilState(
						DepthStencilState_PointlightPass1), 1); // does this ref 1 matter?

				// no color write
				myContext.PSSetShader(nullptr, nullptr, 0);

				// assumes sphere has 1 mesh
				myContext.DrawIndexed(
					cone->Get().myMeshDatas[0].myIndicesCount,
					cone->Get().myMeshDatas[0].myStartIndexLocation,
					cone->Get().myMeshDatas[0].myBaseVertexLocation);
			}

			// PASS 2
			// Render the sphere back faces
			// Every pixel that passes the Z (for back faces) and stencil test goes 
			// through the light pixel shader
			{
				aRenderManager.SetRasterizerState(RasterizerState_PointlightPass2);

				aRenderManager.SetBlendState(BlendState_Additive);

				// Compare against ref 0, if ref is 0, then the pixel passed the test
				// if ref is 1, then it did not pass bcuz it was occluded by something
				myContext.OMSetDepthStencilState(
					aRenderManager.GetDepthStencilState(
						DepthStencilState_PointlightPass2), 0);

				myContext.PSSetShader(mySpotLightShader->Get().GetPixelShader(), nullptr, 0);

				myContext.DrawIndexed(
					cone->Get().myMeshDatas[0].myIndicesCount,
					cone->Get().myMeshDatas[0].myStartIndexLocation,
					cone->Get().myMeshDatas[0].myBaseVertexLocation);
			}

			if (spotLight->IsCastingShadows())
			{
				DxUtils::UnbindShaderResourceView(myContext, 9);
			}

			++stats.mySpotLightDrawCalls;
		}

		return stats;
	}

	const ModelRef& DeferredRenderer::GetDecalCubeModel()
	{
		return myCubeModel;
	}

	bool DeferredRenderer::CreateGBufferPixelShader()
	{
		myGBufferPixelShader =
			myResourceManager.CreateRef<PixelShaderResource>(
				"Assets/Shaders/Engine/Deferred/GBuffer");
		myGBufferPixelShader->RequestLoading();

		return true;
	}

	bool DeferredRenderer::CreateEnvironmentLightPixelShader()
	{
		myEnvironmentLightShader =
			myResourceManager.CreateRef<PixelShaderResource>(
				"Assets/Shaders/Engine/Deferred/EnvironmentLightShader");
		myEnvironmentLightShader->RequestLoading();

		return true;
	}

	bool DeferredRenderer::CreatePointLightPixelShader()
	{
		myPointLightShader = myResourceReferences.AddPixelShader("Assets/Shaders/Engine/Deferred/PointLightShader");

		myPointLightShader->RequestLoading();

		return true;
	}

	bool DeferredRenderer::CreateSpotLightPixelShader()
	{
		mySpotLightShader =
			myResourceManager.CreateRef<PixelShaderResource>(
				"Assets/Shaders/Engine/Deferred/SpotLightShader");
		mySpotLightShader->RequestLoading();

		return true;
	}

	DeferredGBufferStats DeferredRenderer::GenerateGBuffer(
		const Camera* aCamera,
		const std::vector<CulledMeshInstance>& aCulledMeshInstances,
		const std::vector<CulledMeshComponent>& aMeshComponents,
		const RenderCommandList& aCommandList)
	{
		ZoneScopedN("DeferredRenderer::GenerateGBuffer");

		DeferredGBufferStats stats;

		START_TIMER(GBufferGenerateTimer);

		myFrameBuffer.UpdateCamera(*aCamera);

		HRESULT result = 0;

		ResetInternalBindState();

#if 1
		const size_t cmdListSize = aCommandList.myMeshCommands.size();

		for (size_t i = 0; i < cmdListSize;)
		{
			const auto& command = aCommandList.myMeshCommands[i];
			const auto& meshData = command.myMeshData;

			// TODO: find the material textures here
			BindMaterial(command.mySortKey.myMaterialID, *meshData.myMaterial);

			// Must update the custom values before binding the effect pass because
			// internally it maps the values
			//meshData.myMaterialInstance->UpdateMaterial();
			//meshData.myPass->MapConstantBuffersDataToGPU(myContext);

			BindEffectPass(*meshData.myPass,
				BindEffectFlags_VertexShader |
				BindEffectFlags_PixelShader |
				BindEffectFlags_EffectSpecificStuff);

			// UpdateMaterialInstanceValues();

			if (meshData.myPass->IsInstanced())
			{
				assert(false && "unimplemented");
				//BindModelMesh(
				//	*meshData.myModel,
				//	command.mySortKey.myModelID,
				//	meshData.myMeshIndex);

				//// BeginMeshInstanced()
				//D3D11_MAPPED_SUBRESOURCE subRes = {};

				//result = myContext.Map(
				//	myInstanceBuffer,
				//	0,
				//	D3D11_MAP_WRITE_DISCARD,
				//	0,
				//	&subRes);

				//ThrowIfFailed(result);

				//auto data = reinterpret_cast<InstanceBufferData*>(subRes.pData);

				//// First mesh object data
				//data[0].myToWorld = meshData.myTransform;

				//// Start at one because we initially add 1 manually before the loop
				//int instanceCount = 1;

				//const int startIterationIndex = i;
				//const int maxIterationCount = cmdListSize - startIterationIndex;

				//// If we come here
				//// Start a for loop from the current command and forward
				//// and build an instance buffer until pass, model or mesh changed
				//for (int j = 1; j < maxIterationCount; ++j)
				//{
				//	const auto& innerCommand = aCommandList.myMeshCommands[startIterationIndex + j];
				//	const auto& innerMeshData = innerCommand.myMeshData;

				//	if (!IsModelBound(
				//		innerCommand.mySortKey.myModelID,
				//		innerMeshData.myMeshIndex))
				//	{
				//		break;
				//	}

				//	if (!IsBound(*innerMeshData.myPass))
				//	{
				//		break;
				//	}

				//	data[j].myToWorld = innerMeshData.myTransform;

				//	++instanceCount;
				//}

				//i += instanceCount;

				//myContext.Unmap(myInstanceBuffer, 0);
				//// EndMeshInstanced()

				//// myGeometryBuffer.AddInstance();

				//// BindModelMeshInstanced(meshData.myModel, meshData.myMeshIndex);

				//myContext.PSSetShaderResources(6, 1, &myInstanceBufferSRV);
				//myContext.VSSetShaderResources(6, 1, &myInstanceBufferSRV);

				//const auto& meshDatasss = meshData.myModel->myMeshDatas;
				//const auto& modelMesh = meshDatasss[meshData.myMeshIndex];

				//myContext.DrawIndexedInstanced(
				//	modelMesh.myIndicesCount,
				//	instanceCount,
				//	modelMesh.myStartIndexLocation,
				//	modelMesh.myBaseVertexLocation,
				//	0);

				//stats.myMeshDrawCalls++;
			}
			else
			{
				BindModelMesh(
					*meshData.myModel,
					command.mySortKey.myModelID,
					meshData.myMeshIndex,
					BindModelFlags_Textures);

				ObjectBuferData_DeferredRenderer& data = myObjectBuffer.Map(myContext);

				data.myToWorld = meshData.myTransform;
				data.myAdditiveColor = meshData.myMaterialInstance->GetFinalAdditiveColor();
				data.myMainColor = meshData.myMaterialInstance->GetMainColor();
				data.myHasBones = meshData.myHasSkeleton ? 1 : 0;

				myObjectBuffer.Unmap(myContext);

				BindConstantBufferVS(
					ShaderConstants::ObjectConstantBufferSlot,
					myObjectBuffer);

				BindConstantBufferPS(
					ShaderConstants::ObjectConstantBufferSlot,
					myObjectBuffer);

				if (meshData.myHasSkeleton)
				{
					AnimationBufferData& animData = myAnimationBuffer.Map(myContext);
					//animData = *meshData.myAnimTransforms;
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

				// CUSTOM VALUES
				//meshData.myMaterialInstance->UpdateMaterial();

				const bool needsCustomValuesUpdate = meshData.myMaterialInstance->AreValuesModifiedFromDefault();

				if (needsCustomValuesUpdate)
				{
					meshData.myMaterialInstance->UpdateMaterialCustomValues();
					meshData.myPass->MapConstantBuffersDataToGPU(myContext);
				}

				myContext.DrawIndexed(
					modelMesh.myIndicesCount,
					modelMesh.myStartIndexLocation,
					modelMesh.myBaseVertexLocation);

				// Outline does not work properly because the effectpasses order are not executred in the correct order?

				// NOTE(filip): sadly we have to reset to default values and map to
				// GPU again to keep it clean and understandable
				if (needsCustomValuesUpdate)
				{
					meshData.myMaterialInstance->ResetCustomValuesToDefault();
					meshData.myPass->MapConstantBuffersDataToGPU(myContext);
				}

				stats.myMeshDrawCalls++;

				++i;
			}
		}
#endif

		//#if 0
		//		for (int meshInstanceIndex = 0; meshInstanceIndex < aCulledMeshInstances.size(); ++meshInstanceIndex)
		//		{
		//			const auto& culledMeshInstance = aCulledMeshInstances[meshInstanceIndex];
		//			const auto& meshInstance = culledMeshInstance.myMeshInstance;
		//
		//			ModelInstance* modelInstance = meshInstance->GetModelInstance();
		//			const auto& fbxResource = modelInstance->GetModel();
		//
		//			if (!fbxResource->IsValid())
		//			{
		//				continue;
		//			}
		//
		//			auto vertexBuffer = fbxResource->Get().GetVertexBuffer();
		//			auto indexBuffer = fbxResource->Get().GetIndexBuffer();
		//			auto stride = fbxResource->Get().GetStride();
		//			auto offset = fbxResource->Get().GetOffset();
		//
		//			// TODO: We can use polymorphism better here by moving the code into a function
		//			// called PrepareForRendering() into ModelInstance, then we don't need the if check here
		//			// myAnimationBufferData = {};
		//
		//			if (fbxResource->Get().IsSkinned())
		//			{
		//				const auto& finalTransforms = modelInstance->GetFinalAnimTransforms();
		//
		//				AnimationBufferData& animData = myAnimationBuffer.Map(myContext);
		//
		//				memcpy(
		//					animData.myBones,
		//					finalTransforms.data(),
		//					sizeof(Mat4f) * finalTransforms.size());
		//
		//				myAnimationBuffer.Unmap(myContext);
		//
		//				// myAnimationBufferData.OB_HasBones = modelInstance->HasBones();
		//			}
		//
		//			// Map the bones to VS shader
		//			// NOTE: I have to do this everytime even if not skinned because
		//			// it needs to reset the OB_HasBones to 0
		//			// TODO: Find a better solution to this
		//			/*
		//			{
		//				D3D11_MAPPED_SUBRESOURCE bufferData2;
		//
		//				ZeroMemory(&bufferData2, sizeof(bufferData2));
		//				result = myContext.Map(myAnimationBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferData2);
		//				ThrowIfFailed(result);
		//				memcpy(bufferData2.pData, &myAnimationBufferData, sizeof(myAnimationBufferData));
		//				myContext.Unmap(myAnimationBuffer, 0);
		//			}
		//			*/
		//
		//			// animation is on cbuffer slot 2
		//			myContext.VSSetConstantBuffers(2, 1, myAnimationBuffer.GetAddressOf());
		//
		//			D3D11_MAPPED_SUBRESOURCE bufferData2;
		//
		//			myObjectBufferData.myToWorld = modelInstance->GetTransformMatrix();
		//			myObjectBufferData.myHasBones = modelInstance->HasBones();
		//
		//			ObjectBuferData& data = myObjectBuffer.Map(myContext);
		//			data = myObjectBufferData;
		//
		//			myObjectBuffer.Unmap(myContext);
		//
		//			assert(fbxResource->Get().myPrimitiveTopology != D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED);
		//
		//			myContext.IASetPrimitiveTopology(fbxResource->Get().myPrimitiveTopology);
		//			//myContext.IASetVertexBuffers(0, 1, &modelData.myVertexBuffer, &modelData.myStride, &modelData.myOffset);
		//			myContext.IASetVertexBuffers(0, 1, &vertexBuffer, stride, offset);
		//			//myContext.IASetIndexBuffer(modelData.myIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		//			myContext.IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
		//
		//			myContext.VSSetConstantBuffers(1, 1, myObjectBuffer.GetAddressOf());
		//
		//
		//
		//
		//
		//			// Start at slot 1 because environmentlight has slot 1
		//			myContext.PSSetConstantBuffers(1, 1, myObjectBuffer.GetAddressOf());
		//
		//			//myContext.PSSetShader(myGBufferPixelShader->Get().GetPixelShader(), nullptr, 0);
		//
		//			//for (const auto& meshData : modelData.myMeshDatas)
		//			const auto& meshDatas = fbxResource->Get().GetMeshDatas();
		//
		//			// for (int meshIndex = 0; meshIndex < meshDatas.size(); ++meshIndex)
		//			{
		//				const int meshIndex = meshInstance->GetMeshIndex();
		//				const auto& meshData = meshDatas[meshIndex];
		//
		//				//if (!material->GetVertexShader())
		//				//{
		//				//	continue;
		//				//}
		//
		//				//if (!material->GetPixelShader())
		//				//{
		//				//	continue;
		//				//}
		//
		//				//if (!material->GetVertexShader()->IsValid())
		//				//{
		//				//	continue;
		//				//}
		//
		//				//if (!material->GetPixelShader()->IsValid())
		//				//{
		//				//	continue;
		//				//}
		//
		//				meshInstance->GetMaterialInstance().UpdateMaterial();
		//
		//				//for (auto& pass : material.GetEffect()->GetPasses())
		//				EffectPass* pass = culledMeshInstance.myEffectPass;
		//				{
		//					if (!pass->GetVertexShader())
		//					{
		//						continue;
		//					}
		//
		//					if (!pass->GetPixelShader())
		//					{
		//						continue;
		//					}
		//
		//					if (!pass->GetVertexShader()->IsValid())
		//					{
		//						continue;
		//					}
		//
		//					if (!pass->GetPixelShader()->IsValid())
		//					{
		//						continue;
		//					}
		//
		//					EffectPassState state = pass->BindToPipeline(&myContext);
		//					//material->BindBoundTextures(&myContext);
		//
		//					myContext.IASetInputLayout(pass->GetVertexShader()->Get().GetInputLayout());
		//					myContext.VSSetShader(pass->GetVertexShader()->Get().GetVertexShader(), nullptr, 0);
		//					// TODO: Consider making an TextureArray type that can convert to this array internally
		//					ID3D11ShaderResourceView* textures[] =
		//					{
		//						meshData.myTexture[0]->Get().GetSRV(),
		//						meshData.myTexture[1]->Get().GetSRV(),
		//						meshData.myTexture[2]->Get().GetSRV(),
		//					};
		//					myContext.PSSetShaderResources(1, static_cast<UINT>(meshData.myTexture.size()), &textures[0]);
		//					myContext.PSSetShader(pass->GetPixelShader()->Get().GetPixelShader(), nullptr, 0);
		//
		//
		//
		//
		//					myContext.DrawIndexed(
		//						meshData.myIndicesCount,
		//						meshData.myStartIndexLocation,
		//						meshData.myBaseVertexLocation);
		//
		//					state.ResetState(myContext);
		//
		//					stats.myMeshDrawCalls++;
		//				}
		//			}
		//		}
		//#endif
		//
		//#if 0
		//		for (int meshInstanceIndex = 0; meshInstanceIndex < aMeshComponents.size(); ++meshInstanceIndex)
		//		{
		//			const auto& culledMeshComponent = aMeshComponents[meshInstanceIndex];
		//			const auto& meshComponent = culledMeshComponent.myMeshComponent;
		//
		//			const auto& fbxResource = meshComponent->GetModel();
		//
		//			if (!fbxResource->IsValid())
		//			{
		//				continue;
		//			}
		//
		//			auto vertexBuffer = fbxResource->Get().GetVertexBuffer();
		//			auto indexBuffer = fbxResource->Get().GetIndexBuffer();
		//			auto stride = fbxResource->Get().GetStride();
		//			auto offset = fbxResource->Get().GetOffset();
		//
		//			// TODO: We can use polymorphism better here by moving the code into a function
		//			// called PrepareForRendering() into ModelInstance, then we don't need the if check here
		//			// myAnimationBufferData = {};
		//
		//			if (fbxResource->Get().IsSkinned())
		//			{
		//				// TODO: Add the final anim transforms into AnimatorComponent
		//				// and get it them here
		//				// if (meshComponent->GetGameObject()->GetComponent<>())
		//				// {
		//				// 	
		//				// }
		//				// const auto& finalTransforms = modelInstance->GetFinalAnimTransforms();
		//				// 
		//				// memcpy(
		//				// 	myAnimationBufferData.myBones,
		//				// 	finalTransforms.data(),
		//				// 	sizeof(Mat4f) * finalTransforms.size());
		//				// 
		//				// myAnimationBufferData.OB_HasBones = modelInstance->HasBones();
		//			}
		//
		//			// Map the bones to VS shader
		//			// NOTE: I have to do this everytime even if not skinned because
		//			// it needs to reset the OB_HasBones to 0
		//			// TODO: Find a better solution to this
		//			/*
		//			{
		//				D3D11_MAPPED_SUBRESOURCE bufferData2;
		//
		//				ZeroMemory(&bufferData2, sizeof(bufferData2));
		//				result = myContext.Map(myAnimationBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferData2);
		//				ThrowIfFailed(result);
		//				memcpy(bufferData2.pData, &myAnimationBufferData, sizeof(myAnimationBufferData));
		//				myContext.Unmap(myAnimationBuffer, 0);
		//			}
		//
		//			// animation is on cbuffer slot 2
		//			myContext.VSSetConstantBuffers(2, 1, &myAnimationBuffer);
		//			*/
		//
		//			// D3D11_MAPPED_SUBRESOURCE bufferData2;
		//
		//			myObjectBufferData.myToWorld = meshComponent->GetGameObject()->GetTransform().GetMatrix();
		//
		//			// current do not support bones for single meshes
		//			myObjectBufferData.myHasBones = 0;
		//
		//			ObjectBuferData& data = myObjectBuffer.Map(myContext);
		//			data = myObjectBufferData;
		//			myObjectBuffer.Unmap(myContext);
		//
		//			assert(fbxResource->Get().myPrimitiveTopology != D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED);
		//
		//			myContext.IASetPrimitiveTopology(fbxResource->Get().myPrimitiveTopology);
		//			//myContext.IASetVertexBuffers(0, 1, &modelData.myVertexBuffer, &modelData.myStride, &modelData.myOffset);
		//			myContext.IASetVertexBuffers(0, 1, &vertexBuffer, stride, offset);
		//			//myContext.IASetIndexBuffer(modelData.myIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		//			myContext.IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
		//
		//			myContext.VSSetConstantBuffers(1, 1, myObjectBuffer.GetAddressOf());
		//
		//
		//
		//
		//
		//			// Start at slot 1 because environmentlight has slot 1
		//			myContext.PSSetConstantBuffers(1, 1, myObjectBuffer.GetAddressOf());
		//
		//			//myContext.PSSetShader(myGBufferPixelShader->Get().GetPixelShader(), nullptr, 0);
		//
		//			//for (const auto& meshData : modelData.myMeshDatas)
		//			const auto& meshDatas = fbxResource->Get().GetMeshDatas();
		//
		//			// for (int meshIndex = 0; meshIndex < meshDatas.size(); ++meshIndex)
		//			{
		//				const int meshIndex = meshComponent->GetMeshIndex();
		//				const auto& meshData = meshDatas[meshIndex];
		//
		//				//if (!material->GetVertexShader())
		//				//{
		//				//	continue;
		//				//}
		//
		//				//if (!material->GetPixelShader())
		//				//{
		//				//	continue;
		//				//}
		//
		//				//if (!material->GetVertexShader()->IsValid())
		//				//{
		//				//	continue;
		//				//}
		//
		//				//if (!material->GetPixelShader()->IsValid())
		//				//{
		//				//	continue;
		//				//}
		//
		//				meshComponent->GetMaterialInstance().UpdateMaterial();
		//
		//				//for (auto& pass : material.GetEffect()->GetPasses())
		//				EffectPass* pass = culledMeshComponent.myEffectPass;
		//				{
		//					if (!pass->GetVertexShader())
		//					{
		//						continue;
		//					}
		//
		//					if (!pass->GetPixelShader())
		//					{
		//						continue;
		//					}
		//
		//					if (!pass->GetVertexShader()->IsValid())
		//					{
		//						continue;
		//					}
		//
		//					if (!pass->GetPixelShader()->IsValid())
		//					{
		//						continue;
		//					}
		//
		//					EffectPassState state = pass->BindToPipeline(&myContext);
		//					//material->BindBoundTextures(&myContext);
		//
		//					myContext.IASetInputLayout(pass->GetVertexShader()->Get().GetInputLayout());
		//					myContext.VSSetShader(pass->GetVertexShader()->Get().GetVertexShader(), nullptr, 0);
		//					// TODO: Consider making an TextureArray type that can convert to this array internally
		//					ID3D11ShaderResourceView* textures[] =
		//					{
		//						meshData.myTexture[0]->Get().GetSRV(),
		//						meshData.myTexture[1]->Get().GetSRV(),
		//						meshData.myTexture[2]->Get().GetSRV(),
		//					};
		//					myContext.PSSetShaderResources(1, static_cast<UINT>(meshData.myTexture.size()), &textures[0]);
		//					myContext.PSSetShader(pass->GetPixelShader()->Get().GetPixelShader(), nullptr, 0);
		//
		//					myContext.DrawIndexed(
		//						meshData.myIndicesCount,
		//						meshData.myStartIndexLocation,
		//						meshData.myBaseVertexLocation);
		//
		//					state.ResetState(myContext);
		//
		//					stats.myMeshDrawCalls++;
		//				}
		//			}
		//		}
		//#endif

		stats.myRenderTime = END_TIMER_GET_RESULT_MS(GBufferGenerateTimer);

		return stats;
	}

	DeferredGBufferStats DeferredRenderer::RenderDecals(const Camera* aCamera, const std::vector<CulledDecalComponent>& aDecalComponents)
	{
		ZoneNamedN(zone1, "DeferredRenderer::RenderDecals", true);

		DeferredGBufferStats stats;

		START_TIMER(GBufferGenerateTimer);

		myFrameBuffer.UpdateCamera(*aCamera);

		HRESULT result = 0;

		for (int decalIndex = 0; decalIndex < aDecalComponents.size(); ++decalIndex)
		{
			const auto& culledDecalInstance = aDecalComponents[decalIndex];

			if (culledDecalInstance.myDecalComponent->GetMaterialInstance().GetMaterial()->GetType() != MaterialType::Decal)
			{
				LOG_WARNING(LogType::Renderer) << "A decal has a material that is not of type decal: " << culledDecalInstance.myDecalComponent->GetGameObject()->GetName();
				continue;
			}

			const auto& decalComponent = culledDecalInstance.myDecalComponent;

			const auto& fbxResource = myCubeModel;

			if (!fbxResource->IsValid())
			{
				continue;
			}

			// TODO: Consider making a Material.Bind()
			// instead of getting the stuff and binding manually
			auto material = decalComponent->GetMaterialInstance().GetMaterial();
			assert(material &&
				material->GetType() == MaterialType::Decal &&
				dynamic_cast<DecalMaterial*>(material) != nullptr);
			auto decalMat = static_cast<DecalMaterial*>(material);

			auto vertexBuffer = fbxResource->Get().GetVertexBuffer();
			auto indexBuffer = fbxResource->Get().GetIndexBuffer();
			auto stride = fbxResource->Get().GetStride();
			auto offset = fbxResource->Get().GetOffset();

			// TODO: We can use polymorphism better here by moving the code into a function
			// called PrepareForRendering() into ModelInstance, then we don't need the if check here
			// myAnimationBufferData = {};

			// Map the bones to VS shader
			// NOTE: I have to do this everytime even if not skinned because
			// it needs to reset the OB_HasBones to 0
			// TODO: Find a better solution to this
			/*
			{
				D3D11_MAPPED_SUBRESOURCE bufferData2;

				ZeroMemory(&bufferData2, sizeof(bufferData2));
				result = myContext.Map(myAnimationBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferData2);
				ThrowIfFailed(result);
				memcpy(bufferData2.pData, &myAnimationBufferData, sizeof(myAnimationBufferData));
				myContext.Unmap(myAnimationBuffer, 0);
			}
			*/


			AnimationBufferData& animData = myAnimationBuffer.Map(myContext);
			animData = {};
			myAnimationBuffer.Unmap(myContext);

			// animation is on cbuffer slot 2
			myContext.VSSetConstantBuffers(2, 1, myAnimationBuffer.GetAddressOf());

			D3D11_MAPPED_SUBRESOURCE bufferData2;

			myDecalObjectBufferData.myToWorld = decalComponent->GetGameObject()->GetTransform().GetMatrix();
			auto temp = Mat4f::Transpose(myDecalObjectBufferData.myToWorld);

			aiMatrix4x4 mat = CUMatrixToAiMatrix(Mat4f::Transpose(myDecalObjectBufferData.myToWorld));
			mat.Inverse();

			myDecalObjectBufferData.myToInverseWorld = Mat4f::Transpose(AiMatrixToCUMatrix(mat));

			// myDecalObjectBufferData.myToInverseWorld = myDecalObjectBufferData.myToWorld.Inverse();

			// yes or no
			myDecalObjectBufferData.myUVScale = { 1337.f, 1338.f };

			if (decalMat->GetAlbedoTexture() && decalMat->GetAlbedoTexture()->IsValid())
			{
				myDecalObjectBufferData.myHasAlbedoTexture = 1;
			}
			else
			{
				myDecalObjectBufferData.myHasAlbedoTexture = 0;
			}

			if (decalMat->GetMaterialTexture() && decalMat->GetMaterialTexture()->IsValid())
			{
				myDecalObjectBufferData.myHasMaterialTexture = 1;
			}
			else
			{
				myDecalObjectBufferData.myHasMaterialTexture = 0;
			}

			if (decalMat->GetNormalTexture() && decalMat->GetNormalTexture()->IsValid())
			{
				myDecalObjectBufferData.myHasNormalTexture = 1;
			}
			else
			{
				myDecalObjectBufferData.myHasNormalTexture = 0;
			}

			ZeroMemory(&bufferData2, sizeof(bufferData2));
			result = myContext.Map(myDecalObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferData2);
			ThrowIfFailed(result);

			memcpy(bufferData2.pData, &myDecalObjectBufferData, sizeof(myDecalObjectBufferData));
			myContext.Unmap(myDecalObjectBuffer, 0);

			assert(fbxResource->Get().myPrimitiveTopology != D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED);

			myContext.IASetPrimitiveTopology(fbxResource->Get().myPrimitiveTopology);
			//myContext.IASetVertexBuffers(0, 1, &modelData.myVertexBuffer, &modelData.myStride, &modelData.myOffset);
			myContext.IASetVertexBuffers(0, 1, &vertexBuffer, stride, offset);
			//myContext.IASetIndexBuffer(modelData.myIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
			myContext.IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

			myContext.VSSetConstantBuffers(1, 1, &myDecalObjectBuffer);





			// Start at slot 1 because environmentlight has slot 1
			myContext.PSSetConstantBuffers(1, 1, &myDecalObjectBuffer);

			//myContext.PSSetShader(myGBufferPixelShader->Get().GetPixelShader(), nullptr, 0);

			//for (const auto& meshData : modelData.myMeshDatas)
			const auto& meshDatas = fbxResource->Get().GetMeshDatas();

			// for (int meshIndex = 0; meshIndex < meshDatas.size(); ++meshIndex)
			{
				// hard coded because cube only has 1 mesh thank god
				const int meshIndex = 0;
				const auto& meshData = meshDatas[meshIndex];

				decalComponent->GetMaterialInstance().UpdateMaterialCustomValues();

				//for (auto& pass : material.GetEffect()->GetPasses())
				EffectPass* pass = culledDecalInstance.myEffectPass;

				pass->MapConstantBuffersDataToGPU(myContext);

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

					EffectPassState state = pass->BindToPipeline(&myContext,
						BindEffectFlags_PixelShader |
						BindEffectFlags_VertexShader |
						BindEffectFlags_EffectSpecificStuff);
					//material->BindBoundTextures(&myContext);

					myContext.IASetInputLayout(pass->GetVertexShader()->Get().GetInputLayout());
					myContext.VSSetShader(pass->GetVertexShader()->Get().GetVertexShader(), nullptr, 0);

					// TODO: Consider making an TextureArray type that can convert to this array internally
					ID3D11ShaderResourceView* textures[] =
					{
						myDecalObjectBufferData.myHasAlbedoTexture != 0
						? decalMat->GetAlbedoTexture()->Get().GetSRV()
						: nullptr,

						myDecalObjectBufferData.myHasNormalTexture != 0
						? decalMat->GetNormalTexture()->Get().GetSRV()
						: nullptr,

						myDecalObjectBufferData.myHasMaterialTexture != 0
						? decalMat->GetMaterialTexture()->Get().GetSRV()
						: nullptr,

						myDecalObjectBufferData.myHasEmissiveTexture != 0
						? decalMat->GetEmissiveTexture()->Get().GetSRV()
						: nullptr,

						//meshData.myTexture[0]->Get().GetSRV(),
						//meshData.myTexture[1]->Get().GetSRV(),
						//meshData.myTexture[2]->Get().GetSRV(),
					};
					myContext.PSSetShaderResources(1, std::size(textures), &textures[0]);
					myContext.PSSetShader(pass->GetPixelShader()->Get().GetPixelShader(), nullptr, 0);




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

	void DeferredRenderer::CreateInstanceBuffer()
	{
		D3D11_BUFFER_DESC desc;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.ByteWidth = sizeof(InstanceBufferData) * ShaderConstants::MaxInstanceCount;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.StructureByteStride = sizeof(InstanceBufferData);

		HRESULT result = myDevice.CreateBuffer(&desc, nullptr, &myInstanceBuffer);

		assert(SUCCEEDED(result));

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = ShaderConstants::MaxInstanceCount;

		result = myDevice.CreateShaderResourceView(myInstanceBuffer, &srvDesc, &myInstanceBufferSRV);

		assert(SUCCEEDED(result));
	}
}