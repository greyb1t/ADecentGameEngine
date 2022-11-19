#pragma once

#include "../Vertex.h"
#include "../ResourceManagement/ResourceRef.h"

namespace Engine
{
	class ResourceManager;
}

namespace Engine
{
	class Model;
	struct ModelData;
	class Model;

	class ModelLoader
	{
	public:
		ModelLoader(Engine::ResourceManager& aResourceManager);
		~ModelLoader();

		bool Init(ID3D11Device* aDevice);

		ModelRef LoadModel(const std::string& aModelPath);

	private:
		ID3D11Device* myDevice = nullptr;

		Engine::ResourceManager& myResourceManager;
	};
}