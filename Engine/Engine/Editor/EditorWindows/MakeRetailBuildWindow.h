#pragma once

#include "EditorWindow.h"

namespace Engine
{
	class MakeRetailBuildWindow : public EditorWindow
	{
	public:
		MakeRetailBuildWindow(Editor& aEditor);

		std::string GetName() const override;

	protected:
		void Draw(const float aDeltaTime) override;
		bool BuildAllShaders(const std::filesystem::path& aGamePath);
		void DrawSpecificScenesTable();
		bool CopyCurrentBinToDestination();
		void RemoveUnwantedScenes(const std::filesystem::path& aGamePath);

	private:
		Path myDestinationPath;

		bool myCompileShaders = true;
		bool myCleanDestination = true;
		bool myKeepOnlySpecificScenes = true;

		std::vector<std::filesystem::path> mySpecificScenesPaths;
	};
}