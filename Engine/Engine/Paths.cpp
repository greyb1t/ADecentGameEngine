#include "pch.h"
#include "Paths.h"

Engine::Paths::Paths()
{
	PWSTR appDataPathBuf[MAX_PATH] = {};
	SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, 0, appDataPathBuf);

	const std::wstring appDataDir = appDataPathBuf[0];
	const std::wstring saveDirectory = appDataDir + TEXT("\\DogEngine");
	SetPath(PathEnum::EditorSaveFolder, saveDirectory);

	SetPath(PathEnum::PlaceholderTexture, "Assets/Engine/PlaceholderTextures/PlaceholderTexture.dds");
	SetPath(PathEnum::PlaceholderCubemap, "Assets/Engine/PlaceholderTextures/PlaceholderCubemap.dds");
	SetPath(PathEnum::PlaceholderAlbedoTexture, "Assets/Engine/PlaceholderTextures/PlaceholderAlbedo.dds");
	SetPath(PathEnum::PlaceholderNormalTexture, "Assets/Engine/PlaceholderTextures/PlaceholderNormal.dds");
	SetPath(PathEnum::PlaceholderMaterialTexture, "Assets/Engine/PlaceholderTextures/PlaceholderMaterial.dds");
	SetPath(PathEnum::NoTexture, "Assets/Engine/PlaceholderTextures/NoTexture.dds");
	SetPath(PathEnum::MissingCubemapTexture, "Assets/Engine/PlaceholderTextures/MissingCubemap.dds");
	SetPath(PathEnum::PlaceholderEmissiveTexture, "Assets/Engine/PlaceholderTextures/PlaceholderEmissive.dds");
	// SetPath(PathEnum::SkyboxVS, "Engine/Cubemap/SkyboxVS");
	//SetPath(PathEnum::PrefilterCubemapPS, "Engine/Cubemap/PrefilterCubemapPS");
	//SetPath(PathEnum::FullscreenVS, "Fullscreen/FullscreenVS");
}

const std::string& Engine::Paths::GetPathString(const PathEnum aPathEnum) const
{
	return myPaths[static_cast<int>(aPathEnum)].myPathString;
}

const std::wstring& Engine::Paths::GetPathWString(const PathEnum aPathEnum) const
{
	return myPaths[static_cast<int>(aPathEnum)].myPathWString;
}

const std::filesystem::path& Engine::Paths::GetPath(const PathEnum aPathEnum) const
{
	return myPaths[static_cast<int>(aPathEnum)].myPath;
}

void Engine::Paths::SetPath(const PathEnum aPathEnum, const std::filesystem::path& aPath)
{
	Path path;
	path.myPath = aPath;
	path.myPathString = aPath.string();
	path.myPathWString = aPath.wstring();

	myPaths[static_cast<int>(aPathEnum)] = path;
}
