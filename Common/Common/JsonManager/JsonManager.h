#pragma once

#include <string>
#include <unordered_map>
#include <memory>

#include "JsonFile.h"

#include "../FileWatcher.h"

class FileWatcher;

/*
	HOW TO USE:

	// A "definition" for a json file, basically is the whole json file structure
	struct Anims
	{
		std::string myStringValue;
		int myIntValue;
		float myFloatValue;

		struct ObjectValue
		{
			std::string myStringValueInObject;
		} myObjectValue;

		void Deserialize(json& aJson)
		{
			myObjectValue.myStringValueInObject = aJson["ObjectValue"]["StringValueInObject"].get<std::string>();
			myStringValue = aJson["StringValue"];
			myIntValue = aJson["IntValue"];
			myFloatValue = aJson["FloatValue"];
		}
	};
*/

class JsonManager
{
public:
	void SetFileWatcher(FileWatcher& aFileWatcher);

	template <typename T>
	const T* DeserializeAs(
		const std::string& aPath, 
		const std::function<void(const std::string&)> aFileChangedCallback = nullptr);

private:
	std::unordered_map<std::string, std::unique_ptr<JsonFile>> myJsonFiles;
	FileWatcher* myFileWatcher = nullptr;
};

template <typename T>
const T* JsonManager::DeserializeAs(
	const std::string& aPath, 
	const std::function<void(const std::string&)> aFileChangedCallback)
{
	const auto findResult = myJsonFiles.find(aPath);
	const bool fileExists = findResult != myJsonFiles.end();

	if (fileExists)
	{
		const auto& jsonFile = findResult->second;

		return jsonFile->As<T>();
	}
	else
	{
		// Read the file and parse the json
		auto jsonFile = std::make_unique<JsonFile>();

		if (!jsonFile->Init<T>(aPath))
		{
			return nullptr;
		}

		const auto result = jsonFile->As<T>();

		myJsonFiles[aPath] = std::move(jsonFile);

		if (myFileWatcher)
		{
			myFileWatcher->WatchFile(aPath,
				[this, aFileChangedCallback](const std::string& aFilePathChanged)
				{
					//for (auto& file : myJsonFiles)
					myJsonFiles[aFilePathChanged]->ReadJsonContentIntoBuffer<T>(aFilePathChanged);

					if (aFileChangedCallback)
					{
						aFileChangedCallback(aFilePathChanged);
					}

					{
						//file.second->ReadJsonContentIntoBuffer<T>(file.first);
					}
				});
		}

		return result;
	}
}
