#include "pch.h"
#include "MakeRetailBuildWindow.h"
#include "Engine\Editor\ImGuiHelper.h"
#include "Engine\Renderer\Shaders\ShaderLoader.h"
#include "Engine\Engine.h"
#include "Engine\Renderer\GraphicsEngine.h"
#include "Engine\Renderer\Directx11Framework.h"
#include "Engine\Editor\Editor.h"
#include "Engine\Editor\FileTypes.h"
#include "AssetBrowser\AssetBrowserWindow.h"

static bool IsPowerOf2(int aN)
{
	return (aN & (aN - 1)) == 0;
}
C::Vector2ui GetTextureSize(ID3D11ShaderResourceView& aSrv)
{
	ID3D11Resource* resource = nullptr;
	aSrv.GetResource(&resource);
	ID3D11Texture2D* textureResource = reinterpret_cast<ID3D11Texture2D*>(resource);
	D3D11_TEXTURE2D_DESC textureDesc = { };
	textureResource->GetDesc(&textureDesc);

	C::Vector2ui textureSize = C::Vector2ui(textureDesc.Width, textureDesc.Height);

	textureResource->Release();

	return textureSize;
}
bool WarningAboutNotPowerOf2(ID3D11Device& aDevice)
{
	bool result = true;

	for (auto& dir : std::filesystem::recursive_directory_iterator(std::filesystem::current_path()))
	{
		Path p = dir.path().string();

		if (dir.path().has_extension())
		{
			if (p.IsExtension(".dds"))
			{
				ComPtr<ID3D11Texture2D> myTexture2D = nullptr;
				ComPtr<ID3D11ShaderResourceView> mySRV = nullptr;

				HRESULT hr = DirectX::CreateDDSTextureFromFile(
					&aDevice, p.ToWString().c_str(), nullptr, &mySRV);

				if (SUCCEEDED(hr))
				{
					const auto textureSize = GetTextureSize(*mySRV.Get());

					if (!IsPowerOf2(textureSize.x) || !IsPowerOf2(textureSize.y))
					{
						p.MakeRelativeTo(Path(std::filesystem::current_path()));

						std::cout << p << " is not power of 2 (Size: x: " << std::dec
							<< textureSize.x << ", y: " << textureSize.y << ")" << std::endl;

						result = false;
					}
				}
				else
				{
					std::cout << "Failed to open: " << p
						<< " is it bcuz of not being a proper size?, hr: " << std::hex << hr
						<< std::endl;
				}
			}
		}
	}

	return result;
}

Engine::MakeRetailBuildWindow::MakeRetailBuildWindow(Editor& aEditor)
	: EditorWindow(aEditor, Flags::CanBeClosed)
{
}

std::string Engine::MakeRetailBuildWindow::GetName() const
{
	return "Retail Build";
}

void Engine::MakeRetailBuildWindow::Draw(const float aDeltaTime)
{
	if (ImGui::Button("Check power of 2 textures"))
	{
		WarningAboutNotPowerOf2(*GetEngine().GetGraphicsEngine().GetDxFramework().GetDevice());
	}

	ImGui::SameLine();

	if (ImGui::Button("Build All Shaders In Current Directory"))
	{
		BuildAllShaders(std::filesystem::current_path().string());
	}

	ImGuiHelper::TextTitle("What does this do?");
	ImGui::Text("1. Copies current bin directory to destination");

	ImGui::BeginGroup();
	ImGui::Checkbox("##compileshaders", &myCompileShaders);
	ImGui::SameLine();
	ImGui::Text("2. Compiles the shaders in the destination directory");
	ImGui::EndGroup();

	ImGui::BeginGroup();
	ImGui::Checkbox("##cleandestination", &myCleanDestination);
	ImGui::SameLine();
	ImGui::Text("3. Cleans up the destination directory");
	ImGui::EndGroup();

	ImGui::BeginGroup();
	ImGui::Checkbox("##keepspecificscenes", &myKeepOnlySpecificScenes);
	ImGui::SameLine();
	ImGui::Text("4. Keep only specific scenes");
	ImGui::EndGroup();

	if (myKeepOnlySpecificScenes)
	{
		ImGui::BeginChild("##sceneschild", ImVec2(0.f, 200.f));
		DrawSpecificScenesTable();
		ImGui::EndChild();

		// myEditor.DrawReflectedResource();
		// 
		// if (ImGui::Button("Add Scene"))
		// {
		// 
		// }
	}

	ImGui::Dummy(ImVec2(0.f, 20.f));

	ImGuiHelper::AlignedWidget("Destination Path", 0.3f);
	ImGui::BeginGroup();
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth() - 100.f);
	ImGui::InputText("##destinationpath", &myDestinationPath.ToString(), ImGuiInputTextFlags_ReadOnly);
	ImGui::SameLine();
	if (ImGui::Button("Browse"))
	{
		std::vector<Path> folderPaths = FileIO::BrowseFileDialog({ }, L"", FileIO::FileDialogOptions_PickFolders);

		if (!folderPaths.empty())
		{
			myDestinationPath = folderPaths.front();
		}
	}
	ImGui::EndGroup();

	if (ImGui::Button("Build"))
	{
		if (myDestinationPath.ToString().empty())
		{
			LOG_ERROR(LogType::Editor) << "Please choose a directory!";
			return;
		}

		if (!CopyCurrentBinToDestination())
		{
			return;
		}

		if (myCompileShaders)
		{
			// compile
			if (!BuildAllShaders(myDestinationPath.GetRawPath()))
			{
				LOG_ERROR(LogType::Editor) << "Failed to build all shaders!";
				return;
			}
		}

		if (myCleanDestination)
		{

		}

		if (myKeepOnlySpecificScenes)
		{
			RemoveUnwantedScenes(myDestinationPath.GetRawPath());
		}
	}
}

bool Engine::MakeRetailBuildWindow::BuildAllShaders(const std::filesystem::path& aGamePath)
{
	auto shaderLoader = MakeOwned<ShaderLoader>();

	if (!shaderLoader->Init(GetEngine().GetGraphicsEngine().GetDxFramework().GetDevice(), aGamePath / "Assets\\Shaders\\"))
	{
		LOG_ERROR(LogType::Resource) << "Failed to init ShaderLoader";
		return false;
	}

	for (auto& dir : std::filesystem::recursive_directory_iterator(aGamePath / "Assets\\"))
	{
		Path path(dir.path());

		if (path.IsExtension(FileType::Extension_VertexShader) ||
			path.IsExtension(FileType::Extension_PixelShader) ||
			path.IsExtension(FileType::Extension_GeometryShader))
		{
			// Since I have no way of know if a shader is a pixel, vertex or geometry shader
			// i'll have to try compiling with all of them, yikers

			std::vector<uint8_t> bytecode;

			bool succeeded = false;

			if (path.IsExtension(FileType::Extension_VertexShader))
			{
				succeeded = shaderLoader->CompileShaderToBytecode(path, "VSMain", "vs_5_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, bytecode);
			}
			else if (path.IsExtension(FileType::Extension_PixelShader))
			{
				succeeded = shaderLoader->CompileShaderToBytecode(path, "PSMain", "ps_5_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, bytecode);
			}
			else if (path.IsExtension(FileType::Extension_GeometryShader))
			{
				succeeded = shaderLoader->CompileShaderToBytecode(path, "GSMain", "gs_5_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, bytecode);
			}

			if (succeeded)
			{
				auto destinationPath = path.ToString();
				// StringUtilities::Replace(destinationPath, "\\Runtime", "\\Compiled");

				Path newPath = destinationPath;

				newPath.ChangeExtension(".cso");

				if (!std::filesystem::exists(newPath.GetParentPath().GetRawPath()))
				{
					std::filesystem::create_directories(newPath.GetParentPath().GetRawPath());
				}

				std::ofstream file(newPath.ToWString(), std::ios::binary);
				file.write((char*)&bytecode[0], bytecode.size());
				file.close();

				LOG_INFO(LogType::Resource) << "Sucessfully compiled " << path;
			}
			else
			{
				LOG_ERROR(LogType::Resource) << "Failed to compile shader " << path;
			}
		}
	}

	return true;
}

void Engine::MakeRetailBuildWindow::DrawSpecificScenesTable()
{
	// Conditions
	static ImGuiTableFlags flags =
		ImGuiTableFlags_SizingFixedFit |
		ImGuiTableFlags_RowBg |
		ImGuiTableFlags_Borders |
		ImGuiTableFlags_Resizable |
		ImGuiTableFlags_Reorderable |
		ImGuiTableFlags_Hideable;

	std::string conditionToRemove;

	if (ImGui::BeginTable("scenes", 2, flags))
	{
		ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
		ImGui::TableSetupColumn("Scene Name", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableHeadersRow();

		for (const auto& entry : myEditor.GetAssetBrowserWindow().GetCachedAssetsFilePaths())
		{
			if (entry.myIsDirectory)
			{
				continue;
			}

			if (entry.myPath.has_extension())
			{
				std::string extension = entry.myPath.extension().string();

				for (auto& c : extension)
				{
					c = tolower(c);
				}

				if (extension == FileType::Extension_Scene)
				{
					ImGui::TableNextRow();

					for (int column = 0; column < 2; column++)
					{
						ImGui::TableSetColumnIndex(column);

						switch (column)
						{
						case 0:
						{
							ImGui::PushID(entry.myPath.string().c_str());

							const auto f = std::find_if(
								mySpecificScenesPaths.begin(),
								mySpecificScenesPaths.end(),
								[&entry](const std::filesystem::path& aPath)
								{
									return entry.myPath.parent_path() == aPath;
								});

							bool isSelected = f != mySpecificScenesPaths.end();

							if (ImGui::Checkbox("##enabled", &isSelected))
							{
								if (isSelected)
								{
									mySpecificScenesPaths.push_back(entry.myPath.parent_path());
								}
								else
								{
									mySpecificScenesPaths.erase(f);
								}
							}

							ImGui::PopID();
						} break;
						case 1:
						{
							ImGui::Text("%s", entry.myPath.string().c_str());
						} break;
						default:
							break;
						}
					}
				}
			}
		}
		ImGui::EndTable();
	}
}

bool Engine::MakeRetailBuildWindow::CopyCurrentBinToDestination()
{
	std::error_code e;

	if (std::filesystem::exists(myDestinationPath.GetRawPath()))
	{
		for (auto& dir : std::filesystem::recursive_directory_iterator(myDestinationPath.GetRawPath()))
		{
			FileIO::RemoveReadOnly(Path(dir));
		}
	}

	// Copy to destination
	std::filesystem::copy(
		std::filesystem::current_path(),
		myDestinationPath.GetRawPath(),
		std::filesystem::copy_options::recursive |
		std::filesystem::copy_options::overwrite_existing,
		e);

	if (e.value() != 0)
	{
		LOG_ERROR(LogType::Editor) << "Unable to copy the current path to the destination!";
		return false;
	}

	return true;
}

void Engine::MakeRetailBuildWindow::RemoveUnwantedScenes(const std::filesystem::path& aGamePath)
{
	const auto scenesPath = aGamePath / "Assets" / "Scenes";

	if (std::filesystem::exists(scenesPath))
	{
		for (auto& dir : std::filesystem::directory_iterator(scenesPath))
		{
			FileIO::RemoveReadOnly(Path(dir));

			auto relativePath = std::filesystem::relative(dir, aGamePath);

			auto f = std::find_if(
				mySpecificScenesPaths.begin(),
				mySpecificScenesPaths.end(),
				[&relativePath](const std::filesystem::path& aScenePath)
				{
					return relativePath == aScenePath;
				});

			if (f == mySpecificScenesPaths.end())
			{
				std::error_code e;
				std::filesystem::remove_all(dir, e);

				if (e.value() != 0)
				{
					LOG_ERROR(LogType::Editor) << "Failed to delete folder: " << dir.path();
				}
			}
		}
	}
}

