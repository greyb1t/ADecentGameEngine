#pragma once

#include "CreationState.h"
#include "Engine\ResourceManagement\ResourceRef.h"

namespace Engine
{
	class LoaderModel;
	class LoaderMesh;
	class Editor;
}

class GameObject;

namespace Engine
{
	class Resource;

	struct ModelResult
	{
	};

	class ModelImportState : public CreationState
	{
	public:
		enum class State
		{
			Default,
			MassImport,
			HoudiniFBXMssImport,
		};

		ModelImportState();
		ModelImportState(const std::filesystem::path& aPath, Editor& aEditor);
		~ModelImportState();

		bool Draw() override;

		void ParseFiles(const std::vector<std::filesystem::path>& aPaths) override;

		friend class ModelTextureResource;

		State GetState() const;

	private:
		ImVec4 GetColorFromPath(const std::string& aPath) const;
		void DrawTextureTable(LoaderMesh& aLoaderMesh);

		bool DrawDefault();

		bool DrawMassImport();
		bool DrawHoudiniFBXMassImport();
	private:
		// tabort myLoaderModel...
		// Owned<LoaderModel> myLoaderModel = nullptr;
		Editor* myEditor = nullptr;

		std::vector<Owned<LoaderModel>> myLoaderModels;

		std::vector<Owned<Resource>> myResourceToHandle;

		ModelResult myModelResult;

		// Each index is the mesh
		std::vector<MaterialRef> myMaterialRefs;

		// bool myMassImport = false;
		// 
		// bool myImportHoudiniFBX = false;

		State myState = State::Default;
	};
}