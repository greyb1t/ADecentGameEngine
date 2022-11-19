#include "pch.h"
#include "AssetDatabase.h"
#include "Editor\FileTypes.h"

bool Engine::AssetDatabase::myIsCreated = false;
bool Engine::AssetDatabase::myIsDestroyed = false;
bool Engine::AssetDatabase::myAutoUpdateAssetCache = false;
HANDLE Engine::AssetDatabase::myCurrentDirChangeNotificationHandle = nullptr;
std::vector<Owned<Engine::DirectoryFile>> Engine::AssetDatabase::myFiles;
std::atomic_bool Engine::AssetDatabase::myRun = true;
std::atomic_int Engine::AssetDatabase::myDirectoryStatusVersion = 0;
int Engine::AssetDatabase::myDirectoryStatusVersionCached = 0;
std::thread Engine::AssetDatabase::myNotificationThread;
std::unordered_map<std::filesystem::path, Engine::DirectoryFolder*, Engine::AssetDatabase::PathHash> Engine::AssetDatabase::myHierarchy;
std::unordered_map<int, std::function<void()>> Engine::AssetDatabase::myCallbacks;
int Engine::AssetDatabase::myIdCounter = 0;
std::vector<Owned<Engine::DirectoryFolder>> Engine::AssetDatabase::myFolders;
std::filesystem::path Engine::AssetDatabase::myWatchPath;

bool Engine::AssetDatabase::Create(const std::filesystem::path& aWatchPath)
{
	assert(!myIsCreated && "already created");
	myIsCreated = true;

	myWatchPath = aWatchPath;
	myRun = true;

	RefreshWatchedDirectory();

	return true;
}

bool Engine::AssetDatabase::Destroy()
{
	assert(!myIsDestroyed && "already destroyed");
	myIsDestroyed = true;

	StopThread();

	return true;
}

const Engine::DirectoryFolder& Engine::AssetDatabase::GetDirectory(const std::filesystem::path& aPath)
{
	const auto& it = myHierarchy.find(aPath);
	assert(it != myHierarchy.end() && "DirectoryFolder does not exist");
	return *it->second;
}

const std::vector<Owned<Engine::DirectoryFolder>>& Engine::AssetDatabase::GetFolders()
{
	return myFolders;
}

const std::vector<Owned<Engine::DirectoryFile>>& Engine::AssetDatabase::GetFiles()
{
	return myFiles;
}

void Engine::AssetDatabase::UpdateDatabase()
{
	if (myDirectoryStatusVersionCached != myDirectoryStatusVersion)
	{
		myDirectoryStatusVersionCached = myDirectoryStatusVersion;

		RefreshWatchedDirectory();

		for (auto& [id, callback] : myCallbacks)
		{
			callback();
		}
	}
}

int Engine::AssetDatabase::GetCachedDirectoryVersion()
{
	return myDirectoryStatusVersionCached;
}

int Engine::AssetDatabase::GetLatestDirectoryVersion()
{
	return myDirectoryStatusVersion;
}

int Engine::AssetDatabase::RegisterChangeNotificationCallback(std::function<void()> aCallback)
{
	const int id = myIdCounter;
	myCallbacks[id] = aCallback;
	++myIdCounter;
	return id;
}

void Engine::AssetDatabase::UnregisterChangeNotificationCallback(const int aId)
{
	auto f = myCallbacks.find(aId);

	if (f != myCallbacks.end())
	{
		myCallbacks.erase(f);
	}
}

void Engine::AssetDatabase::Update()
{
	if (myAutoUpdateAssetCache)
	{
		UpdateDatabase();
	}
}

void Engine::AssetDatabase::DisableWatchAssetsNotifications()
{
	if (myCurrentDirChangeNotificationHandle != nullptr)
	{
		BOOL result = FindCloseChangeNotification(myCurrentDirChangeNotificationHandle);
		assert(result);
		myCurrentDirChangeNotificationHandle = nullptr;
	}
}

void Engine::AssetDatabase::EnableWatchAssetsNotifications()
{
	if (myCurrentDirChangeNotificationHandle != nullptr)
	{
		BOOL result = FindCloseChangeNotification(myCurrentDirChangeNotificationHandle);
		assert(result);
		myCurrentDirChangeNotificationHandle = nullptr;
	}

	myCurrentDirChangeNotificationHandle = FindFirstChangeNotification(
		myWatchPath.wstring().c_str(),
		TRUE,
		FILE_NOTIFY_CHANGE_FILE_NAME |
		FILE_NOTIFY_CHANGE_DIR_NAME |
		FILE_NOTIFY_CHANGE_ATTRIBUTES |
		FILE_NOTIFY_CHANGE_SIZE |
		FILE_NOTIFY_CHANGE_LAST_WRITE |
		FILE_NOTIFY_CHANGE_SECURITY);

	if (myCurrentDirChangeNotificationHandle == INVALID_HANDLE_VALUE)
	{
		LOG_ERROR(LogType::Resource) << "Failed to start watching the path: " <<
			myCurrentDirChangeNotificationHandle;
	}

	if (myNotificationThread.joinable())
	{
		StopThread();
	}

	myNotificationThread = std::thread(AssetDatabase::WaitNotificationThread);
}

void Engine::AssetDatabase::RefreshWatchedDirectory()
{
	myFolders.clear();
	myFiles.clear();
	myHierarchy.clear();

	// Edge case: must manually insert the path I am searcing within
	myFolders.push_back(MakeOwned<DirectoryFolder>(myWatchPath));
	myHierarchy[myWatchPath] = myFolders.back().get();

	for (const auto& dir : std::filesystem::recursive_directory_iterator(myWatchPath))
	{
		// Skip unknown files
		if (FileType::ExtensionToFileType(dir.path().extension().string()) == FileType::FileType::Unknown &&
			!dir.is_directory())
		{
			continue;
		}

		const auto& p = dir.path();

		if (dir.is_directory())
		{
			myFolders.push_back(MakeOwned<DirectoryFolder>(p));
		}
		else
		{
			myFiles.push_back(MakeOwned<DirectoryFile>(p));
		}
	}

	// Create the hierarchy
	for (const auto& folder : myFolders)
	{
		myHierarchy[folder->myPath] = folder.get();

		// l�gg till folders i folders f�r fan.
		// lita inte p� att ordning �r korrekt, s� om foldern inte finns, skapa den


		if (folder->myPath.has_parent_path())
		{
			auto f = myHierarchy.find(folder->myPath.parent_path());

			if (f != myHierarchy.end())
			{
				f->second->myFolders.push_back(folder.get());
			}
			else
			{
				assert(false && "parent path not found?");
			}
		}
	}

	for (const auto& file : myFiles)
	{
		myHierarchy[file->myPath.parent_path()]->myFiles.push_back(file.get());
	}
}

void Engine::AssetDatabase::WaitNotificationThread()
{
	while (myRun)
	{
		// Must retry wait if it did not succeed, otherwise
		// can miss notifications according to MSDN
		const DWORD wait = WaitForSingleObject(
			myCurrentDirChangeNotificationHandle, 0);

		if (wait == WAIT_OBJECT_0)
		{
			// NOTE(filip): Notify the main thread that the version of the directory has
			// changed and that we need to refresh the files
			// We dont do it in this thread to avoid complicating stuff with
			// threading, ugh
			myDirectoryStatusVersion++;

			// Restart the notification
			BOOL result = FindNextChangeNotification(myCurrentDirChangeNotificationHandle);
			assert(result);
		}
	};
}

void Engine::AssetDatabase::StopThread()
{
	// Signal the thread to stop waiting
	SetEvent(myCurrentDirChangeNotificationHandle);
	myRun = false;

	if (myNotificationThread.joinable())
	{
		myNotificationThread.join();
	}
}

void Engine::AssetDatabase::ToggleAutoUpdateAssetCache(const bool aAutoUpdateAssetCache)
{
	if (myAutoUpdateAssetCache)
	{
		DisableWatchAssetsNotifications();
	}
	else
	{
		EnableWatchAssetsNotifications();
	}

	myAutoUpdateAssetCache = aAutoUpdateAssetCache;
}

Engine::DirectoryFile::DirectoryFile(const std::filesystem::path& aPath) : myPath(aPath)
{
	myFileType = FileType::ExtensionToFileType(aPath.extension().string());
}
