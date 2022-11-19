#include "pch.h"
#include "Resource.h"

Engine::Resource::Resource(const std::filesystem::path& aPath)
	: myPath(aPath)
{
}

const std::filesystem::path& Engine::Resource::GetPath() const
{
	return myPath;
}
