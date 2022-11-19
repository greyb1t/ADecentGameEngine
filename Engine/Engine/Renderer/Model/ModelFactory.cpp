#include "pch.h"
#include "ModelFactory.h"
#include "../Vertex.h"
#include "../ModelLoader/ModelLoader.h"
#include "../ModelLoader/FbxLoader.h"
#include "Engine/Editor/FileTypes.h"

namespace Engine
{
	ModelFactory::ModelFactory(Engine::ResourceManager& aResourceManager)
		: myResourceManager(aResourceManager)
	{
	}

	ModelFactory::~ModelFactory()
	{
	}

	bool ModelFactory::Init(ID3D11Device* aDevice)
	{
		myModelLoader = MakeOwned<ModelLoader>(myResourceManager);
		myModelLoader->Init(aDevice);

		CacheAllFbxFiles();
		CacheAllModelFiles();

		myDevice = aDevice;
		return true;
	}

	ModelRef ModelFactory::LoadModel(const std::string& aModelPath, const std::string& aIdentifier)
	{
		return myModelLoader->LoadModel(aModelPath);
	}

	std::string* ModelFactory::ConvertModelFileNameToPath(const std::string& aFileName)
	{
		const auto findResult = myCachedModelFileNamesToFilePaths.find(aFileName);
		const bool exists = findResult != myCachedModelFileNamesToFilePaths.end();

		if (exists)
		{
			return &findResult->second;
		}
		else
		{
			return nullptr;
		}
	}

	std::string* ModelFactory::ConvertFbxFileNameToPath(const std::string& aFileName)
	{
		const auto findResult = myCachesFBXFileNamesToFilePaths.find(aFileName);
		const bool exists = findResult != myCachesFBXFileNamesToFilePaths.end();

		if (exists)
		{
			return &findResult->second;
		}
		else
		{
			return nullptr;
		}
	}

	ModelRef ModelFactory::LoadModelByFileName(const std::string& aFileName, const std::string& aIdentifier)
	{
		const auto findResult = myCachedModelFileNamesToFilePaths.find(aFileName);
		const bool exists = findResult != myCachedModelFileNamesToFilePaths.end();

		if (!exists)
		{
			LOG_ERROR(LogType::Renderer) << "LoadModelByFileName(): Could not find FBX with name " << aFileName;

			// Load the placeholder model
			return LoadModel("", aIdentifier);
		}
		else
		{
			return LoadModel(findResult->second, aIdentifier);
		}
	}

	void ModelFactory::CacheAllFbxFiles()
	{
		for (auto& dir : std::filesystem::recursive_directory_iterator("Assets\\"))
		{
			if (dir.path().has_extension())
			{
				std::string extension = dir.path().extension().string();

				for (auto& c : extension)
				{
					c = static_cast<char>(::toupper(static_cast<int>(c)));
				}

				if (extension == ".FBX")
				{
					const std::string& path = dir.path().string();
					const std::string stem = dir.path().stem().string();

					myCachesFBXFileNamesToFilePaths[stem] = path;
				}
			}
		}
	}

	void ModelFactory::CacheAllModelFiles()
	{
		for (auto& dir : std::filesystem::recursive_directory_iterator("Assets\\"))
		{
			if (dir.path().has_extension())
			{
				std::string extension = dir.path().extension().string();

				for (auto& c : extension)
				{
					c = static_cast<char>(::tolower(static_cast<int>(c)));
				}

				if (extension == FileType::Extension_Model)
				{
					const std::string& path = dir.path().string();
					const std::string stem = dir.path().stem().string();

					myCachedModelFileNamesToFilePaths[stem] = path;
				}
			}
		}
	}

}