#pragma once

#include "..\Vertex.h"
#include "..\VertexTypes.h"
#include "Engine/Renderer/MeshPointsData.h"

namespace Assimp
{
	class Importer;
}

namespace Engine
{
	class ResourceManager;
	class ModelResource;
}

namespace Engine
{
	class Model;

	class FbxLoader
	{
	public:
		FbxLoader(ID3D11Device* aDevice, ResourceManager& aResourceManagerHandler);

		bool Init();

		Owned<Model> LoadFbx(Path aFbxPath, ModelResource& aFbxResource);
#if 0
		Owned<Model> LoadCustomModel(const Path& aModelPath);
#endif
		Owned<Model> LoadCustomModel2(const Path& aModelPath, ModelResource& aFbxResource);
		std::vector<MeshPointsData> LoadMeshDatas(const Path& aModelPath);

	private:
		ComPtr<ID3D11Buffer> CreateVertexBuffer(const std::vector<DefaultVertex>& aVertices);
		ComPtr<ID3D11Buffer> CreateIndexBuffer(const std::vector<unsigned int>& aIndices);

		ComPtr<ID3D11Buffer> CreateVertexBufferRaw(const DefaultVertex* aVertices, const int aVerticesCount);
		ComPtr<ID3D11Buffer> CreateIndexBufferRaw(const unsigned int* aIndices, const int aIndicesCount);

	private:
		ResourceManager& myResourceManagerHandler;
		ID3D11Device* myDevice = nullptr;
		Owned<Assimp::Importer> myImporter;
	};
}