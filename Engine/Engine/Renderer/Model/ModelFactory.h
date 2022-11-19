#pragma once

#include "Engine\ResourceManagement\ResourceRef.h"

namespace Engine
{
	class ResourceManager;
}

namespace Engine
{
	class Model;
	class ModelLoader;
	class SkinnedModel;
	class FbxLoader;
	class MaterialFactory;

	class ModelFactory
	{
	public:
		ModelFactory(ResourceManager& aResourceManager);
		ModelFactory(const ModelFactory&) = delete;
		~ModelFactory();

		bool Init(ID3D11Device* aDevice);

		ModelRef LoadModel(const std::string& aModelPath, const std::string& aIdentifier);

		// Added for convenience of not requiring 1 to 1 file structure as Unreal has
		ModelRef LoadModelByFileName(const std::string& aFileName, const std::string& aIdentifier);

		std::string* ConvertModelFileNameToPath(const std::string& aFileName);
		std::string* ConvertFbxFileNameToPath(const std::string& aFileName);

	private:
		void CacheAllFbxFiles();
		void CacheAllModelFiles();

	private:
		ID3D11Device* myDevice = nullptr;

		Owned<ModelLoader> myModelLoader = nullptr;

		ResourceManager& myResourceManager;

		std::unordered_map<std::string, std::string> myCachesFBXFileNamesToFilePaths;
		std::unordered_map<std::string, std::string> myCachedModelFileNamesToFilePaths;
	};
}