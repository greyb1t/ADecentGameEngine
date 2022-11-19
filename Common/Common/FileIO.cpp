#include "FileIO.h"

#include <fstream>
#include <ShObjIdl_core.h>
#include <ShlObj_core.h>

void FileIO::WriteBinaryFile(const Path& aPath, const std::vector<uint8_t>& aBuffer)
{
	RemoveReadOnly(aPath);

	std::ofstream file(aPath.ToWString(), std::ios::binary);
	file.write((char*)&aBuffer[0], aBuffer.size());
	file.close();
}

void FileIO::RemoveReadOnly(const Path& aPath)
{
	// Remove the read only permission
	if (std::filesystem::exists(aPath.GetRawPath()))
	{
		std::filesystem::permissions(
			aPath.GetRawPath(), std::filesystem::perms::all, std::filesystem::perm_options::add);
	}
}

static void SetDefaultPath(IFileDialog* dialog, const std::wstring& defaultPath)
{
	IShellItem* folder = nullptr;

	HRESULT result = SHCreateItemFromParsingName(defaultPath.c_str(), NULL, IID_PPV_ARGS(&folder));

	dialog->SetDefaultFolder(folder);

	folder->Release();
}

std::vector<Path> FileIO::BrowseFileDialog(const std::vector<std::wstring>& aFileTypes,
	const std::wstring& aDefaultFileType,
	const FileDialogOptions aOptions)
{
	std::vector<Path> paths;

	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	if (SUCCEEDED(hr))
	{
		IFileOpenDialog* pFileOpen = nullptr;

		hr = CoCreateInstance(CLSID_FileOpenDialog,
			NULL,
			CLSCTX_ALL,
			IID_IFileOpenDialog,
			reinterpret_cast<void**>(&pFileOpen));

		if (SUCCEEDED(hr))
		{
			/*
			COMDLG_FILTERSPEC fileTypes[] =
			{
				{ L"DDS", L"*.dds" }
			};

			pFileOpen->SetFileTypes(ARRAYSIZE(fileTypes), fileTypes);
			pFileOpen->SetDefaultExtension(L"dds");
			*/

			std::vector<std::pair<std::wstring, std::wstring>> fileTypesAdjusted;

			fileTypesAdjusted.reserve(aFileTypes.size());

			for (int i = 0; i < aFileTypes.size(); ++i)
			{
				fileTypesAdjusted.push_back(std::make_pair(aFileTypes[i], L"*." + aFileTypes[i]));
			}

			COMDLG_FILTERSPEC fileTypes[50] = { };
			for (int i = 0; i < aFileTypes.size(); ++i)
			{
				fileTypes[i].pszName = fileTypesAdjusted[i].first.c_str();
				fileTypes[i].pszSpec = fileTypesAdjusted[i].second.c_str();
			}

			pFileOpen->SetFileTypes(aFileTypes.size(), fileTypes);
			pFileOpen->SetDefaultExtension(aDefaultFileType.c_str());

			SetDefaultPath(pFileOpen, std::filesystem::current_path());

			FILEOPENDIALOGOPTIONS opt;
			hr = pFileOpen->GetOptions(&opt);

			if ((aOptions & FileDialogOptions_Multiselect) != 0)
			{
				opt |= FOS_ALLOWMULTISELECT;
			}

			if ((aOptions & FileDialogOptions_PickFolders) != 0)
			{
				opt |= FOS_PICKFOLDERS;
			}

			hr = pFileOpen->SetOptions(opt);

			// Show the Open dialog box.
			hr = pFileOpen->Show(NULL);

			// Get the file name from the dialog box.
			if (SUCCEEDED(hr))
			{
				IShellItemArray* itemArray = nullptr;
				hr = pFileOpen->GetResults(&itemArray);

				if (SUCCEEDED(hr))
				{
					DWORD count = 0;
					itemArray->GetCount(&count);

					for (int i = 0; i < count; ++i)
					{
						IShellItem* item = nullptr;
						hr = itemArray->GetItemAt(i, &item);

						PWSTR pszFilePath;
						hr = item->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

						if (SUCCEEDED(hr))
						{
							paths.push_back(Path(std::wstring(pszFilePath)));

							CoTaskMemFree(pszFilePath);
						}

						item->Release();
					}

					itemArray->Release();
				}
			}
			pFileOpen->Release();
		}
		CoUninitialize();
	}

	return paths;
}

std::filesystem::path FileIO::GenerateFirstValidFileName(
	const std::filesystem::path& aInitialFilename,
	const std::filesystem::path& aDirectory)
{
	std::filesystem::path allowedName = aInitialFilename;

	int counter = 1;

	while (std::filesystem::exists(aDirectory / allowedName))
	{
		allowedName
			= allowedName.replace_filename(
				aInitialFilename.stem().string() +
				std::to_string(counter) +
				aInitialFilename.extension().string());
		++counter;
	}

	return allowedName;
}

void FileIO::ShowFileInExplorer(const std::filesystem::path& aPath)
{
	// probably not good to do each time, but fuck it
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	ITEMIDLIST* pidl = ILCreateFromPath(aPath.wstring().c_str());

	if (pidl)
	{
		SHOpenFolderAndSelectItems(pidl, 0, 0, 0);
		ILFree(pidl);
	}

	CoUninitialize();
}
