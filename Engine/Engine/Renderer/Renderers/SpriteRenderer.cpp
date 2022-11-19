#include "pch.h"
#include "SpriteRenderer.h"
#include "Engine/Renderer/Directx11Framework.h"
#include "Engine/Renderer/Camera/Camera.h"
#include "Engine/Renderer/WindowHandler.h"
#include "Engine/ResourceManagement/Resources/TextureResource.h"
#include "Engine/Renderer/Texture/Texture2D.h"
#include "Engine/ResourceManagement/Resources/VertexShaderResource.h"
#include "Engine/Renderer/Shaders/VertexShader.h"
#include "Engine/Renderer/Shaders/PixelShader.h"
#include "Engine/Renderer/Shaders/GeometryShader.h"
#include "Engine/ResourceManagement/Resources/GeometryShaderResource.h"
#include "Engine/ResourceManagement/Resources/PixelShaderResource.h"
#include "Engine/Renderer/VertexTypes.h"
#include "Engine/Renderer/Material/MeshMaterial.h"
#include "Engine/GameObject/Components/SpriteComponent.h"
#include "Engine/Renderer/Material/EffectPass.h"
#include "Engine/GameObject/GameObject.h"

namespace Engine
{
	bool SpriteRenderer::Init(
		ResourceManager& aResourceManager,
		Directx11Framework& aFramework,
		WindowHandler& aWindowHandler)
	{
		myFramework = &aFramework;
		myContext = aFramework.GetContext();
		myDevice = aFramework.GetDevice();
		myWindowHandler = &aWindowHandler;

		if (!CreateFrameBuffer())
		{
			LOG_ERROR(LogType::Renderer) << "failed to create sprite renderer frame buffer";
			return false;
		}

		if (!CreateVertexBuffer())
		{
			LOG_ERROR(LogType::Renderer) << "failed to create sprite renderer vertex buffer";
			return false;
		}

		myGeometryShader =
			aResourceManager.CreateRef<GeometryShaderResource>(
				"Assets/Shaders/Engine/Sprites/SpriteGeometryShader");
		myGeometryShader->RequestLoading();

		return true;
	}

	void SpriteRenderer::Render(const Camera* aCamera, const std::vector<SpriteComponent*>& aSpriteInstances)
	{
		ZoneNamedN(zone1, "SpriteRenderer::Render", true);

		D3D11_MAPPED_SUBRESOURCE bufferData = {};

		// Update frame buffer
		{
			bufferData = {};

			HRESULT result = myContext->Map(myFrameBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferData);

			if (FAILED(result))
			{
				LOG_ERROR(LogType::Renderer) << "Context.Map() failed";
				return;
			}
			myFrameBufferData.myToCamera = Mat4f::GetFastInverse(aCamera->GetTransform().ToMatrix());
			myFrameBufferData.myToProjection = aCamera->GetProjectionMatrix();
			myFrameBufferData.myCameraPosition = Vec4f{
				aCamera->GetTransform().GetPosition().x,
				aCamera->GetTransform().GetPosition().y,
				aCamera->GetTransform().GetPosition().z,
				1.0f
			};
			
			myFrameBufferData.myNear = aCamera->GetNearPlane();
			myFrameBufferData.myFar = aCamera->GetFarPlane();

			myFrameBufferData.myResolution = myWindowHandler->GetRenderingSize<float>();

			memcpy(bufferData.pData, &myFrameBufferData, sizeof(myFrameBufferData));

			myContext->Unmap(myFrameBuffer, 0);

			myContext->GSSetConstantBuffers(0, 1, &myFrameBuffer);
			myContext->VSSetConstantBuffers(0, 1, &myFrameBuffer);
			myContext->PSSetConstantBuffers(0, 1, &myFrameBuffer);
		}

		const auto renderResolution = myWindowHandler->GetRenderingSize<float>();

		// Sort them based on their sort order
		std::vector<SpriteComponent*> sortedSpriteInstances = aSpriteInstances;

		std::sort(sortedSpriteInstances.begin(), sortedSpriteInstances.end(),
			[](const SpriteComponent* aLeft, const SpriteComponent* aRight)
			{
				return aLeft->GetSortOrder() < aRight->GetSortOrder();
			});

		for (const auto& spriteInstance : sortedSpriteInstances)
		{
			auto& materialInstance = spriteInstance->GetMaterialInstance();

			if (materialInstance.GetMaterial()->GetType() != MaterialType::Sprite)
			{
				LOG_WARNING(LogType::Renderer) << "A sprite has a material that is not of type sprite: " << spriteInstance->GetGameObject()->GetName();
				continue;
			}

			// const auto& sprite = spriteInstance->GetSprite();
			// 
			// const Sprite::SpriteData& spriteData = sprite->GetData();

			// Update vertex buffer
			{
				bufferData = {};

				HRESULT result = myContext->Map(myVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferData);

				if (FAILED(result))
				{
					LOG_ERROR(LogType::Renderer) << "Context.Map() failed";
					return;
				}

				SpriteVertex spriteVertexToDraw = spriteInstance->GetSpriteVertex();

				const auto res = myWindowHandler->GetRenderingSize<float>();
				const float ratio = res.x / res.y;

				const auto targetSize = myWindowHandler->GetTargetSize().CastTo<float>();

				const auto scaleFactors = Vec2f(
					renderResolution.x / targetSize.x,
					renderResolution.y / targetSize.y);

				const auto& textureSize = spriteInstance->GetTexture()->Get().GetImageSize();

				// Adjust the size to the shader here, then we can 
				// avoid getting resolution and texture size into the shader
				spriteVertexToDraw.mySize = Vec2f(
					spriteVertexToDraw.mySize.x * static_cast<float>(textureSize.x) / renderResolution.x * ratio * scaleFactors.x,
					spriteVertexToDraw.mySize.y * static_cast<float>(textureSize.y) / renderResolution.y * scaleFactors.y);

				// spriteVertexToDraw.myScaleFactor =
				// 	static_cast<float>(spriteData.myTextureSize.x) /
				// 	static_cast<float>(spriteData.myTextureSize.y);

				// spriteVertexToDraw.myPosition = {0.7f, 0.7f, 0, 1 };

				// spriteVertexToDraw.myColor.w = 0.2f;

				memcpy(
					bufferData.pData,
					&spriteVertexToDraw,
					sizeof(SpriteVertex));

				myContext->Unmap(myVertexBuffer.Get(), 0);
			}

			myContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

			UINT stride = sizeof(SpriteVertex);
			UINT offset = 0;

			myContext->IASetVertexBuffers(
				0,
				1,
				myVertexBuffer.GetAddressOf(),
				&stride,
				&offset);

			// Updates the parent material values in the buffers
			materialInstance.UpdateMaterialCustomValues();

			// TODO: I don't need to store Material in Sprite anymore, its already in MaterialInstance
			// which is in SpriteInstance

			// spriteData.myMaterial->BindBoundTextures(myContext);

			for (auto& pass : materialInstance.GetMaterial()->GetPasses())
			{
				pass->MapConstantBuffersDataToGPU(*myContext);

				EffectPassState state = pass->BindToPipeline(myContext,
					BindEffectFlags_PixelShader |
					BindEffectFlags_VertexShader |
					BindEffectFlags_EffectSpecificStuff);

				//myContext->VSSetConstantBuffers(1, 1, &myObjectBuffer);

				myContext->IASetInputLayout(pass->GetVertexShader()->Get().GetInputLayout());

				// TODO: do this in the material Bind?
				myContext->VSSetShader(pass->GetVertexShader()->Get().GetVertexShader(), nullptr, 0);
				myContext->PSSetShader(pass->GetPixelShader()->Get().GetPixelShader(), nullptr, 0);

				if (pass->GetGeometryShader() && pass->GetGeometryShader()->IsValid())
				{
					myContext->GSSetShader(pass->GetGeometryShader()->Get().GetGeometryShader(), nullptr, 0);
				}
				else
				{
					myContext->GSSetShader(myGeometryShader->Get().GetGeometryShader(), nullptr, 0);
				}

				myContext->PSSetShaderResources(0, 1, spriteInstance->GetTexture()->Get().GetSRVPtr());


				// TODO: Support rendering multiple sprites with same texture at same time?
				myContext->Draw(1, 0);

				state.ResetState(*myContext);
			}

			////myContext->VSSetConstantBuffers(1, 1, &myObjectBuffer);

			//// TODO: do this in the material Bind?
			//myContext->VSSetShader(spriteData.myMaterial->GetVertexShader()->Get().GetVertexShader(), nullptr, 0);
			//myContext->PSSetShader(spriteData.myMaterial->GetPixelShader()->Get().GetPixelShader(), nullptr, 0);

			//myContext->GSSetShader(spriteData.myGeometryShader->Get().GetGeometryShader(), nullptr, 0);

			//myContext->PSSetShaderResources(0, 1, spriteData.myTexture->Get().GetSRVPtr());


			//// TODO: Support rendering multiple sprites with same texture at same time?
			//myContext->Draw(1, 0);
		}

		myContext->GSSetShader(nullptr, nullptr, 0);
	}

	void SpriteRenderer::RenderSingle2D(SpriteComponent* aSprite)
	{
		D3D11_MAPPED_SUBRESOURCE bufferData = {};

		// Update frame buffer
		{
			bufferData = {};

			HRESULT result = myContext->Map(myFrameBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferData);

			if (FAILED(result))
			{
				LOG_ERROR(LogType::Renderer) << "Context.Map() failed";
				return;
			}
			// This is not needed for 2D sprites.
			myFrameBufferData.myToCamera = Mat4f();
			myFrameBufferData.myToProjection = Mat4f();
			myFrameBufferData.myCameraPosition = Vec4f{
				0.0f,
				0.0f,
				0.0f,
				1.0f
			};

			myFrameBufferData.myNear = 0.0f;
			myFrameBufferData.myFar = 0.0f;

			myFrameBufferData.myResolution = myWindowHandler->GetRenderingSize<float>();

			memcpy(bufferData.pData, &myFrameBufferData, sizeof(myFrameBufferData));

			myContext->Unmap(myFrameBuffer, 0);

			myContext->GSSetConstantBuffers(0, 1, &myFrameBuffer);
			myContext->VSSetConstantBuffers(0, 1, &myFrameBuffer);
			myContext->PSSetConstantBuffers(0, 1, &myFrameBuffer);
		}

		const auto renderResolution = myWindowHandler->GetRenderingSize<float>();

		auto& materialInstance = aSprite->GetMaterialInstance();

		if (materialInstance.GetMaterial()->GetType() != MaterialType::Sprite)
		{
			LOG_WARNING(LogType::Renderer) << "A sprite has a material that is not of type sprite: " << aSprite->GetGameObject()->GetName();
			return;
		}

		// const auto& sprite = spriteInstance->GetSprite();
		// 
		// const Sprite::SpriteData& spriteData = sprite->GetData();

		// Update vertex buffer
		{
			bufferData = {};

			HRESULT result = myContext->Map(myVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferData);

			if (FAILED(result))
			{
				LOG_ERROR(LogType::Renderer) << "Context.Map() failed";
				return;
			}

			SpriteVertex spriteVertexToDraw = aSprite->GetSpriteVertex();

			const auto res = myWindowHandler->GetRenderingSize<float>();
			const float ratio = res.x / res.y;

			const auto targetSize = myWindowHandler->GetTargetSize().CastTo<float>();

			const auto scaleFactors = Vec2f(
				renderResolution.x / targetSize.x,
				renderResolution.y / targetSize.y);

			const auto& textureSize = aSprite->GetTexture()->Get().GetImageSize();

			// Adjust the size to the shader here, then we can 
			// avoid getting resolution and texture size into the shader
			spriteVertexToDraw.mySize = Vec2f(
				spriteVertexToDraw.mySize.x * static_cast<float>(textureSize.x) / renderResolution.x * ratio * scaleFactors.x,
				spriteVertexToDraw.mySize.y * static_cast<float>(textureSize.y) / renderResolution.y * scaleFactors.y);

			// spriteVertexToDraw.myScaleFactor =
			// 	static_cast<float>(spriteData.myTextureSize.x) /
			// 	static_cast<float>(spriteData.myTextureSize.y);

			// spriteVertexToDraw.myPosition = {0.7f, 0.7f, 0, 1 };

			// spriteVertexToDraw.myColor.w = 0.2f;

			memcpy(
				bufferData.pData,
				&spriteVertexToDraw,
				sizeof(SpriteVertex));

			myContext->Unmap(myVertexBuffer.Get(), 0);
		}

		myContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

		UINT stride = sizeof(SpriteVertex);
		UINT offset = 0;

		myContext->IASetVertexBuffers(
			0,
			1,
			myVertexBuffer.GetAddressOf(),
			&stride,
			&offset);

		// Updates the parent material values in the buffers
		materialInstance.UpdateMaterialCustomValues();

		// TODO: I don't need to store Material in Sprite anymore, its already in MaterialInstance
		// which is in SpriteInstance

		// spriteData.myMaterial->BindBoundTextures(myContext);

		for (auto& pass : materialInstance.GetMaterial()->GetPasses())
		{
			pass->MapConstantBuffersDataToGPU(*myContext);

			EffectPassState state = pass->BindToPipeline(myContext,
				BindEffectFlags_PixelShader |
				BindEffectFlags_VertexShader |
				BindEffectFlags_EffectSpecificStuff);

			//myContext->VSSetConstantBuffers(1, 1, &myObjectBuffer);

			myContext->IASetInputLayout(pass->GetVertexShader()->Get().GetInputLayout());

			// TODO: do this in the material Bind?
			myContext->VSSetShader(pass->GetVertexShader()->Get().GetVertexShader(), nullptr, 0);
			myContext->PSSetShader(pass->GetPixelShader()->Get().GetPixelShader(), nullptr, 0);

			if (pass->GetGeometryShader() && pass->GetGeometryShader()->IsValid())
			{
				myContext->GSSetShader(pass->GetGeometryShader()->Get().GetGeometryShader(), nullptr, 0);
			}
			else
			{
				myContext->GSSetShader(myGeometryShader->Get().GetGeometryShader(), nullptr, 0);
			}

			myContext->PSSetShaderResources(0, 1, aSprite->GetTexture()->Get().GetSRVPtr());

			myContext->Draw(1, 0);

			state.ResetState(*myContext);
		}

		myContext->GSSetShader(nullptr, nullptr, 0);
	}

	bool SpriteRenderer::CreateFrameBuffer()
	{
		D3D11_BUFFER_DESC bufDesc = {};
		{
			bufDesc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
			bufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			bufDesc.ByteWidth = sizeof(FrameBufferData);
		}

		HRESULT result = myDevice->CreateBuffer(&bufDesc, nullptr, &myFrameBuffer);

		if (FAILED(result))
		{
			LOG_ERROR(LogType::Renderer) << "CreateBuffer failed to create frame cbuffer";
			return false;
		}

		return true;
	}

	bool SpriteRenderer::CreateVertexBuffer()
	{
		// Vertex buffer
		{
			D3D11_BUFFER_DESC vertexBufferDesc = {};
			{
				vertexBufferDesc.ByteWidth = sizeof(SpriteVertex);

				vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
				vertexBufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;

				// TODO: not needed for sprites, or is it? unsure atm
				vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			}

			HRESULT result = myDevice->CreateBuffer(&vertexBufferDesc, nullptr, &myVertexBuffer);

			if (FAILED(result))
			{
				return false;
			}
		}

		return true;
	}

}