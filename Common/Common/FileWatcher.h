#pragma once

#include <atomic>
#include <functional>
#include <thread>
#include <mutex>
#include <filesystem>

using FileChanged_t = std::function<void(const std::string& aPath)>;

class FileWatcher
{
private:
	struct File
	{
		// std::string myPath;
		std::vector<FileChanged_t> myFileChangedCallbacks;
		std::filesystem::file_time_type myLastWriteTime;
	};

public:
	virtual ~FileWatcher();

	virtual void Start();
	virtual void Stop();

	virtual bool WatchFile(const std::string aPath, FileChanged_t aFileChangedCallback);

	// Runs the callbacks, should be called on the main thread
	// this is to avoid race conditions
	virtual void FlushChangedFiles();

private:
	void WatchThread();

private:
	std::atomic_bool myRunning = false;
	std::thread myWatchThread;

	std::mutex myWatchedFilesMutex;
	std::unordered_map<std::string, File> myWatchedFiles;

	std::mutex myChangedFilesMutex;
	std::unordered_map<std::string, File> myChangedFiles;



	std::mutex myFilesToWatchMutex;
	std::unordered_map<std::string, File> myFilesToWatch;
};

// Only used to replace the normal FileWatcher when in Retail to avoid tons of null checks everywhere
// Quite an elegant solution if I do say so myself :)
class FileWatcherNull : public FileWatcher
{
public:
	void Start() override {}

	void Stop() override {}

	bool WatchFile(const std::string aPath, FileChanged_t aFileChangedCallback) override { return true; }

	void FlushChangedFiles() override {}

private:
};