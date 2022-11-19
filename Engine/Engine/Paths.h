#pragma once

namespace Engine
{
	enum class PathEnum
	{
		// EDITOR
		EditorSaveFolder,

		// PLACEHOLDER TEXTURES
		PlaceholderTexture,
		PlaceholderCubemap,
		PlaceholderAlbedoTexture,
		PlaceholderNormalTexture,
		PlaceholderMaterialTexture,
		PlaceholderEmissiveTexture,
		NoTexture,
		MissingCubemapTexture,

		Count
	};

	class Paths
	{
	public:
		struct Path
		{
			std::string myPathString;
			std::wstring myPathWString;
			std::filesystem::path myPath;
		};

		Paths();
		Paths(const Paths&) = delete;

		const std::string& GetPathString(const PathEnum aPathEnum) const;
		const std::wstring& GetPathWString(const PathEnum aPathEnum) const;
		const std::filesystem::path& GetPath(const PathEnum aPathEnum) const;

	private:
		void SetPath(const PathEnum aPathEnum, const std::filesystem::path& aPath);

	private:
		std::array<Path, static_cast<int>(PathEnum::Count)> myPaths;
	};
}