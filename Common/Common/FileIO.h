#pragma once

#include <string>

#include "Path.h"
#include <vector>

namespace FileIO
{
	enum FileDialogOptions : uint32_t
	{
		FileDialogOptions_None,
		FileDialogOptions_Multiselect = 1 << 0,
		FileDialogOptions_PickFolders = 1 << 1,
	};

	// Also removes the readonly permission on the file
	void WriteBinaryFile(const Path& aPath, const std::vector<uint8_t>& aBuffer);

	void RemoveReadOnly(const Path& aPath);

	// TODO: it return a path
	std::vector<Path>
		BrowseFileDialog(const std::vector<std::wstring>& aFileTypes,
			const std::wstring& aDefaultFileType,
			const FileDialogOptions aOptions = FileDialogOptions_None);

	// Generates a valid filename that does not already exist
	std::filesystem::path GenerateFirstValidFileName(
		const std::filesystem::path& aInitialFilename,
		const std::filesystem::path& aDirectory);

	void ShowFileInExplorer(const std::filesystem::path& aPath);
}
