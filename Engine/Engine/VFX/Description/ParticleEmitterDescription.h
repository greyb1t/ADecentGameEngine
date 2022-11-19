#pragma once

#include "EmitterDescription.h"
#include "ParticleDescription.h"
#include "Engine\ResourceManagement\ResourceRef.h"

namespace VFX
{
	struct ParticleEmitterDescription
	{
		struct ParticleData
		{
			unsigned int myStride = 0;
			unsigned int myOffset = 0;
			ID3D11Buffer* myParticleVertexBuffer = nullptr;
			//VertexShaderRef myVSShader;
			GeometryShaderRef myGSShader;
			//PixelShaderRef myPSShader;
			D3D11_PRIMITIVE_TOPOLOGY myPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
			TextureRef myTexture;
		};

		EmitterDescription base;
		VFX::ParticleDescription particleDescription;

		ParticleData myRenderData;
	};
}
