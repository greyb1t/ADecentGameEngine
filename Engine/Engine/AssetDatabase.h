#pragma once

#include "Editor\FileTypes.h"

namespace Engine
{
	struct DirectoryFile
	{
		DirectoryFile(const std::filesystem::path& aPath);

		std::filesystem::path myPath;
		FileType::FileType myFileType;
	};

	struct DirectoryFolder
	{
		DirectoryFolder() = default;
		DirectoryFolder(const std::filesystem::path& aPath)
			: myPath(aPath)
		{
		}

		std::filesystem::path myPath;
		std::vector<DirectoryFile*> myFiles;
		std::vector<DirectoryFolder*> myFolders;
	};

	class AssetDatabase
	{
	public:
		static bool Create(const std::filesystem::path& aWatchPath);
		static bool Destroy();
		static void Update();

		static const DirectoryFolder& GetDirectory(const std::filesystem::path& aPath);
		static const std::vector<Owned<DirectoryFolder>>& GetFolders();
		static const std::vector<Owned<DirectoryFile>>& GetFiles();

		static void UpdateDatabase();
		static int GetCachedDirectoryVersion();
		static int GetLatestDirectoryVersion();

		// Returns ID needed to unregister it
		static int RegisterChangeNotificationCallback(std::function<void()> aCallback);
		static void UnregisterChangeNotificationCallback(const int aId);

		static void ToggleAutoUpdateAssetCache(const bool aAutoUpdateAssetCache);

	private:
		static void DisableWatchAssetsNotifications();
		static void EnableWatchAssetsNotifications();

		static void RefreshWatchedDirectory();

		static void WaitNotificationThread();
		static void StopThread();

	private:
		static bool myIsCreated;
		static bool myIsDestroyed;

		static std::filesystem::path myWatchPath;

		static bool myAutoUpdateAssetCache;
		static HANDLE myCurrentDirChangeNotificationHandle;

		static std::vector<Owned<DirectoryFolder>> myFolders;
		static std::vector<Owned<DirectoryFile>> myFiles;

		static std::atomic_bool myRun;
		static std::atomic_int myDirectoryStatusVersion;
		static int myDirectoryStatusVersionCached;

		static std::thread myNotificationThread;

		struct PathHash
		{
			std::size_t operator()(std::filesystem::path const& p) const noexcept
			{
				return std::filesystem::hash_value(p);
			}
		};

		static std::unordered_map<std::filesystem::path, DirectoryFolder*, PathHash> myHierarchy;

		static std::unordered_map<int, std::function<void()>> myCallbacks;
		static int myIdCounter;
	};
}