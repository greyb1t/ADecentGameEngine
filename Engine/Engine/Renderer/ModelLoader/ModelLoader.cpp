#include "pch.h"
#include "ModelLoader.h"
#include "../Vertex.h"
#include "LoaderModel.h"

#include "Engine/Renderer/Shaders/PixelShader.h"
#include "Engine/Renderer/Shaders/VertexShader.h"
#include "Engine/Renderer/Model/Model.h"
#include "Engine/Renderer/Texture/Texture2D.h"
#include "Engine/ResourceManagement/Resources/PixelShaderResource.h"
#include "Engine/ResourceManagement/Resources/VertexShaderResource.h"
#include "Engine/ResourceManagement/Resources/TextureResource.h"
#include "Engine/ResourceManagement/Resources/ModelResource.h"
#include "Engine/ResourceManagement/ResourceManager.h"

namespace Engine
{
	ModelLoader::ModelLoader(
		Engine::ResourceManager& aResourceManager)
		: myResourceManager(aResourceManager)
	{
	}

	ModelLoader::~ModelLoader()
	{
	}

	bool ModelLoader::Init(ID3D11Device* aDevice)
	{
		myDevice = aDevice;

		return true;
	}

	ModelRef ModelLoader::LoadModel(const std::string& aModelPath)
	{
		auto fbxResource = myResourceManager.CreateRef<Engine::ModelResource>(aModelPath);
		fbxResource->RequestLoading();

		return fbxResource;
	}
}