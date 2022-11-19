#include "pch.h"
#include "ParticleRenderer.h"
#include "../Directx11Framework.h"
#include "../Camera/Camera.h"
#include "../ResourceManagement/Resources/VertexShaderResource.h"
#include "Engine/Renderer/Shaders/VertexShader.h"
#include "../ResourceManagement/Resources/GeometryShaderResource.h"
#include "Engine/Renderer/Shaders/GeometryShader.h"
#include "../ResourceManagement/Resources/PixelShaderResource.h"
#include "Engine/Renderer/Shaders/PixelShader.h"
#include "../FrameConstantBuffer.h"
#include "Engine/ResourceManagement/Resources/TextureResource.h"
#include "Engine/Renderer/Texture/Texture2D.h"
#include "Engine/GameObject/Components/ParticleEmitterComponent.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/Renderer/Material/Material.h"

namespace Engine
{
	ParticleRenderer::ParticleRenderer(Directx11Framework& aFramework, FrameBufferTempName& aFrameBuffer)
		: myFramework(aFramework),
		myContext(*aFramework.GetContext()),
		myFrameBuffer(aFrameBuffer),
		myDevice(*aFramework.GetDevice())
	{
	}

	bool ParticleRenderer::Init()
	{
		if (!CreateObjectBuffer())
		{
			return false;
		}

		return true;
	}

	void ParticleRenderer::Render(
		const Camera* aCamera,
		const std::vector<ParticleEmitterComponent*>& aParticleEmitters)
	{
		ZoneNamedN(zone1, "ParticleRenderer::Render", true);

		myFrameBuffer.UpdateCamera(*aCamera);

		D3D11_MAPPED_SUBRESOURCE bufferData = {};

		for (const auto& instance : aParticleEmitters)
		{
			const VFX::ParticleEmitterDescription::ParticleData& emitterData = instance->GetDesc().myRenderData;

			if (!emitterData.myTexture || !emitterData.myTexture->IsValid() || !emitterData.myParticleVertexBuffer)
				continue;

			auto& materialInstance = instance->GetMaterialInstance();

			if (materialInstance.GetMaterial()->GetType() != MaterialType::Particle)
			{
				LOG_WARNING(LogType::Renderer) << "A particle has a material that is not of type particle: " << instance->GetGameObject()->GetName();
				continue;
			}

			const auto& emitterTransform = instance->GetGameObject()->GetTransform();

			// Sort the particles
			{
				const auto& pos = emitterTransform.GetPosition();

				auto& vertices = instance->GetVertices();

				for (auto& vert : vertices)
				{
					vert.myDistanceToCamera = (pos + vert.myPosition.ToVec3() - aCamera->GetTransform().GetPosition()).LengthSqr();
				}

				std::sort(vertices.begin(), vertices.end(),
					[](const ParticleVertex& aFirst, const ParticleVertex& aSecond)
					{
						return aFirst.myDistanceToCamera < aSecond.myDistanceToCamera;
					});
			}

			// const auto& emitter = instance->GetEmitter();

			// Update object buffer
			{
				myObjectBufferData.myToWorld = 
					instance->GetDescription().space == eSpace::LOCALSPACE ? 
					emitterTransform.GetMatrix() :
					Mat4f::Identity;

				bufferData = {};

				HRESULT result = myContext.Map(myObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferData);

				if (FAILED(result))
				{
					LOG_ERROR(LogType::Renderer) << "Context.Map() failed";
					return;
				}

				memcpy(bufferData.pData, &myObjectBufferData, sizeof(myObjectBufferData));

				myContext.Unmap(myObjectBuffer, 0);
			}


			// Update vertex buffer
			{
				bufferData = {};


				HRESULT result = myContext.Map(emitterData.myParticleVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferData);

				if (FAILED(result))
				{
					LOG_ERROR(LogType::Renderer) << "Context.Map() failed";
					return;
				}

				if (!instance->GetVertices().empty())
				{
					unsigned siz = sizeof(ParticleVertex) * std::min(instance->GetVertices().size(), static_cast<size_t>(instance->GetMaxParticles()));

					memcpy(
						bufferData.pData,
						&instance->GetVertices()[0],
						sizeof(ParticleVertex) * std::min(instance->GetVertices().size(), static_cast<size_t>(instance->GetMaxParticles())));
				}

				myContext.Unmap(emitterData.myParticleVertexBuffer, 0);
			}

			myContext.IASetPrimitiveTopology(emitterData.myPrimitiveTopology);
			//myContext.IASetInputLayout(emitterData.myVSShadder->Get().GetInputLayout());

			myContext.IASetVertexBuffers(
				0,
				1,
				&emitterData.myParticleVertexBuffer,
				&emitterData.myStride,
				&emitterData.myOffset);

			myContext.VSSetConstantBuffers(1, 1, &myObjectBuffer);
			//myContext.VSSetShader(emitterData.myVSShader->Get().GetVertexShader(), nullptr, 0);

			myContext.GSSetShader(emitterData.myGSShader->Get().GetGeometryShader(), nullptr, 0);

			// myContext.PSSetConstantBuffers(1, 1, &myObjectBuffer);
			// myContext.VSSetConstantBuffers(1, 1, &myObjectBuffer);

			myContext.PSSetShaderResources(0, 1, emitterData.myTexture->Get().GetSRVPtr());
			//myContext.PSSetShader(emitterData.myPSShader->Get().GetPixelShader(), nullptr, 0);


			//myContext.Draw(static_cast<UINT8>(instance->GetVertices().size()), 0);

			{
				instance->GetMaterialInstance().UpdateMaterialCustomValues();

				for (const auto& pass : instance->GetMaterialInstance().GetMaterial()->GetPasses())
				{
					pass->MapConstantBuffersDataToGPU(myContext);

					// EffectPass* pass = culledMeshInstance.myEffectPass;
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
							BindEffectFlags_EffectSpecificStuff |
							BindEffectFlags_PixelShader |
							BindEffectFlags_VertexShader);
						//material->BindBoundTextures(&myContext);

						myContext.IASetInputLayout(pass->GetVertexShader()->Get().GetInputLayout());
						myContext.VSSetShader(pass->GetVertexShader()->Get().GetVertexShader(), nullptr, 0);
						// TODO: Consider making an TextureArray type that can convert to this array internally
						myContext.PSSetShader(pass->GetPixelShader()->Get().GetPixelShader(), nullptr, 0);

						myContext.Draw(instance->GetVertices().size(), 0);

						state.ResetState(myContext);
					}
				}
			}

			// temp, move to better location?

			myContext.GSSetShader(nullptr, nullptr, 0);
		}
	}

	bool ParticleRenderer::CreateObjectBuffer()
	{
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
	}
}