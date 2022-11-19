#pragma once

#include <string>
#include <stdint.h>
#include <fstream>

#include "../nlohmann/json.hpp"

class JsonFile
{
public:
	JsonFile() = default;

	~JsonFile();

	template <typename T>
	bool Init(const std::string& aFileName);

	template <typename T>
	bool ReadJsonContentIntoBuffer(const std::string& aPath);

	template <typename T>
	const T* As();

private:
	uint8_t* myBuffer = nullptr;

#ifdef _DEBUG
	const std::type_info* myTypeInfo = nullptr;
#endif

	nlohmann::json myJson;
};

template <typename T>
bool JsonFile::Init(const std::string& aFileName)
{
#ifdef _DEBUG
	myTypeInfo = &typeid(T);
#endif

	// Allocate the buffer
	myBuffer = new uint8_t[sizeof(T)]{};
	new (myBuffer) T();

	if (!ReadJsonContentIntoBuffer<T>(aFileName))
	{
		return false;
	}

	return true;
}

template <typename T>
bool JsonFile::ReadJsonContentIntoBuffer(const std::string& aPath)
{
#ifdef _DEBUG
	assert(typeid(T) == *myTypeInfo && "T must be the same as when called with Start()");
#endif

	std::ifstream file(aPath);

	if (!file.is_open())
	{
		return false;
	}

	const bool ignoreComments = true;
	const bool allowExceptions = false;
	nlohmann::json j = nlohmann::json::parse(file, nullptr, allowExceptions, ignoreComments);

	// If failed to parse json file
	if (j.is_discarded())
	{
		return false;
	}

	myJson = std::move(j);

	reinterpret_cast<T*>(myBuffer)->Deserialize(myJson);

	return true;
}

template <typename T>
const T* JsonFile::As()
{
#ifdef _DEBUG
	assert(typeid(T) == *myTypeInfo && "T must be the same as when called with Start()");
#endif

	return reinterpret_cast<T*>(myBuffer);
}
