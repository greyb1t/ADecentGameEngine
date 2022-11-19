#include "pch.h"
#include "Renderer.h"
#include "Engine\Renderer\Material\EffectPass.h"
#include "Engine\ResourceManagement\Resources\PixelShaderResource.h"
#include "Engine\Renderer\Shaders\PixelShader.h"
#include "Engine\ResourceManagement\Resources\VertexShaderResource.h"
#include "Engine\Renderer\Shaders\VertexShader.h"
#include "Engine\ResourceManagement\Resources/ModelResource.h"
#include "Engine\Renderer\Model\Model.h"
#include "Engine\Renderer\Texture\Texture2D.h"
#include "Engine\ResourceManagement\Resources\TextureResource.h"
#include "Engine\Renderer\ConstantBuffer.h"
#include "Engine\Renderer\Material\Material.h"

Engine::Renderer::Renderer(ID3D11DeviceContext& aContext)
	: myContext(&aContext)
{
}

void Engine::Renderer::BindMaterial(const uint16_t aMaterialID, Material& aMaterial)
{
	// When changing the model, we must also assume the mesh changed
	// because the mesh currenctly holds the its textures

	// i is the index of the mesh in the model

	if (myBoundMaterialID != aMaterialID)
	{
		myBoundMaterialID = aMaterialID;

		aMaterial.Bind(*myContext);
	}
}

void Engine::Renderer::BindEffectPass(
	EffectPass& aEffectPass,
	const BindEffectFlags aFlags)
{
	if (myBoundEffectPassID != aEffectPass.GetID())
	{
		myBoundEffectPassID = aEffectPass.GetID();

		aEffectPass.BindToPipeline(myContext, aFlags);
	}
}

void Engine::Renderer::BindModelMesh(
	Model& aModel,
	const uint16_t aModelID,
	const uint16_t aMeshIndex,
	const BindModelFlags aFlags)
{
	if (myBoundModelID != aModelID || myBoundMeshIndex != aMeshIndex)
	{
		myBoundModelID = aModelID;
		myBoundMeshIndex = aMeshIndex;

		auto vertexBuffer = aModel.GetVertexBuffer();
		auto indexBuffer = aModel.GetIndexBuffer();
		auto stride = aModel.GetStride();
		auto offset = aModel.GetOffset();

		myContext->IASetVertexBuffers(0, 1, &vertexBuffer, stride, offset);

		myContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

		myContext->IASetPrimitiveTopology(aModel.myPrimitiveTopology);

		if ((aFlags & BindModelFlags_Textures) != 0)
		{
			//// When changing the model, we must also assume the mesh changed
			//// because the mesh currenctly holds the its textures

			//// i is the index of the mesh in the model
			//const auto& modelMesh = aModel.GetMeshDatas()[aMeshIndex];

			//ID3D11ShaderResourceView* textures[] =
			//{
			//	modelMesh.myTexture[0]->Get().GetSRV(),
			//	modelMesh.myTexture[1]->Get().GetSRV(),
			//	modelMesh.myTexture[2]->Get().GetSRV(),
			//};

			//myContext->PSSetShaderResources(
			//	1,
			//	static_cast<UINT>(modelMesh.myTexture.size()),
			//	&textures[0]);
		}
	}
}

bool Engine::Renderer::IsBound(EffectPass& aEffectPass) const
{
	return myBoundEffectPassID == aEffectPass.GetID();
}

bool Engine::Renderer::IsModelBound(const uint16_t aModelID, const uint16_t aMeshIndex) const
{
	return myBoundModelID == aModelID && myBoundMeshIndex == aMeshIndex;
}

void Engine::Renderer::ResetInternalBindState()
{
	myBoundEffectPassID = -1;
	myBoundModelID = -1;
	myBoundMeshIndex = -1;
	myBoundMaterialID = -1;

	std::fill(myBoundConstantBufferVS.begin(), myBoundConstantBufferVS.end(), -1);
	std::fill(myBoundConstantBufferPS.begin(), myBoundConstantBufferPS.end(), -1);
}
