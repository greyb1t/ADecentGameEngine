#include "pch.h"
#include "MaterialFactory.h"

#include "Engine\ResourceManagement\Resources\PixelShaderResource.h"
#include "Engine/Renderer\VertexTypes.h"
#include "Engine\ResourceManagement\ResourceManager.h"
#include "Engine/ResourceManagement/Resources/MaterialResource.h"
#include "Engine/Editor/FileTypes.h"

namespace Engine
{
	MaterialFactory::MaterialFactory(
		JsonManager& aJsonManager,
		MaterialFactoryContext& aFactoryContext)
		: myJsonManager(aJsonManager),
		myFactoryContext(aFactoryContext)
	{
	}

	bool MaterialFactory::Init()
	{
		if (!InitDefaultMaterials())
		{
			return false;
		}

		//if (!LoadMeshMaterials("Assets\\Materials\\Mesh"))
		if (!LoadMeshMaterials("Assets"))
		{
			return false;
		}

		//if (!LoadSpriteMaterials("Assets\\Materials\\Sprite"))
		if (!LoadSpriteMaterials("Assets"))
		{
			return false;
		}

		return true;
	}

	MaterialRef MaterialFactory::GetMeshMaterial(const std::string& aUnrealMaterialName)
	{
		std::string* s = ConvertMaterialNameToPath(aUnrealMaterialName);

		if (s != nullptr)
		{
			auto mat = myFactoryContext.myResourceManager->CreateRef<Engine::MaterialResource>(*s);

			return mat;
		}
		else
		{
			return myDefaultModelMaterial;
		}
	}

	MaterialRef MaterialFactory::GetSpriteMaterial(const std::string& aMaterialName)
	{
		std::string* s = ConvertSpriteMaterialNameToPath(aMaterialName);

		if (s != nullptr)
		{
			auto mat = myFactoryContext.myResourceManager->CreateRef<Engine::MaterialResource>(*s);

			return mat;
		}
		else
		{
			return myDefaultSpriteMaterial;
		}
	}

	const MaterialRef& MaterialFactory::GetDefaultMeshMaterialDeferred() const
	{
		return myDefaultModelMaterial;
	}

	const MaterialRef& MaterialFactory::GetDefaultMeshMaterialForward() const
	{
		return myDefaultModelForwardMaterial;
	}

	const MaterialRef& MaterialFactory::GetDefaultSpriteMaterial() const
	{
		return myDefaultSpriteMaterial;
	}

	const MaterialRef& MaterialFactory::GetDefault3DSpriteMaterial() const
	{
		return myDefaultSprite3DMaterial;
	}

	std::string* MaterialFactory::ConvertMaterialNameToPath(const std::string& aFileName)
	{
		const auto findResult = myMeshMaterialNamesToFilePaths.find(aFileName);
		const bool exists = findResult != myMeshMaterialNamesToFilePaths.end();

		if (exists)
		{
			return &findResult->second;
		}
		else
		{
			return nullptr;
		}
	}

	std::string* MaterialFactory::ConvertSpriteMaterialNameToPath(const std::string& aFileName)
	{
		const auto findResult = mySpriteMaterialNamesToFilePaths.find(aFileName);
		const bool exists = findResult != mySpriteMaterialNamesToFilePaths.end();

		if (exists)
		{
			return &findResult->second;
		}
		else
		{
			return nullptr;
		}
	}

	const std::unordered_map<std::string, std::string>&
		MaterialFactory::GetMeshMaterialNamesToFilePaths() const
	{
		return myMeshMaterialNamesToFilePaths;
	}

	const std::unordered_map<std::string, std::string>&
		MaterialFactory::GetSpriteMaterialNamesToFilePaths() const
	{
		return mySpriteMaterialNamesToFilePaths;
	}

	bool MaterialFactory::LoadMeshMaterials(const std::filesystem::path& aPathToSearch)
	{
		for (auto& dir : std::filesystem::recursive_directory_iterator(aPathToSearch))
		{
			if (dir.path().has_extension())
			{
				std::string extension = dir.path().extension().string();

				for (auto& c : extension)
				{
					c = static_cast<char>(::tolower(static_cast<int>(c)));
				}

				if (extension == FileType::Extension_MeshMaterial)
				{
					const std::string& path = dir.path().string();
					const std::string stem = dir.path().stem().string();

					myMeshMaterialNamesToFilePaths[stem] = path;
				}
			}
		}

		return true;
	}

	bool MaterialFactory::LoadSpriteMaterials(const std::filesystem::path& aPathToSearch)
	{
		for (auto& dir : std::filesystem::recursive_directory_iterator(aPathToSearch))
		{
			if (dir.path().has_extension())
			{
				std::string extension = dir.path().extension().string();

				for (auto& c : extension)
				{
					c = static_cast<char>(::tolower(static_cast<int>(c)));
				}

				if (extension == FileType::Extension_SpriteMaterial)
				{
					const std::string& path = dir.path().string();
					const std::string stem = dir.path().stem().string();

					mySpriteMaterialNamesToFilePaths[stem] = path;
				}
			}
		}

		return true;
	}

	bool MaterialFactory::InitDefaultMaterials()
	{
		// The default ones
		if (!LoadMeshMaterials("Assets\\Engine\\Materials"))
		{
			return false;
		}

		if (!LoadSpriteMaterials("Assets\\Engine\\Materials"))
		{
			return false;
		}

		// Default Mesh Deferred
		{
			myDefaultModelMaterial =
				myFactoryContext.myResourceManager->CreateRef<Engine::MaterialResource>(
					myMeshMaterialNamesToFilePaths.at("DefaultDeferred"));

			myDefaultModelMaterial->Load();

			if (!myDefaultModelMaterial->IsValid())
			{
				return false;
			}
		}

		{
			myDefaultModelForwardMaterial =
				myFactoryContext.myResourceManager->CreateRef<Engine::MaterialResource>(
					myMeshMaterialNamesToFilePaths.at("DefaultForward"));

			myDefaultModelForwardMaterial->Load();

			if (!myDefaultModelForwardMaterial->IsValid())
			{
				return false;
			}
		}

		{
			myDefaultSpriteMaterial =
				myFactoryContext.myResourceManager->CreateRef<Engine::MaterialResource>(
					mySpriteMaterialNamesToFilePaths.at("Default"));

			myDefaultSpriteMaterial->Load();

			if (!myDefaultSpriteMaterial->IsValid())
			{
				return false;
			}
		}

		{
			myDefaultSprite3DMaterial =
				myFactoryContext.myResourceManager->CreateRef<Engine::MaterialResource>(
					mySpriteMaterialNamesToFilePaths.at("Default3D"));

			myDefaultSprite3DMaterial->Load();

			if (!myDefaultSprite3DMaterial->IsValid())
			{
				return false;
			}
		}

		return true;
	}
}
