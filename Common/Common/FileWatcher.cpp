#include "FileWatcher.h"

#include <chrono>
#include <iostream>

FileWatcher::~FileWatcher()
{
	std::cout << "~FileWatcher" << std::endl;
	Stop();
	std::cout << "~FileWatcher after Stop()" << std::endl;
}

void FileWatcher::Start()
{
	myRunning = true;

	myWatchThread = std::thread(&FileWatcher::WatchThread, this);
}

void FileWatcher::Stop()
{
	myRunning = false;

	if (myWatchThread.joinable())
	{
		myWatchThread.join();
	}
}

bool FileWatcher::WatchFile(const std::string aPath, FileChanged_t aFileChangedCallback)
{
	std::unique_lock<std::mutex> lock(myFilesToWatchMutex);

	File file;
	{
		// file.myPath = aPath;
		file.myFileChangedCallbacks.push_back(aFileChangedCallback);
	}

	myFilesToWatch.insert(std::make_pair(aPath, file));

	return true;
}

void FileWatcher::FlushChangedFiles()
{
	std::unique_lock<std::mutex> lock(myChangedFilesMutex);

	// loop through files and if changed, call callback with path in the argument
	for (const auto& [path, file] : myChangedFiles)
	{
		// Notify of the change
		for (const auto& callback : file.myFileChangedCallbacks)
		{
			callback(path);
		}
	}

	myChangedFiles.clear();
}

void FileWatcher::WatchThread()
{
	while (myRunning)
	{
		// Check if files are to be added to the watch list
		{
			std::unordered_map<std::string, File> filesToWatch;

			// Make local copy to release the mutex as quickly as possible
			{
				std::unique_lock<std::mutex> lock(myFilesToWatchMutex);
				filesToWatch = myFilesToWatch;
				myFilesToWatch.clear();
			}

			for (const auto& [path, fileToWatch] : filesToWatch)
			{
				// Ensure file exist
				if (!std::filesystem::exists(path))
				{
					continue;
				}

				// Add it to watch list
				{
					std::unique_lock<std::mutex> lock(myWatchedFilesMutex);

					File file = fileToWatch;
					file.myLastWriteTime = std::filesystem::last_write_time(path);

					const bool exists = myWatchedFiles.find(path) != myWatchedFiles.end();

					// If it exists, we push back the callback to the vector
					// otherwise created new
					if (exists)
					{
						myWatchedFiles[path].myFileChangedCallbacks.push_back(
							file.myFileChangedCallbacks.front());
					}
					else
					{
						myWatchedFiles.insert(std::make_pair(path, file));
					}
				}
			}
		}

		{
			std::unique_lock<std::mutex> lock(myWatchedFilesMutex);

			for (auto& [path, file] : myWatchedFiles)
			{
				if (!std::filesystem::exists(path))
				{
					continue;
				}

				std::error_code error;

				const auto writeTime = std::filesystem::last_write_time(path, error);

				if (error)
				{
					printf("[FileWatcher] last_write_time failed: %s\n", error.message().c_str());
					continue;
				}

				if (writeTime != file.myLastWriteTime)
				{
					std::unique_lock<std::mutex> lock2(myChangedFilesMutex);

					// It was changed
					myChangedFiles.insert(std::make_pair(path, file));

					file.myLastWriteTime = writeTime;
				}
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(30));
	}
}