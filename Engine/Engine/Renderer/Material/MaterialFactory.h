#pragma once

#include "Engine/Renderer/RendererType.h"

// Must have these in header because unordered_map with Owned<> inside of it cries
#include "Engine/Renderer/Material/MeshMaterial.h"

#include "Engine/Renderer/Material/MaterialFactoryContext.h"

class JsonManager;

namespace Engine
{
	class ResourceManager;
	struct LoaderMaterial;
	struct LoaderMaterials;
	class RenderManager;

	class MaterialFactory
	{
	public:
		MaterialFactory(
			JsonManager& aJsonManager,
			MaterialFactoryContext& aFactoryContext);

		bool Init();

		MaterialRef GetMeshMaterial(const std::string& aUnrealMaterialName);
		MaterialRef GetSpriteMaterial(const std::string& aMaterialName);

		const MaterialRef& GetDefaultMeshMaterialDeferred() const;
		const MaterialRef& GetDefaultMeshMaterialForward() const;

		const MaterialRef& GetDefaultSpriteMaterial() const;
		const MaterialRef& GetDefault3DSpriteMaterial() const;

		MaterialFactoryContext& GetFactoryContext() { return myFactoryContext; }

		std::string* ConvertMaterialNameToPath(const std::string& aFileName);
		std::string* ConvertSpriteMaterialNameToPath(const std::string& aFileName);

		const std::unordered_map<std::string, std::string>& GetMeshMaterialNamesToFilePaths() const;
		const std::unordered_map<std::string, std::string>& GetSpriteMaterialNamesToFilePaths() const;

	private:
		bool LoadMeshMaterials(const std::filesystem::path& aPathToSearch);
		bool LoadSpriteMaterials(const std::filesystem::path& aPathToSearch);
		bool InitDefaultMaterials();

	private:
		JsonManager& myJsonManager;
		MaterialFactoryContext myFactoryContext;

		std::unordered_map<std::string, std::string> myMeshMaterialNamesToFilePaths;
		std::unordered_map<std::string, std::string> mySpriteMaterialNamesToFilePaths;

		MaterialRef myDefaultModelMaterial;
		MaterialRef myDefaultModelForwardMaterial;
		MaterialRef myDefaultSpriteMaterial;
		MaterialRef myDefaultSprite3DMaterial;
	};
}