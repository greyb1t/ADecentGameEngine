#pragma once

#include "Engine/Editor/EditorWindows/EditorWindow.h"
#include "Engine/Engine.h"
#include "Engine/ResourceManagement/ResourceRef.h"
#include "Engine/AssetDatabase.h"

namespace Engine
{
	class ResourceReferences;

	struct DirectoryEntry
	{
		DirectoryEntry() = default;
		DirectoryEntry(const std::filesystem::path& aPath, const bool aIsDirectory)
			: myPath(aPath),
			myIsDirectory(aIsDirectory)
		{
		}

		bool myIsDirectory = false;
		std::filesystem::path myPath;
	};

	class AssetBrowserWindow : public EditorWindow
	{
		struct DdsTextureEntry
		{
			TextureRef myDds;
			float myUnloadTimer = 0.f;

			inline static const float ourUnloadTimerDuration = 5.f;
		};

		enum class FileViewType
		{
			Thumbnails,
			ListView
		};

	public:
		AssetBrowserWindow(Editor& aEditor, const int aNumber);
		~AssetBrowserWindow();

		bool Init(ResourceReferences& aResourceReferences);

		void InitFromJson(const nlohmann::json& aJson);
		nlohmann::json ToJson() const;

		void Draw(const float aDeltaTime) override;
		void UpdateFocused(const float aDeltaTime) override;

		void GotoDirectory(const std::filesystem::path& aPath);
		void SetSelectedFile(const int aIndex/*const std::filesystem::path& aPathIncludingFilename*/);
		bool SetSelectedFileByPath(const std::filesystem::path& aPathIncludingFilename);

		std::string GetName() const override { return "Asset Thumbnails " + std::to_string(myNumber); }

		// Get the full relative path to e.g. Assets\\Shit\\Dick.money
		std::filesystem::path GetSelectedFileFullRelativePath() const;

		const std::vector<DirectoryEntry>& GetCachedAssetsFilePaths() const;

		void ScrollSelectedItemVisible();

	private:
		void DrawDirectoryPath();
		void DrawContent();
		void DrawAssetThumbnails();
		void DrawAssetListView();
		void DrawIconName(const int aPathIndex, const std::filesystem::path& aPath);

		// void CreateGameObjectFromModel(const ResourceRef<FbxResource>& aModel, GameObject* aGameobject);

		void DrawFileIcon(
			const int aPathIndex,
			const std::filesystem::path& aPath,
			const TextureRef& aTexture,
			std::function<void()> aRightClickContextMenuCallback);
		void DrawDdsIcon(const int aPathIndex, const std::filesystem::path& aPath);
		void DrawHDRIcon(const int aPathIndex, const std::filesystem::path& aPath);
		void DrawMeshMaterialIcon(const int aPathIndex, const std::filesystem::path& aPath);
		void DrawDecalMaterialIcon(const int aPathIndex, const std::filesystem::path& aPath);
		void DrawSpriteMaterialIcon(const int aPathIndex, const std::filesystem::path& aPath);
		void DrawParticleMaterialIcon(const int aPathIndex, const std::filesystem::path& aPath);
		void DrawFbxIcon(const int aPathIndex, const std::filesystem::path& aPath);
		void DrawAnimationIcon(const int aPathIndex, const std::filesystem::path& aPath);
		void DrawModelIcon(const int aPathIndex, const std::filesystem::path& aPath);
		void DrawVisualGraphIcon(const int aPathIndex, const std::filesystem::path& aPath);
		void DrawGameObjectPrefabIcon(const int aPathIndex, const std::filesystem::path& aPath);
		void DrawAnimationClipIcon(const int aPathIndex, const std::filesystem::path& aPath);
		void DrawAnimationCurveIcon(const int aPathIndex, const std::filesystem::path& aPath);
		void DrawSceneIcon(const int aPathIndex, const std::filesystem::path& aPath);
		void DrawVFXIcon(const int aPathIndex, const std::filesystem::path& aPath);
		void DrawVertexShaderIcon(const int aPathIndex, const std::filesystem::path& aPath);
		void DrawPixelShaderIcon(const int aPathIndex, const std::filesystem::path& aPath);
		void DrawGeometryShaderIcon(const int aPathIndex, const std::filesystem::path& aPath);
		void DrawUnknownFileIcon(const int aPathIndex, const std::filesystem::path& aPath);

		void HandleRightClickEmptySpace();
		void HandleGameObjectDragDropped();

		void CreateNewFileOf(
			const std::string& aNewFileNameStem,
			const std::string& aExtension,
			const std::string& aTemplatePath);

		void StartRenaming();
		void ApplyRenaming();
		void AbortRenaming();

		void RefreshCachedCurrentDirEntries();
		void AllFilesContextMenu(const std::filesystem::path& aPath);

		bool IsFileSelected(const int aPathIndex) const;
		std::filesystem::path GetSelectedFileName(const int aIndex) const;

		void DrawThumbnailSizeSlider();

		void DrawRightClickEmptyMenu();
		void DrawSearchBox();

		void StartSearching();
		void StopSearching();

		void RefreshCachedAssetsFilePaths();

		void UpdateImplicitSearchTimer(const float aDeltaTime);
		void UpdateImplicitSearch();

		void FileChangedNotificationCallback();

		bool ReimportModel(const std::filesystem::path& aPath);

	private:
		std::filesystem::path myCurrentDirectoryAbsolute;
		std::filesystem::path myCurrentDirectoryRelative;
		std::vector<DirectoryEntry> myCurrentDirEntries;
		std::vector<DirectoryEntry> myCachedAssetsFilePaths;

		// HANDLE myCurrentDirChangeNotificationHandle = nullptr;

		int mySelectedPathIndex = -1;

		TextureRef myFolderTexture;
		TextureRef myDDSTexture;
		TextureRef myHDRTexture;
		TextureRef myMaterialTexture;
		TextureRef myAnimationIconTexture;
		TextureRef myModelIconTexture;
		TextureRef myAnimationClipTexture;
		TextureRef myVisualGraphIconTexture;
		TextureRef myGameObjectPrefabIconTexture;
		TextureRef myAnimationCurveIconTexture;
		TextureRef mySceneIconTexture;
		TextureRef myVFXIconTexture;
		TextureRef myVertexShaderIconTexture;
		TextureRef myPixelShaderIconTexture;
		TextureRef myGeometryShaderIconTexture;
		TextureRef myUnknownFileTexture;

		TextureRef myXIcon;

		FileViewType myFileViewType = FileViewType::Thumbnails;

		float myThumbnailSize = 72.f;
		float myThumbnailImageSize = 64.f;

		// To avoid double popup open when right clicking in
		// the asset browser thumbnail on an icon
		bool myOpenedIconPopup = false;

		bool myIsRenamingSelectedFile = false;
		bool myHasGivenFocusToRenameInputText = false;
		char myRenameBuffer[255] = { };

		std::unordered_map<std::string, DdsTextureEntry> myDdsTextureEntries;

		bool myIsSearching = false;
		bool myShouldFocusOnSearchInput = false;
		char mySearchBuffer[255] = { };
		std::vector<DirectoryEntry> myCachedEntriesCurrentDirBackup;
		bool mySelectedSomethingDuringSearch = false;
		int myDirectoryIndexSelectedDuringSearch = -1;

		bool myHasReleasedMouseButtonAfterChangedDirectory = true;

		bool myScrollSelectedItemVisible = false;

		// Used when pressing keys randomly in the asset browser without
		// actually doing the explicit Ctrl+F search
		float myImplicitSearchTimeoutTimerSec = 0.f;
		const static inline float ourImplicitSearchTimeoutDurationSec = 0.5f;
		std::string myImplicitSearchTerm;

		int myNumber = 0;

		int myDatabaseCallbackId = 0;
	};
}
