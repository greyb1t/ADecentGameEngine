#pragma once

#include "ExtraSceneJsonData.h"
#include "SceneJson.h"
#include "Engine/Scene/Scene.h"
#include "LevelImporter/UnrealProject.h"

namespace Engine
{
	class SceneSerializer;
	// A scene that is created using a folder that contains extra json files
	// the reason we do it this way is because we still want to be able to create
	// a clean normal scene for e.g. MainMenu
	class FolderScene : public Scene
	{
	public:
		FolderScene() = default;
		virtual ~FolderScene();
		FolderScene(const FolderScene&) = default;

		void SetFolderPath(const Path& aFolderPath);

		virtual Result LoadScene() override;
		virtual Result LoadWorld() override;

		virtual bool Init();

		const Path& GetFolderPath() const;
		std::string GetName() const override;

		const std::string& GetSceneTypeName() const;

		// Each scene must implement this
		virtual FolderScene* Clone() const override;


	private:
		bool InitResourceReferencesFromWorld(const Path& aPath);
		bool InitResourceReferences();
	private:
		friend class SceneGraph;
		friend class SceneSerializerJson;
		friend class SceneRegistry;
		friend class Editor;

	protected:
		LevelImporter::UnrealProject myUnrealProject;

		Path myFolderPath;

	private:
		//ExtraSceneJsonData myExtraSceneData;
		SceneJson mySceneJson;

		size_t myTypeId = static_cast<size_t>(-1);
		std::string myTypeName;
	};
}