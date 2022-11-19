#include "JsonManager.h"

void JsonManager::SetFileWatcher(FileWatcher& aFileWatcher)
{
	myFileWatcher = &aFileWatcher;
}
