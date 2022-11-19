#include "pch.h"
#include "AssetBrowserWindow.h"

#include "Engine/Editor/EditorWindows/AnimationNodeEditorWindow.h"
#include "Engine/Editor/EditorWindows/VisualScriptWindow.h"
#include "Engine/Engine.h"
#include "Engine/Editor/Editor.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/MeshComponent.h"
#include "Engine/Renderer/GraphicsEngine.h"
#include "Engine/Renderer/Model/Model.h"
#include "Engine/Renderer/Model/ModelFactory.h"
#include "Engine/Renderer/ModelLoader/LoaderModel.h"
#include "Engine/Renderer/Texture/Texture2D.h"
#include "Engine/ResourceManagement/Resources/ModelResource.h"
#include "Engine/ResourceManagement/Resources/TextureResource.h"
#include "Engine/GameObject/Prefab/GameObjectPrefab.h"
#include "Engine/Animation/AnimationStateMachine.h"
#include "Engine/Animation/State/AnimationMachine.h"
#include "Engine/Editor/FileTypes.h"
#include "Engine/GraphManager/GraphInstance.h"
#include "Engine/GraphManager/VisualScript.h"
#include "Engine/ResourceManagement/Resources/AnimationCurveResource.h"
#include "Engine/ResourceManagement/Resources/AnimationStateMachineResource.h"
#include "Engine/ResourceManagement/Resources/VisualScriptResource.h"
#include "Engine/ResourceManagement/Resources/MaterialResource.h"
#include "Engine/Scene/FolderScene.h"
#include "Importing/CreationStates/ModelImportState.h"
#include "Importing/CreationStates/AnimationCreationState.h"
#include "Engine/Editor/ImGuiHelper.h"
#include "Engine/Editor/DragDropConstants.h"
#include "Engine/ResourceManagement/ResourceManager.h"
#include "Engine/AssetDatabase.h"
#include "Engine/Renderer/ModelLoader/Binary/FlatbufferModel.h"

Engine::AssetBrowserWindow::AssetBrowserWindow(Editor& aEditor, const int aNumber)
	: EditorWindow(aEditor),
	myNumber(aNumber)
{
	myDatabaseCallbackId = AssetDatabase::RegisterChangeNotificationCallback(
		[this]() { FileChangedNotificationCallback(); });
}

Engine::AssetBrowserWindow::~AssetBrowserWindow()
{
	AssetDatabase::UnregisterChangeNotificationCallback(myDatabaseCallbackId);
}

bool Engine::AssetBrowserWindow::Init(ResourceReferences& aResourceReferences)
{
	myDDSTexture = aResourceReferences.AddTexture("Assets/Engine/Editor/Icons/DdsIcon.dds");
	myHDRTexture = aResourceReferences.AddTexture("Assets/Engine/Editor/Icons/DdsIcon.dds");
	myFolderTexture = aResourceReferences.AddTexture("Assets/Engine/Editor/Icons/FolderIcon.dds");
	myUnknownFileTexture
		= aResourceReferences.AddTexture("Assets/Engine/Editor/Icons/UnknownFile.dds");
	myMaterialTexture
		= aResourceReferences.AddTexture("Assets/Engine/Editor/Icons/MaterialIcon.dds");
	myAnimationIconTexture
		= aResourceReferences.AddTexture("Assets/Engine/Editor/Icons/AnimationIcon.dds");
	mySceneIconTexture
		= aResourceReferences.AddTexture("Assets/Engine/Editor/Icons/SceneIcon.dds");
	myModelIconTexture = aResourceReferences.AddTexture("Assets/Engine/Editor/Icons/ModelIcon.dds");
	myVisualGraphIconTexture
		= aResourceReferences.AddTexture("Assets/Engine/Editor/Icons/VisualGraphIcon.dds");
	myGameObjectPrefabIconTexture
		= aResourceReferences.AddTexture("Assets/Engine/Editor/Icons/GameObjectPrefabIcon.dds");

	myAnimationClipTexture
		= aResourceReferences.AddTexture("Assets/Engine/Editor/Icons/AnimationClipIcon.dds");

	myAnimationCurveIconTexture
		= aResourceReferences.AddTexture("Assets/Engine/Editor/Icons/AnimationCurveIcon.dds");

	myVFXIconTexture
		= aResourceReferences.AddTexture("Assets/Engine/Editor/Icons/VFXIcon.dds");

	myVertexShaderIconTexture
		= aResourceReferences.AddTexture("Assets/Engine/Editor/Icons/VertexShaderIcon.dds");
	myPixelShaderIconTexture
		= aResourceReferences.AddTexture("Assets/Engine/Editor/Icons/PixelShaderIcon.dds");
	myGeometryShaderIconTexture
		= aResourceReferences.AddTexture("Assets/Engine/Editor/Icons/GeometryShaderIcon.dds");

	myXIcon
		= aResourceReferences.AddTexture("Assets/Engine/Editor/Icons/XIcon.dds");

	// Start in assets
	GotoDirectory(std::filesystem::current_path() / "Assets");

	RefreshCachedAssetsFilePaths();

	return true;
}

void Engine::AssetBrowserWindow::InitFromJson(const nlohmann::json& aJson)
{
	if (aJson.contains("FileViewType"))
	{
		myFileViewType = aJson["FileViewType"];
	}

	if (aJson.contains("ThumbnailSize"))
	{
		myThumbnailSize = aJson["ThumbnailSize"];
	}
	else
	{
		// Default to listview
		myThumbnailSize = 17.f;
	}

	if (aJson.contains("ThumbnailImageSize"))
	{
		myThumbnailImageSize = aJson["ThumbnailImageSize"];
	}
	else
	{
		// Default to listview
		myThumbnailImageSize = 17.f;
	}

	if (aJson.contains("RelativePath"))
	{
		const std::string s = aJson["RelativePath"];
		if (std::filesystem::exists(s))
		{
			GotoDirectory(std::filesystem::absolute(s));
		}
	}
}

nlohmann::json Engine::AssetBrowserWindow::ToJson() const
{
	nlohmann::json j;

	j["FileViewType"] = myFileViewType;
	j["ThumbnailSize"] = myThumbnailSize;
	j["ThumbnailImageSize"] = myThumbnailImageSize;
	j["RelativePath"] = myCurrentDirectoryRelative.string();

	return j;
}

void Engine::AssetBrowserWindow::Draw(const float aDeltaTime)
{
	ZoneNamedN(zone1, "AssetBrowserWindow::Update", true);

	UpdateImplicitSearchTimer(aDeltaTime);

	myOpenedIconPopup = false;

	DrawContent();

	std::vector<std::string> ddsToRemove;

	for (auto& entry : myDdsTextureEntries)
	{
		entry.second.myUnloadTimer -= aDeltaTime;

		if (entry.second.myUnloadTimer <= 0.f)
		{
			entry.second.myUnloadTimer = 0.f;

			// TODO: It unloads in the in the game as well
			// TODO: simply fix by abusing the ref count
			// entry.second.myDds->Unload();

			ddsToRemove.push_back(entry.first);
		}
	}

	for (const auto& toRemove : ddsToRemove)
	{
		myDdsTextureEntries.erase(toRemove);
	}

	if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
	{
		myHasReleasedMouseButtonAfterChangedDirectory = true;
	}
}

void Engine::AssetBrowserWindow::UpdateFocused(const float aDeltaTime)
{
	if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_LeftControl)))
	{
		if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F), false))
		{
			if (myIsSearching)
			{
				// TODO: Select all in the InputText when clicking Ctrl+F 
				// when already in search mode
				myShouldFocusOnSearchInput = true;
			}
			else
			{
				StartSearching();
			}
		}
	}

	if (myIsSearching)
	{
		if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape), false))
		{
			StopSearching();
		}
	}

	if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F2), false))
	{
		StartRenaming();
	}

	if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_UpArrow)))
	{
		--mySelectedPathIndex;
		mySelectedPathIndex = std::max(mySelectedPathIndex, 0);

		SetSelectedFile(mySelectedPathIndex);
		ScrollSelectedItemVisible();
	}

	if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_DownArrow)))
	{
		++mySelectedPathIndex;
		mySelectedPathIndex = std::min(mySelectedPathIndex, static_cast<int>(myCurrentDirEntries.size()) - 1);

		SetSelectedFile(mySelectedPathIndex);
		ScrollSelectedItemVisible();
	}

	if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter)))
	{
		if (myIsSearching)
		{
			StopSearching();
		}
		else
		{
			if (mySelectedPathIndex != -1)
			{
				if (myCurrentDirEntries[mySelectedPathIndex].myIsDirectory)
				{
					GotoDirectory(GetSelectedFileFullRelativePath());
				}
			}
		}
	}

	if (!myIsSearching && !myIsRenamingSelectedFile)
	{
		if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Backspace)))
		{
			if (myCurrentDirectoryRelative != "Assets")
			{
				GotoDirectory(myCurrentDirectoryRelative.parent_path());
			}
		}
	}

	UpdateImplicitSearch();

	//if (myIsSearching)
	//{
	//	if (ImGui::IsKeyPressed(static_cast<int>(CU::KeyCode::Enter)))
	//	{
	//		StopSearching();
	//		//GotoDirectory(resourcePathsInList[mySelectedResourceIndex].myPath);
	//		//SetSelectedFile(resourcePathsInList[mySelectedResourceIndex].myPath);
	//	}
	//}
}

void Engine::AssetBrowserWindow::DrawDirectoryPath()
{
	std::filesystem::path path;

	std::filesystem::path clickedOnPath = myCurrentDirectoryAbsolute;

	// NOTE(filip): std::filesystem::relative is very slow, takes 0.25ms (cache the results)
	auto relativeToCurrent = myCurrentDirectoryRelative;

	std::vector<std::filesystem::path> folders;
	for (const auto& folder : relativeToCurrent)
	{
		folders.push_back(folder);
	}

	int maxVisiblePathButtons = 4;
	const int startIndex = std::max(static_cast<int>(folders.size()) - maxVisiblePathButtons, 0);

	for (int i = 0; i < startIndex; ++i)
	{
		const auto& folder = folders[i];
		path /= folder;
	}

	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.f);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.f, 0.f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 0.f));
	ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ChildBg]);

	if (startIndex != 0)
	{
		ImGui::Text("... /");
		ImGui::SameLine();
	}

	//for (const auto& folder : relativeToCurrent)
	for (int i = startIndex; i < folders.size(); ++i)
	{
		const auto& folder = folders[i];

		path /= folder;

		if (ImGui::SmallButton(folder.filename().string().c_str()))
		{
			clickedOnPath = std::filesystem::absolute(path);
			GotoDirectory(clickedOnPath);
		}

		ImGui::SameLine();

		ImGui::Text("/");

		ImGui::SameLine();
	}

	ImGui::PopStyleVar(3);
	ImGui::PopStyleColor();

	// GotoDirectory(clickedOnPath);
	//myCurrentDirectory = clickedOnPath;

	ImGui::NewLine();
}

void Engine::AssetBrowserWindow::DrawContent()
{
	ZoneNamedN(zone1, "AssetBrowserWindow::DrawAssetThumbnails", true);

	if (myIsSearching)
	{
		DrawSearchBox();
	}
	else
	{
		DrawDirectoryPath();
	}

	ImGui::SameLine();

	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvailWidth() - 70.f);
	if (ImGui::SmallButton("Settings"))
	{
		ImGui::OpenPopup("AssetBrowserSettings");
	}

	if (ImGui::BeginPopup("AssetBrowserSettings"))
	{
		DrawThumbnailSizeSlider();

		ImGui::EndPopup();
	}

	ImGui::Separator();

	// NOTE(filip): Always listview because its better
	// myThumbnailSize = 17.f;
	// myThumbnailImageSize = 17.f;
	// myFileViewType = FileViewType::ListView;

	switch (myFileViewType)
	{
	case FileViewType::Thumbnails:
	{
		// MUST DO HERE TO MAKE DRAG DROP INTO WINDOW WORK
		if (ImGui::BeginChild("##money", ImGui::GetContentRegionAvail(), false))
		{
			// TODO: Do a search bar up here
			DrawAssetThumbnails();
		}
		ImGui::EndChild();
	} break;

	case FileViewType::ListView:
	{
		// MUST DO HERE TO MAKE DRAG DROP INTO WINDOW WORK
		if (ImGui::BeginChild("##money", ImGui::GetContentRegionAvail(), false))
		{
			DrawAssetListView();
		}
		ImGui::EndChild();
	} break;

	default:
		break;
	}

	HandleRightClickEmptySpace();

	HandleGameObjectDragDropped();
}

void Engine::AssetBrowserWindow::DrawAssetThumbnails()
{
	const float width = ImGui::GetContentRegionAvail().x;
	const float padding = 4.f;
	const float cellSize = myThumbnailSize + padding;

	int columnCount = static_cast<int>(width / cellSize);

	columnCount = std::max(1, columnCount);

	ImGui::Columns(columnCount, 0, false);

	bool gotoDirectory = false;
	std::filesystem::path directoryToGoto;

	for (int i = 0; i < myCurrentDirEntries.size(); ++i)
	{
		auto& entry = myCurrentDirEntries[i];

		const auto& entryPath = entry.myPath;
		ImGui::PushID(entryPath.string().c_str());

		if (entry.myIsDirectory)
		{
			auto onRightClickMenu = [&]()
			{};

			DrawFileIcon(i, entryPath, myFolderTexture, onRightClickMenu);

			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (myIsSearching)
				{
					StopSearching();
				}
				else
				{
					directoryToGoto = myCurrentDirectoryAbsolute.append(entryPath.stem().wstring());
					gotoDirectory = true;
					SetSelectedFile(i);
				}
			}

			//if (ImGui::BeginPopupContextItem("RightClickFolder"))
			//{
			//	myOpenedIconPopup = true;

			//	// mySelectedFilePath = entryPath;
			//	mySelectedFile = entryPath.filename();

			//	if (ImGui::Selectable("Delete folder"))
			//	{
			//	}

			//	ImGui::EndPopup();
			//}

			// ImGui::TextWrapped(entryPath.stem().string().c_str());
		}
		else
		{
			if (entryPath.has_extension())
			{
				const std::string extension
					= StringUtilities::ToLower(entryPath.extension().string());

				FileType::FileType fileType = FileType::ExtensionToFileType(extension);

				switch (fileType)
				{
				case FileType::FileType::DDS:
					DrawDdsIcon(i, entryPath);
					break;
				case FileType::FileType::HDR:
					DrawHDRIcon(i, entryPath);
					break;
				case FileType::FileType::MeshMaterial:
					DrawMeshMaterialIcon(i, entryPath);
					break;
				case FileType::FileType::SpriteMaterial:
					DrawSpriteMaterialIcon(i, entryPath);
					break;
				case FileType::FileType::DecalMaterial:
					DrawDecalMaterialIcon(i, entryPath);
					break;
				case FileType::FileType::ParticleMaterial:
					DrawParticleMaterialIcon(i, entryPath);
					break;
				case FileType::FileType::FBX:
					DrawFbxIcon(i, entryPath);
					break;
				case FileType::FileType::AnimationStateMachine:
					DrawAnimationIcon(i, entryPath);
					break;
				case FileType::FileType::Model:
					DrawModelIcon(i, entryPath);
					break;
				case FileType::FileType::VisualScript:
					DrawVisualGraphIcon(i, entryPath);
					break;
				case FileType::FileType::PrefabGameObject:
					DrawGameObjectPrefabIcon(i, entryPath);
					break;
				case FileType::FileType::AnimationClip:
					DrawAnimationClipIcon(i, entryPath);
					break;
				case FileType::FileType::AnimationCurve:
					DrawAnimationCurveIcon(i, entryPath);
					break;
				case FileType::FileType::Scene:
					DrawSceneIcon(i, entryPath);
					break;
				case FileType::FileType::VFX:
					DrawVFXIcon(i, entryPath);
					break;
				case FileType::FileType::VertexShader:
					DrawVertexShaderIcon(i, entryPath);
					break;
				case FileType::FileType::PixelShader:
					DrawPixelShaderIcon(i, entryPath);
					break;
				case FileType::FileType::GeometryShader:
					DrawGeometryShaderIcon(i, entryPath);
					break;
				case FileType::FileType::Unknown:
					// DrawUnknownFileIcon(entryPath);
					ImGui::PopID();
					continue;
					break;

				default:
					LOG_WARNING(LogType::Editor) << "Missing filetype case";
					break;
				}

				//// Select the icon we right clicked
				//if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
				//{
				//	mySelectedFile = entryPath.filename();
				//}

				// DrawIconName(entryPath);
			}
		}

		ImGui::NextColumn();

		ImGui::PopID();
	}

	if (gotoDirectory)
	{
		GotoDirectory(directoryToGoto);
		myHasReleasedMouseButtonAfterChangedDirectory = false;
	}
}

void Engine::AssetBrowserWindow::DrawAssetListView()
{
	const float width = ImGui::GetContentRegionAvail().x;
	const float padding = 4.f;
	const float cellSize = /*myThumbnailSize + padding*/width;

	int columnCount = static_cast<int>(width / cellSize);

	columnCount = std::max(1, columnCount);

	ImGui::Columns(columnCount, 0, false);

	bool gotoDirectory = false;
	std::filesystem::path directoryToGoto;

	for (int i = 0; i < myCurrentDirEntries.size(); ++i)
	{
		auto& entry = myCurrentDirEntries[i];
		const auto& entryPath = entry.myPath;
		ImGui::PushID(entryPath.string().c_str());

		if (entry.myIsDirectory)
		{
			auto onRightClickMenu = [&]()
			{};

			DrawFileIcon(i, entryPath, myFolderTexture, onRightClickMenu);

			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (myIsSearching)
				{
					StopSearching();
				}
				else
				{
					directoryToGoto = myCurrentDirectoryAbsolute.append(entryPath.stem().wstring());
					gotoDirectory = true;
					SetSelectedFile(i);
				}
			}
		}
		else
		{
			if (entryPath.has_extension())
			{
				const std::string extension
					= StringUtilities::ToLower(entryPath.extension().string());

				FileType::FileType fileType = FileType::ExtensionToFileType(extension);

				switch (fileType)
				{
				case FileType::FileType::DDS:
					DrawDdsIcon(i, entryPath);
					break;
				case FileType::FileType::HDR:
					DrawHDRIcon(i, entryPath);
					break;
				case FileType::FileType::MeshMaterial:
					DrawMeshMaterialIcon(i, entryPath);
					break;
				case FileType::FileType::DecalMaterial:
					DrawDecalMaterialIcon(i, entryPath);
					break;
				case FileType::FileType::SpriteMaterial:
					DrawSpriteMaterialIcon(i, entryPath);
					break;
				case FileType::FileType::ParticleMaterial:
					DrawParticleMaterialIcon(i, entryPath);
					break;
				case FileType::FileType::FBX:
					DrawFbxIcon(i, entryPath);
					break;
				case FileType::FileType::AnimationStateMachine:
					DrawAnimationIcon(i, entryPath);
					break;
				case FileType::FileType::Model:
					DrawModelIcon(i, entryPath);
					break;
				case FileType::FileType::VisualScript:
					DrawVisualGraphIcon(i, entryPath);
					break;
				case FileType::FileType::PrefabGameObject:
					DrawGameObjectPrefabIcon(i, entryPath);
					break;
				case FileType::FileType::AnimationClip:
					DrawAnimationClipIcon(i, entryPath);
					break;
				case FileType::FileType::AnimationCurve:
					DrawAnimationCurveIcon(i, entryPath);
					break;
				case FileType::FileType::Scene:
					DrawSceneIcon(i, entryPath);
					break;
				case FileType::FileType::VFX:
					DrawVFXIcon(i, entryPath);
					break;
				case FileType::FileType::VertexShader:
					DrawVertexShaderIcon(i, entryPath);
					break;
				case FileType::FileType::PixelShader:
					DrawPixelShaderIcon(i, entryPath);
					break;
				case FileType::FileType::GeometryShader:
					DrawGeometryShaderIcon(i, entryPath);
					break;
				case FileType::FileType::Unknown:
					// DrawUnknownFileIcon(entryPath);
					ImGui::PopID();
					continue;
					break;
				default:
					LOG_WARNING(LogType::Editor) << "Missing filetype case";
					break;
				}
			}
		}

		ImGui::NextColumn();

		ImGui::PopID();
	}

	if (gotoDirectory)
	{
		GotoDirectory(directoryToGoto);
		myHasReleasedMouseButtonAfterChangedDirectory = false;
	}
}

void Engine::AssetBrowserWindow::DrawIconName(const int aPathIndex, const std::filesystem::path& aPath)
{
	const bool isSelected = IsFileSelected(aPathIndex);

	if (isSelected && myIsRenamingSelectedFile)
	{
		if (!myHasGivenFocusToRenameInputText)
		{
			myHasGivenFocusToRenameInputText = true;
			ImGui::SetKeyboardFocusHere();
		}

		if (ImGui::InputText("##rename",
			myRenameBuffer,
			sizeof(myRenameBuffer),
			ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
		{
			ApplyRenaming();
		}

		if (ImGui::IsItemDeactivated())
		{
			ApplyRenaming();
		}
	}
	else
	{
		std::string s = aPath.filename().string();

		if (myFileViewType == FileViewType::Thumbnails)
		{
			int wordLength = 0;

			for (char& c : s)
			{
				++wordLength;

				if (c == ' ' && wordLength > 9)
				{
					c = '\n';
					wordLength = 0;
				}
				else if (wordLength > 9)
				{
					// Cut off the word here 3 dots
					s = s.substr(0, wordLength - 1) + "...";
					break;
				}
			}

			ImGuiHelper::TextCentered(s.c_str());
		}
		else if (myFileViewType == FileViewType::ListView)
		{
			ImGui::Text(s.c_str());
		}
	}
}

//void CreateGameObjectFromModelInternal(const Engine::ResourceRef<Engine::FbxResource>& aModel,
//	GameObject* aGameobject,
//	const Renderer::Fbx::Node* aNode)
//{
//	auto g = aGameobject->GetScene()->AddGameObject<GameObject>();
//	g->SetName(aNode->myName);
//
//	auto& t = g->GetTransform();
//
//	t.SetParent(&aGameobject->GetTransform());
//	t.SetScaleLocal(aNode->myTransform.GetScaleLocal());
//	t.SetPositionLocal(aNode->myTransform.GetPositionLocal());
//	t.SetRotationLocal(aNode->myTransform.GetRotationLocal());
//
//	//assert(false && "I changed myMeshIndex to myMeshIndices, now node can have multiple meshes, fix");
//
//	if (aNode->myMeshIndices.size() > 1)
//	{
//		LOG_WARNING(LogType::Engine) << "Multiple meshes in same not not handled currently, todo handle by simply creating another gameobject for that mesh with same transform";
//	}
//
//	if (!aNode->myMeshIndices.empty())
//	{
//		auto meshComponent = g->AddComponent<Engine::MeshComponent>();
//		meshComponent->SetModel(aModel->GetPath());
//		meshComponent->SetMeshIndex(aNode->myMeshIndices.front());
//		meshComponent->SetMaterial("");
//	}
//
//	for (const int childIndex : aNode->myChildrenIndices)
//	{
//		CreateGameObjectFromModelInternal(aModel, g, &aModel->Get().GetNodes()[childIndex]);
//	}
//}

//void Engine::AssetBrowserWindow::CreateGameObjectFromModel(
//	const ResourceRef<FbxResource>& aModel, GameObject* aGameobject)
//{
//	CreateGameObjectFromModelInternal(aModel, aGameobject, &aModel->Get().GetNodes()[0]);
//
//	//std::queue<std::pair<const Renderer::Fbx::Node*, GameObject*>> nodeStack;
//
//	//const auto& nodes = aModel->Get().GetNodes();
//
//	//nodeStack.push(std::make_pair(&nodes.front(), aGameobject));
//
//	//while (!nodeStack.empty())
//	//{
//	//	const auto& [node, parentGameObject] = nodeStack.front();
//	//	nodeStack.pop();
//
//	//	auto g = parentGameObject->GetScene()->AddGameObject<GameObject>();
//	//	g->SetName(node->myName);
//
//	//	auto& t = g->GetTransform();
//
//	//	t.SetParent(&parentGameObject->GetTransform());
//	//	t.SetScaleLocal(node->myTransform.GetScaleLocal());
//	//	t.SetPositionLocal(node->myTransform.GetPositionLocal());
//	//	t.SetRotationLocal(node->myTransform.GetRotationLocal());
//
//	//	for (const int childIndex : node->myChildrenIndices)
//	//	{
//	//		nodeStack.push(std::make_pair(&nodes[childIndex], g));
//	//	}
//	//}
//}

void Engine::AssetBrowserWindow::DrawFileIcon(
	const int aPathIndex,
	const std::filesystem::path& aPath,
	const TextureRef& aTexture,
	std::function<void()> aRightClickContextMenuCallback)
{
	const auto startPos = ImGui::GetCursorPos();

	ImGui::GetWindowDrawList()->ChannelsSplit(2);
	ImGui::GetWindowDrawList()->ChannelsSetCurrent(1);

	ImGui::BeginGroup();

	if (myFileViewType == FileViewType::Thumbnails)
	{
		float delta = (myThumbnailSize - myThumbnailImageSize) * 0.5f;
		ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPos().x + delta, ImGui::GetCursorPos().y + delta));
		ImGui::Image(aTexture->Get().GetSRV(), ImVec2(myThumbnailImageSize, myThumbnailImageSize));
	}
	else if (myFileViewType == FileViewType::ListView)
	{
		//float delta = (myThumbnailImageSize) * 0.25f;
		float delta = 0.f;

		ImGui::SetCursorPos(ImVec2(
			ImGui::GetCursorPos().x,
			ImGui::GetCursorPos().y + delta));

		ImGui::Image(aTexture->Get().GetSRV(), ImVec2(myThumbnailImageSize, myThumbnailImageSize));
	}

	// Must have invisible button to make drag drop work
	ImGui::SetCursorPos(startPos);
	ImGui::InvisibleButton(aPath.string().c_str(), ImVec2(myThumbnailSize, myThumbnailSize));

	ImGui::SetCursorPos(startPos);
	ImGui::Dummy(ImVec2(myThumbnailSize, myThumbnailSize));

	if (myFileViewType == FileViewType::Thumbnails)
	{
		DrawIconName(aPathIndex, aPath);
	}
	else
	{
		std::string fileTypeString = "";

		if (aPath.has_extension())
		{
			fileTypeString = FileType::FileTypeToString(FileType::ExtensionToFileType(aPath.extension().string()));
		}
		else
		{
			fileTypeString = "Folder";
		}

		ImGui::SameLine();
		DrawIconName(aPathIndex, aPath);
		ImGui::SameLine();
		ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvailWidth() - 100, 0.f));
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.f),
			fileTypeString.c_str());
		ImGui::SameLine();
		ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvailWidth(), 0.f));
	}

	ImGui::EndGroup();

	ImGui::GetWindowDrawList()->ChannelsSetCurrent(0);

	const auto min = ImGui::GetItemRectMin();
	const auto max = ImGui::GetItemRectMax();

	const bool isSelected = IsFileSelected(aPathIndex);

	ImDrawList* drawList = ImGui::GetWindowDrawList();

	if (ImGui::IsItemHovered() && !myIsRenamingSelectedFile)
	{
		if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
		{
			const auto clr = ImGui::ColorConvertFloat4ToU32(ImVec4(1.f, 1.f, 1.f, 0.3f));
			drawList->AddRectFilled(min, max, clr);
		}
		else
		{
			const auto clr = ImGui::ColorConvertFloat4ToU32(ImVec4(1.f, 1.f, 1.f, 0.4f));
			drawList->AddRectFilled(min, max, clr);
		}
	}
	else if (isSelected)
	{
		const auto clr = ImGui::ColorConvertFloat4ToU32(ImVec4(1.f, 1.f, 1.f, 0.2f));
		drawList->AddRectFilled(min, max, clr);

		if (myScrollSelectedItemVisible)
		{
			myScrollSelectedItemVisible = false;

			ImGui::SetScrollHereY();
		}
	}

	if (ImGui::IsWindowFocused() && ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
	{
		// Ensure that user has released the mouse left button after having clicked
		// on a directory to go into it
		// otherwise we'd select a file immediately the same frame
		if (myHasReleasedMouseButtonAfterChangedDirectory)
		{
			SetSelectedFile(aPathIndex);
		}
	}

	if (ImGui::IsItemHovered())
	{
		if (aPath.has_extension() && aPath.extension() == FileType::Extension_DDS)
		{
			ImGui::BeginTooltip();
			ImGui::Image(aTexture->Get().GetSRV(), ImVec2(64, 64));
			ImGui::EndTooltip();
		}
		else
		{
			ImGui::SetTooltip(aPath.string().c_str());
		}
	}

	if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
	{
		SetSelectedFile(aPathIndex);
	}

	if (ImGui::BeginPopupContextItem("FbxContextMenu"))
	{
		myOpenedIconPopup = true;

		AllFilesContextMenu(aPath);

		ImGui::Separator();

		if (aRightClickContextMenuCallback)
		{
			aRightClickContextMenuCallback();
		}

		ImGui::EndPopup();
	}

	ImGui::GetWindowDrawList()->ChannelsMerge();
}

void Engine::AssetBrowserWindow::DrawDdsIcon(const int aPathIndex, const std::filesystem::path& aPath)
{
	auto onRightClickMenu = [&]()
	{};

	auto ddsTexture = GResourceManager->CreateRef<TextureResource>(aPath.string());

	// If we're searching, we dont wanna do this, all textures will be loaded
	if (!myIsSearching)
	{
		ddsTexture->RequestLoading();
	}

	const auto ddsFindResult = myDdsTextureEntries.find(aPath.string());

	if (ddsFindResult != myDdsTextureEntries.end())
	{
		// If already loaded the dds, reset its timer
		ddsFindResult->second.myUnloadTimer = DdsTextureEntry::ourUnloadTimerDuration;
	}
	else
	{
		// we are not loading it when searhing, therefore no point doing this
		if (!myIsSearching)
		{
			DdsTextureEntry ddsEntry;
			ddsEntry.myDds = ddsTexture;
			ddsEntry.myUnloadTimer = DdsTextureEntry::ourUnloadTimerDuration;
			myDdsTextureEntries.insert(std::make_pair(aPath.string(), ddsEntry));
		}
	}

	TextureRef iconTexture = myDDSTexture;

	if (ddsTexture->IsValid())
	{
		iconTexture = ddsTexture;
	}

	DrawFileIcon(aPathIndex, aPath, iconTexture, onRightClickMenu);

	// We need this to check if it has already started a payload, otherwise it would do other items in the tree
	// NOTE: Having this means we cannot render stuff when dragging
	//if (!ImGui::GetDragDropPayload())
	{

		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
		{
			Path p(aPath);
			p.MakeRelativeTo(Path(std::filesystem::current_path()));

			ImGui::SetDragDropPayload(DragDropConstants::Texture.c_str(), p.ToString().c_str(), p.ToString().size(), 0);

			ImGui::Text(aPath.string().c_str());

			if (iconTexture->IsValid())
			{
				ImGui::Image(iconTexture->Get().GetSRV(), ImVec2(64, 64));
			}

			ImGui::EndDragDropSource();
		}
	}
}

void Engine::AssetBrowserWindow::DrawHDRIcon(const int aPathIndex, const std::filesystem::path& aPath)
{
	auto onRightClickMenu = [&]()
	{};

	auto hdrTexture = GResourceManager->CreateRef<TextureResource>(aPath.string());

	// If we're searching, we dont wanna do this, all textures will be loaded
	if (!myIsSearching)
	{
		hdrTexture->RequestLoading();
	}

	DrawFileIcon(aPathIndex, aPath, myHDRTexture, onRightClickMenu);

	// We need this to check if it has already started a payload, otherwise it would do other items in the tree
	// NOTE: Having this means we cannot render stuff when dragging
	//if (!ImGui::GetDragDropPayload())
	{

		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
		{
			Path p(aPath);
			p.MakeRelativeTo(Path(std::filesystem::current_path()));

			ImGui::SetDragDropPayload(DragDropConstants::HDRTexture.c_str(), p.ToString().c_str(), p.ToString().size(), 0);

			ImGui::Text(aPath.string().c_str());

			ImGui::EndDragDropSource();
		}
	}
}

void Engine::AssetBrowserWindow::DrawMeshMaterialIcon(const int aPathIndex, const std::filesystem::path& aPath)
{
	auto onRightClickMenu = [&]()
	{};

	DrawFileIcon(aPathIndex, aPath, myMaterialTexture, onRightClickMenu);

	if (ImGui::IsItemHovered() &&
		ImGui::IsWindowFocused() &&
		ImGui::IsMouseReleased(ImGuiMouseButton_Left) &&
		myHasReleasedMouseButtonAfterChangedDirectory)
	{
		myEditor.ShowMaterialInInspector(GetSelectedFileFullRelativePath());
	}

	// if (!ImGui::GetDragDropPayload())
	{
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
		{
			Path p(aPath);
			p.MakeRelativeTo(Path(std::filesystem::current_path()));

			ImGui::SetDragDropPayload(DragDropConstants::MeshMaterial.c_str(), p.ToString().c_str(), p.ToString().size(), 0);

			ImGui::Text(aPath.string().c_str());

			if (myMaterialTexture->IsValid())
			{
				ImGui::Image(myMaterialTexture->Get().GetSRV(), ImVec2(32, 32));
			}

			ImGui::EndDragDropSource();
		}
	}
}

void Engine::AssetBrowserWindow::DrawDecalMaterialIcon(const int aPathIndex, const std::filesystem::path& aPath)
{
	auto onRightClickMenu = [&]()
	{};

	DrawFileIcon(aPathIndex, aPath, myMaterialTexture, onRightClickMenu);

	if (ImGui::IsItemHovered() &&
		ImGui::IsWindowFocused() &&
		ImGui::IsMouseReleased(ImGuiMouseButton_Left) &&
		myHasReleasedMouseButtonAfterChangedDirectory)
	{
		myEditor.ShowMaterialInInspector(GetSelectedFileFullRelativePath());
	}

	// if (!ImGui::GetDragDropPayload())
	{
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
		{
			Path p(aPath);
			p.MakeRelativeTo(Path(std::filesystem::current_path()));

			ImGui::SetDragDropPayload(DragDropConstants::DecalMaterial.c_str(), p.ToString().c_str(), p.ToString().size(), 0);

			ImGui::Text(aPath.string().c_str());

			if (myMaterialTexture->IsValid())
			{
				ImGui::Image(myMaterialTexture->Get().GetSRV(), ImVec2(32, 32));
			}

			ImGui::EndDragDropSource();
		}
	}
}

void Engine::AssetBrowserWindow::DrawSpriteMaterialIcon(const int aPathIndex, const std::filesystem::path& aPath)
{
	auto onRightClickMenu = [&]()
	{};

	DrawFileIcon(aPathIndex, aPath, myMaterialTexture, onRightClickMenu);

	if (ImGui::IsItemHovered() &&
		ImGui::IsWindowFocused() &&
		ImGui::IsMouseReleased(ImGuiMouseButton_Left) &&
		myHasReleasedMouseButtonAfterChangedDirectory)
	{
		myEditor.ShowMaterialInInspector(GetSelectedFileFullRelativePath());
	}

	// if (!ImGui::GetDragDropPayload())
	{
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
		{
			Path p(aPath);
			p.MakeRelativeTo(Path(std::filesystem::current_path()));

			ImGui::SetDragDropPayload(DragDropConstants::SpriteMaterial.c_str(), p.ToString().c_str(), p.ToString().size(), 0);

			ImGui::Text(aPath.string().c_str());

			if (myMaterialTexture->IsValid())
			{
				ImGui::Image(myMaterialTexture->Get().GetSRV(), ImVec2(32, 32));
			}

			ImGui::EndDragDropSource();
		}
	}
}

void Engine::AssetBrowserWindow::DrawParticleMaterialIcon(const int aPathIndex, const std::filesystem::path& aPath)
{
	auto onRightClickMenu = [&]()
	{};

	DrawFileIcon(aPathIndex, aPath, myMaterialTexture, onRightClickMenu);

	if (ImGui::IsItemHovered() &&
		ImGui::IsWindowFocused() &&
		ImGui::IsMouseReleased(ImGuiMouseButton_Left) &&
		myHasReleasedMouseButtonAfterChangedDirectory)
	{
		myEditor.ShowMaterialInInspector(GetSelectedFileFullRelativePath());
	}

	// if (!ImGui::GetDragDropPayload())
	{
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
		{
			Path p(aPath);
			p.MakeRelativeTo(Path(std::filesystem::current_path()));

			ImGui::SetDragDropPayload(DragDropConstants::ParticleMaterial.c_str(), p.ToString().c_str(), p.ToString().size(), 0);

			ImGui::Text(aPath.string().c_str());

			if (myMaterialTexture->IsValid())
			{
				ImGui::Image(myMaterialTexture->Get().GetSRV(), ImVec2(32, 32));
			}

			ImGui::EndDragDropSource();
		}
	}
}

void Engine::AssetBrowserWindow::DrawFbxIcon(const int aPathIndex, const std::filesystem::path& aPath)
{
	auto onRightClickMenu = [&]()
	{
		if (ImGui::BeginMenu("Import as"))
		{
			if (ImGui::MenuItem("Model"))
			{
				myEditor.OpenImportWindow(ResourceImportState::Model, aPath);
			}

			if (ImGui::MenuItem("Animation clip"))
			{
				myEditor.OpenImportWindow(ResourceImportState::AnimationClip, aPath);
			}

			ImGui::EndMenu();
		}
	};

	DrawFileIcon(aPathIndex, aPath, myUnknownFileTexture, onRightClickMenu);

	/*if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
	{
		ImGui::OpenPopup("FbxContextMenu");
	}

	if (ImGui::BeginPopupContextItem("FbxContextMenu"))
	{
		myOpenedIconPopup = true;

		if (ImGui::BeginMenu("Import as"))
		{
			if (ImGui::MenuItem("Model"))
			{
				myEditor.OpenImportWindow(ResourceImportState::Model, aPath);
			}

			if (ImGui::MenuItem("Animation clip"))
			{
				myEditor.OpenImportWindow(ResourceImportState::AnimationClip, aPath);
			}

			ImGui::EndMenu();
		}

		AllFilesContextMenu(aPath);

		ImGui::EndPopup();
	}*/
}

void Engine::AssetBrowserWindow::DrawAnimationIcon(const int aPathIndex, const std::filesystem::path& aPath)
{
	auto onRightClickMenu = [&]()
	{};

	DrawFileIcon(aPathIndex, aPath, myAnimationIconTexture, onRightClickMenu);

	if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
	{
		// Here we creaet a new instance and modify that
		// means we cannot see results live
		// we can also double click an AnimatorComponent to open to see live flow

		myEditor.OpenAnimationEditorNewInstanceOfMachine(Path(aPath));
	}

	// if (!ImGui::GetDragDropPayload())
	{
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
		{
			Path p(aPath);
			p.MakeRelativeTo(Path(std::filesystem::current_path()));

			ImGui::SetDragDropPayload(DragDropConstants::AnimationStateMachine.c_str(), p.ToString().c_str(), p.ToString().size(), 0);

			ImGui::Text(aPath.string().c_str());

			if (myAnimationIconTexture->IsValid())
			{
				ImGui::Image(myAnimationIconTexture->Get().GetSRV(), ImVec2(32, 32));
			}

			ImGui::EndDragDropSource();
		}
	}
}

void Engine::AssetBrowserWindow::DrawModelIcon(const int aPathIndex, const std::filesystem::path& aPath)
{
	auto onRightClickMenu = [&]()
	{
		if (ImGui::MenuItem("Reimport"))
		{
			if (ReimportModel(aPath))
			{
				LOG_INFO(LogType::Editor) << "Successfully reimported model";
			}
			else
			{
				LOG_ERROR(LogType::Editor) << "Failed to reimport model";
			}
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("Re-imports the model from its FBX with the same materials");
		}
	};

	DrawFileIcon(aPathIndex, aPath, myModelIconTexture, onRightClickMenu);

	if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
	{
		// assert(false);

		// auto animCurveResource = GetEngine().GetResourceManager().
		// 	CreateRef<AnimationCurveResource>(aPath.string());
		// 
		// animCurveResource->Load();
		// myEditor.OpenCurveEditor(animCurveResource);
	}

	// if (!ImGui::GetDragDropPayload())
	{
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
		{
			Path p(aPath);
			p.MakeRelativeTo(Path(std::filesystem::current_path()));

			ImGui::SetDragDropPayload(
				DragDropConstants::Model.c_str(),
				p.ToString().c_str(),
				p.ToString().size(),
				0);

			ImGui::Text(aPath.string().c_str());

			if (myModelIconTexture->IsValid())
			{
				ImGui::Image(myModelIconTexture->Get().GetSRV(), ImVec2(32, 32));
			}

			ImGui::EndDragDropSource();
		}
	}
}

void Engine::AssetBrowserWindow::DrawVisualGraphIcon(const int aPathIndex, const std::filesystem::path& aPath)
{
	auto onRightClickMenu = [&]()
	{};

	DrawFileIcon(aPathIndex, aPath, myVisualGraphIconTexture, onRightClickMenu);

	if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
	{
		myEditor.OpenVisualGraphNodeEditorNewInstance(Path(aPath));
	}

	// if (!ImGui::GetDragDropPayload())
	{
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
		{
			Path p(aPath);
			p.MakeRelativeTo(Path(std::filesystem::current_path()));

			ImGui::SetDragDropPayload(DragDropConstants::VisualScript.c_str(), p.ToString().c_str(), p.ToString().size(), 0);

			ImGui::Text(aPath.string().c_str());

			if (myVisualGraphIconTexture->IsValid())
			{
				ImGui::Image(myVisualGraphIconTexture->Get().GetSRV(), ImVec2(32, 32));
			}

			ImGui::EndDragDropSource();
		}
	}
}

void Engine::AssetBrowserWindow::DrawGameObjectPrefabIcon(const int aPathIndex, const std::filesystem::path& aPath)
{
	auto onRightClickMenu = [&]()
	{};

	DrawFileIcon(aPathIndex, aPath, myGameObjectPrefabIconTexture, onRightClickMenu);

	if (ImGui::IsItemHovered() &&
		ImGui::IsWindowFocused() &&
		ImGui::IsMouseReleased(ImGuiMouseButton_Left) &&
		myHasReleasedMouseButtonAfterChangedDirectory)
	{
		// Show in Scene Graph
		// Show in Inspector
		myEditor.ShowGameObjectPrefabInInspector(aPath);
	}

	// if (!ImGui::GetDragDropPayload())
	{
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
		{
			Path p(aPath);
			p.MakeRelativeTo(Path(std::filesystem::current_path()));

			ImGui::SetDragDropPayload(
				DragDropConstants::GameObjectPrefab.c_str(), p.ToString().c_str(), p.ToString().size(), 0);

			ImGui::Text(aPath.string().c_str());

			if (myGameObjectPrefabIconTexture->IsValid())
			{
				ImGui::Image(myGameObjectPrefabIconTexture->Get().GetSRV(), ImVec2(32, 32));
			}

			ImGui::EndDragDropSource();
		}
	}
}

void Engine::AssetBrowserWindow::DrawAnimationClipIcon(const int aPathIndex, const std::filesystem::path& aPath)
{
	auto onRightClickMenu = [&]()
	{};

	DrawFileIcon(aPathIndex, aPath, myAnimationClipTexture, onRightClickMenu);

	//if (!ImGui::GetDragDropPayload())
	{
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
		{
			Path p(aPath);
			p.MakeRelativeTo(Path(std::filesystem::current_path()));

			ImGui::SetDragDropPayload(DragDropConstants::AnimationClip.c_str(), p.ToString().c_str(), p.ToString().size(), 0);

			ImGui::Text(aPath.string().c_str());

			if (myAnimationClipTexture->IsValid())
			{
				ImGui::Image(myAnimationClipTexture->Get().GetSRV(), ImVec2(32, 32));
			}

			ImGui::EndDragDropSource();
		}
	}
}

void Engine::AssetBrowserWindow::DrawAnimationCurveIcon(const int aPathIndex, const std::filesystem::path& aPath)
{
	auto onRightClickMenu = [&]()
	{};

	DrawFileIcon(aPathIndex, aPath, myAnimationCurveIconTexture, onRightClickMenu);

	if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
	{
		// Open in curve editor

		auto animCurveResource = GetEngine().GetResourceManager().
			CreateRef<AnimationCurveResource>(aPath.string());

		animCurveResource->Load();
		myEditor.OpenCurveEditor(animCurveResource);
	}

	// if (!ImGui::GetDragDropPayload())
	{
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
		{
			Path p(aPath);
			p.MakeRelativeTo(Path(std::filesystem::current_path()));

			ImGui::SetDragDropPayload(DragDropConstants::AnimationCurve.c_str(), p.ToString().c_str(), p.ToString().size(), 0);

			ImGui::Text(aPath.string().c_str());

			if (myAnimationCurveIconTexture->IsValid())
			{
				ImGui::Image(myAnimationCurveIconTexture->Get().GetSRV(), ImVec2(32, 32));
			}

			ImGui::EndDragDropSource();
		}
	}
}

void Engine::AssetBrowserWindow::DrawSceneIcon(const int aPathIndex, const std::filesystem::path& aPath)
{
	auto onRightClickMenu = [&]()
	{};

	DrawFileIcon(aPathIndex, aPath, mySceneIconTexture, onRightClickMenu);

	if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
	{
		// open in editor
		myEditor.OpenScene(Path(aPath));
	}
}

void Engine::AssetBrowserWindow::DrawVFXIcon(const int aPathIndex, const std::filesystem::path& aPath)
{
	auto onRightClickMenu = [&]()
	{};

	DrawFileIcon(aPathIndex, aPath, myVFXIconTexture, onRightClickMenu);

	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
	{
		Path p(aPath);
		p.MakeRelativeTo(Path(std::filesystem::current_path()));

		ImGui::SetDragDropPayload(DragDropConstants::VFX.c_str(), p.ToString().c_str(), p.ToString().size(), 0);

		ImGui::Text(aPath.string().c_str());

		if (myVFXIconTexture->IsValid())
		{
			ImGui::Image(myVFXIconTexture->Get().GetSRV(), ImVec2(32, 32));
		}

		ImGui::EndDragDropSource();
	}
}

void Engine::AssetBrowserWindow::DrawVertexShaderIcon(const int aPathIndex, const std::filesystem::path& aPath)
{
	auto onRightClickMenu = [&]()
	{};

	DrawFileIcon(aPathIndex, aPath, myVertexShaderIconTexture, onRightClickMenu);

	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
	{
		Path p(aPath);
		p.MakeRelativeTo(Path(std::filesystem::current_path()));

		ImGui::SetDragDropPayload(DragDropConstants::VertexShader.c_str(), p.ToString().c_str(), p.ToString().size(), 0);

		ImGui::Text(aPath.string().c_str());

		if (myVertexShaderIconTexture->IsValid())
		{
			ImGui::Image(myVertexShaderIconTexture->Get().GetSRV(), ImVec2(32, 32));
		}

		ImGui::EndDragDropSource();
	}
}

void Engine::AssetBrowserWindow::DrawPixelShaderIcon(const int aPathIndex, const std::filesystem::path& aPath)
{
	auto onRightClickMenu = [&]()
	{};

	DrawFileIcon(aPathIndex, aPath, myPixelShaderIconTexture, onRightClickMenu);

	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
	{
		Path p(aPath);
		p.MakeRelativeTo(Path(std::filesystem::current_path()));

		ImGui::SetDragDropPayload(DragDropConstants::PixelShader.c_str(), p.ToString().c_str(), p.ToString().size(), 0);

		ImGui::Text(aPath.string().c_str());

		if (myPixelShaderIconTexture->IsValid())
		{
			ImGui::Image(myPixelShaderIconTexture->Get().GetSRV(), ImVec2(32, 32));
		}

		ImGui::EndDragDropSource();
	}
}

void Engine::AssetBrowserWindow::DrawGeometryShaderIcon(const int aPathIndex, const std::filesystem::path& aPath)
{
	auto onRightClickMenu = [&]()
	{};

	DrawFileIcon(aPathIndex, aPath, myGeometryShaderIconTexture, onRightClickMenu);

	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
	{
		Path p(aPath);
		p.MakeRelativeTo(Path(std::filesystem::current_path()));

		ImGui::SetDragDropPayload(DragDropConstants::GeometryShader.c_str(), p.ToString().c_str(), p.ToString().size(), 0);

		ImGui::Text(aPath.string().c_str());

		if (myPixelShaderIconTexture->IsValid())
		{
			ImGui::Image(myGeometryShaderIconTexture->Get().GetSRV(), ImVec2(32, 32));
		}

		ImGui::EndDragDropSource();
	}
}

void Engine::AssetBrowserWindow::DrawUnknownFileIcon(const int aPathIndex, const std::filesystem::path& aPath)
{
	auto onRightClickMenu = [&]()
	{};

	DrawFileIcon(aPathIndex, aPath, myUnknownFileTexture, onRightClickMenu);
}

void Engine::AssetBrowserWindow::HandleRightClickEmptySpace()
{
	// When clicking in empty space in thumbnail assets browser
	// and it we have NOT right clicked an item
	if (!myOpenedIconPopup && ImGui::BeginPopupContextItem("RightClickEmptyAssetBrowser2"))
	{
		DrawRightClickEmptyMenu();

		ImGui::EndPopup();
	}
}

void Engine::AssetBrowserWindow::HandleGameObjectDragDropped()
{
	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GameObject");

		if (payload)
		{
			auto droppedGameObject = *reinterpret_cast<Shared<GameObject>*>(payload->Data);
			assert(payload->DataSize == sizeof(Shared<GameObject>));

			// Create prefab from the gameobject
			if (GameObjectPrefab::Serialize(*droppedGameObject, Path(myCurrentDirectoryAbsolute)))
			{
				// succeeded
			}
			else
			{
				// failed
			}

			// droppedGameObject->GetTransform().SetParent(&aGameobject.GetTransform());
		}

		ImGui::EndDragDropTarget();
	}
}

void Engine::AssetBrowserWindow::CreateNewFileOf(
	const std::string& aNewFileNameStem,
	const std::string& aExtension,
	const std::string& aTemplatePath)
{
	const std::filesystem::path allowedName =
		FileIO::GenerateFirstValidFileName(aNewFileNameStem + aExtension, myCurrentDirectoryRelative);

	const auto resultPath = myCurrentDirectoryRelative / allowedName;

	const int directoryVersion = AssetDatabase::GetCachedDirectoryVersion();

	std::filesystem::copy(aTemplatePath + aExtension, resultPath);

	// Must refresh because we created new file, otherwise 
	// we could not select it immediately
	{
		while (AssetDatabase::GetLatestDirectoryVersion() == directoryVersion)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		AssetDatabase::UpdateDatabase();
	}

	SetSelectedFileByPath(resultPath);
	ScrollSelectedItemVisible();

	StartRenaming();
}

void Engine::AssetBrowserWindow::StartRenaming()
{
	myIsRenamingSelectedFile = true;
	strcpy_s(myRenameBuffer, GetSelectedFileName(0).stem().string().c_str());
}

void Engine::AssetBrowserWindow::ApplyRenaming()
{
	if (!myIsRenamingSelectedFile)
	{
		return;
	}

	auto f = GetSelectedFileName(0);
	auto newNamePath = myCurrentDirectoryRelative / f;

	const auto extension = newNamePath.extension();

	// newNamePath.replace_filename(myRenameBuffer);
	newNamePath.replace_filename(std::string(myRenameBuffer) + extension.string());

	std::filesystem::rename(myCurrentDirectoryRelative / GetSelectedFileName(0), newNamePath);
	myIsRenamingSelectedFile = false;
	myHasGivenFocusToRenameInputText = false;

	SetSelectedFileByPath(newNamePath);
	ScrollSelectedItemVisible();
}

void Engine::AssetBrowserWindow::AbortRenaming()
{
	myIsRenamingSelectedFile = false;
	myHasGivenFocusToRenameInputText = false;
}

void Engine::AssetBrowserWindow::RefreshCachedCurrentDirEntries()
{
	const auto& currentDirectory = AssetDatabase::GetDirectory(myCurrentDirectoryRelative);

	myCurrentDirEntries.clear();

	for (const auto& folder : currentDirectory.myFolders)
	{
		myCurrentDirEntries.push_back(DirectoryEntry(folder->myPath, true));
	}
	for (const auto& file : currentDirectory.myFiles)
	{
		myCurrentDirEntries.push_back(DirectoryEntry(file->myPath, false));
	}

	// Sort it based on file type
	std::sort(
		myCurrentDirEntries.begin(),
		myCurrentDirEntries.end(),
		[](const DirectoryEntry& aLeft,
			const DirectoryEntry& aRight)
		{
			// Folders are sorted first
			int leftExtension = -1;
			int rightExtension = -1;

			if (aLeft.myPath.has_extension())
			{
				leftExtension = static_cast<int>(FileType::ExtensionToFileType(aLeft.myPath.extension().string()));
			}

			if (aRight.myPath.has_extension())
			{
				rightExtension = static_cast<int>(FileType::ExtensionToFileType(aRight.myPath.extension().string()));
			}

			return leftExtension < rightExtension;
		});
}

void Engine::AssetBrowserWindow::AllFilesContextMenu(const std::filesystem::path& aPath)
{
	DrawRightClickEmptyMenu();

	if (ImGui::MenuItem("Rename"))
	{
		StartRenaming();
	}

	if (ImGui::MenuItem("Show in explorer"))
	{
		FileIO::ShowFileInExplorer(std::filesystem::absolute(aPath));
	}
}

bool Engine::AssetBrowserWindow::IsFileSelected(const int aPathIndex) const
{
	return mySelectedPathIndex == aPathIndex;
}

std::filesystem::path Engine::AssetBrowserWindow::GetSelectedFileName(const int aIndex) const
{
	return myCurrentDirEntries[mySelectedPathIndex].myPath.filename();
}

void Engine::AssetBrowserWindow::DrawThumbnailSizeSlider()
{
	ImGui::SetNextItemWidth(100);
	if (ImGui::SliderFloat("Thumbnail Size", &myThumbnailSize, 17.f, 90.f))
	{
		if (myThumbnailSize <= 17.f)
		{
			myThumbnailImageSize = myThumbnailSize;

			myFileViewType = FileViewType::ListView;
		}
		else
		{
			myThumbnailImageSize = myThumbnailSize - 8.f;

			myFileViewType = FileViewType::Thumbnails;
		}
	}
}

void Engine::AssetBrowserWindow::DrawRightClickEmptyMenu()
{
	if (ImGui::BeginMenu("Create"))
	{
		if (ImGui::MenuItem("Visual script"))
		{
			CreateNewFileOf(
				"NewVisualScript",
				FileType::FileTypeToExtension(FileType::FileType::VisualScript),
				"Assets\\Engine\\Editor\\Graph\\DefaultGraph");
		}

		if (ImGui::MenuItem("Animation state machine"))
		{
			CreateNewFileOf(
				"NewAnimMachine",
				FileType::FileTypeToExtension(FileType::FileType::AnimationStateMachine),
				"Assets\\Engine\\Editor\\AnimationStateMachineEditor\\Default");
		}

		if (ImGui::MenuItem("Animation curve"))
		{
			CreateNewFileOf(
				"NewAnimCurve",
				FileType::FileTypeToExtension(FileType::FileType::AnimationCurve),
				"Assets\\Engine\\Editor\\AnimationCurve\\Default");
		}

		if (ImGui::BeginMenu("Material"))
		{
			if (ImGui::MenuItem("Mesh"))
			{
				CreateNewFileOf(
					"NewMeshMaterial",
					FileType::FileTypeToExtension(FileType::FileType::MeshMaterial),
					"Assets\\Engine\\Materials\\DefaultDeferred");
			}

			if (ImGui::MenuItem("Skybox (Mesh)"))
			{
				CreateNewFileOf(
					"NewSkyboxMaterial",
					FileType::FileTypeToExtension(FileType::FileType::MeshMaterial),
					"Assets\\Engine\\Materials\\DefaultSkybox");
			}

			if (ImGui::MenuItem("Decal"))
			{
				CreateNewFileOf(
					"NewDecalMaterial",
					FileType::FileTypeToExtension(FileType::FileType::DecalMaterial),
					"Assets\\Engine\\Materials\\Default");
			}

			if (ImGui::MenuItem("Particle"))
			{
				CreateNewFileOf(
					"NewParticleMaterial",
					FileType::FileTypeToExtension(FileType::FileType::ParticleMaterial),
					"Assets\\Engine\\Materials\\Default");
			}

			if (ImGui::MenuItem("Sprite"))
			{
				CreateNewFileOf(
					"NewSpriteMaterial",
					FileType::FileTypeToExtension(FileType::FileType::SpriteMaterial),
					"Assets\\Engine\\Materials\\Default");
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Shader"))
		{
			if (ImGui::BeginMenu("Mesh"))
			{
				if (ImGui::MenuItem("Vertex Shader Static"))
				{
					CreateNewFileOf(
						"MeshStaticVS",
						FileType::FileTypeToExtension(FileType::FileType::VertexShader),
						"Assets\\Engine\\ShaderTemplates\\TemplateMeshStaticVS");
				}
				if (ImGui::MenuItem("Vertex Shader Dynamic (Animated)"))
				{
					CreateNewFileOf(
						"MeshDynamicVS",
						FileType::FileTypeToExtension(FileType::FileType::VertexShader),
						"Assets\\Engine\\ShaderTemplates\\TemplateMeshDynamicVS");
				}
				if (ImGui::MenuItem("Pixel Shader Deferred"))
				{
					CreateNewFileOf(
						"MeshDeferredPS",
						FileType::FileTypeToExtension(FileType::FileType::PixelShader),
						"Assets\\Engine\\ShaderTemplates\\TemplateMeshDeferred");
				}
				if (ImGui::MenuItem("Pixel Shader Forward Lit"))
				{
					CreateNewFileOf(
						"MeshForwardLitPS",
						FileType::FileTypeToExtension(FileType::FileType::PixelShader),
						"Assets\\Engine\\ShaderTemplates\\TemplateMeshForwardLitPS");
				}
				else if (ImGui::MenuItem("Pixel Shader Forward UnLit"))
				{
					CreateNewFileOf(
						"MeshForwardLitPS",
						FileType::FileTypeToExtension(FileType::FileType::PixelShader),
						"Assets\\Engine\\ShaderTemplates\\TemplateMeshForwardUnlitPS");
				}

				ImGui::Separator();

				if (ImGui::MenuItem("Pixel Shader Skybox"))
				{
					CreateNewFileOf(
						"NewSkyboxPS",
						FileType::FileTypeToExtension(FileType::FileType::PixelShader),
						"Assets\\Engine\\ShaderTemplates\\TemplateSkyBoxPS");
				}
				if (ImGui::MenuItem("Vertex Shader Skybox"))
				{
					CreateNewFileOf(
						"NewSkyboxVS",
						FileType::FileTypeToExtension(FileType::FileType::VertexShader),
						"Assets\\Engine\\ShaderTemplates\\TemplateSkyBoxVS");
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Decal"))
			{
				if (ImGui::MenuItem("Vertex Shader"))
				{
					CreateNewFileOf(
						"DecalVS",
						FileType::FileTypeToExtension(FileType::FileType::VertexShader),
						"Assets\\Engine\\ShaderTemplates\\TemplateDecalVS");
				}
				if (ImGui::MenuItem("Pixel Shader"))
				{
					CreateNewFileOf(
						"DecalPS",
						FileType::FileTypeToExtension(FileType::FileType::PixelShader),
						"Assets\\Engine\\ShaderTemplates\\TemplateDecalPS");
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Particle"))
			{
				if (ImGui::MenuItem("Vertex Shader"))
				{
					CreateNewFileOf(
						"ParticleVS",
						FileType::FileTypeToExtension(FileType::FileType::VertexShader),
						"Assets\\Engine\\ShaderTemplates\\TemplateParticleVS");
				}
				if (ImGui::MenuItem("Pixel Shader"))
				{
					CreateNewFileOf(
						"ParticlePS",
						FileType::FileTypeToExtension(FileType::FileType::PixelShader),
						"Assets\\Engine\\ShaderTemplates\\TemplateParticlePS");
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("2D Sprite"))
			{
				if (ImGui::MenuItem("Vertex Shader"))
				{
					CreateNewFileOf(
						"SpriteVS",
						FileType::FileTypeToExtension(FileType::FileType::VertexShader),
						"Assets\\Engine\\ShaderTemplates\\TemplateSpriteVS");
				}
				if (ImGui::MenuItem("Pixel Shader"))
				{
					CreateNewFileOf(
						"SpritePS",
						FileType::FileTypeToExtension(FileType::FileType::PixelShader),
						"Assets\\Engine\\ShaderTemplates\\TemplateSpritePS");
				}
				if (ImGui::MenuItem("Geometry Shader"))
				{
					CreateNewFileOf(
						"SpritePS",
						FileType::FileTypeToExtension(FileType::FileType::GeometryShader),
						"Assets\\Engine\\ShaderTemplates\\TemplateSpriteGS");
				}

				ImGui::EndMenu();
			}

			ImGui::EndMenu();
		}

		ImGui::EndMenu();
	}
}

void Engine::AssetBrowserWindow::DrawSearchBox()
{
	bool changedSearch = false;

	// focus on the text input
	if (myShouldFocusOnSearchInput)
	{
		myShouldFocusOnSearchInput = false;

		ImGui::SetKeyboardFocusHere();

		changedSearch = true;
	}

	if (ImGui::InputTextWithHint(
		"##searchfile",
		"Search file",
		mySearchBuffer,
		sizeof(mySearchBuffer)))
	{
		// For some reason InputTextWithHint() returns true when hitting escape
		// while it is active, therfore just edge case that shit away
		if (!ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_Escape)))
		{
			changedSearch = true;
		}
	}

	ImGui::SameLine();

	if (ImGui::ImageButton(myXIcon->Get().GetSRV(), ImVec2(16, 16)))
	{
		StopSearching();
		return;
	}

	const std::string pathSearchBufferStr = mySearchBuffer;

	if (pathSearchBufferStr.empty())
	{
		if (changedSearch)
		{
			myCurrentDirEntries.clear();

			for (const auto& entry : myCachedAssetsFilePaths)
			{
				// dont search for folders
				if (entry.myIsDirectory)
				{
					continue;
				}

				myCurrentDirEntries.push_back(entry);
			}

			if (!myCurrentDirEntries.empty())
			{
				mySelectedPathIndex = 0;
				SetSelectedFile(mySelectedPathIndex);
				//SetSelectedFile(myFilePathsInList[mySelectedSearchedFileIndex].myPath);
			}
		}
	}
	else
	{
		if (changedSearch)
		{
			myCurrentDirEntries.clear();

			// Search using keys separated by spaces
			const std::vector<std::string> searchKeys = StringUtilities::Split(pathSearchBufferStr, " ");

			for (const auto& entry : myCachedAssetsFilePaths)
			{
				// dont search for folders
				if (entry.myIsDirectory)
				{
					continue;
				}

				const auto filename = entry.myPath.filename().string();

				bool allKeysMatch = true;

				for (const std::string& searchKey : searchKeys)
				{
					auto findResult = std::search(
						filename.begin(),
						filename.end(),
						searchKey.begin(),
						searchKey.end(),
						[](char c1, char c2)
						{
							return toupper(c1) == toupper(c2);
						});

					if (findResult == filename.end())
					{
						allKeysMatch = false;
						break;
					}
				}

				if (allKeysMatch)
				{
					myCurrentDirEntries.push_back(entry);
				}
			}

			if (!myCurrentDirEntries.empty())
			{
				mySelectedPathIndex = 0;
				SetSelectedFile(mySelectedPathIndex);
				//SetSelectedFile(myFilePathsInList[mySelectedSearchedFileIndex].myPath);
			}
		}
	}
}

void Engine::AssetBrowserWindow::StartSearching()
{
	myIsSearching = true;
	myShouldFocusOnSearchInput = true;
	memset(mySearchBuffer, 0, sizeof(mySearchBuffer));

	myCachedEntriesCurrentDirBackup = myCurrentDirEntries;
}

void Engine::AssetBrowserWindow::StopSearching()
{
	myIsSearching = false;

	if (mySelectedSomethingDuringSearch && myDirectoryIndexSelectedDuringSearch < myCurrentDirEntries.size())
	{
		const DirectoryEntry dir = myCurrentDirEntries[myDirectoryIndexSelectedDuringSearch];

		if (dir.myIsDirectory)
		{
			GotoDirectory(dir.myPath);
		}
		else
		{
			GotoDirectory(dir.myPath.parent_path());
			SetSelectedFileByPath(dir.myPath);
			ScrollSelectedItemVisible();
		}
	}
	else
	{
		// Restore the backup
		myCurrentDirEntries = myCachedEntriesCurrentDirBackup;
	}
}

void Engine::AssetBrowserWindow::RefreshCachedAssetsFilePaths()
{
	myCachedAssetsFilePaths.clear();

	for (const auto& folder : AssetDatabase::GetFolders())
	{
		myCachedAssetsFilePaths.push_back(DirectoryEntry(folder->myPath, true));
	}
	for (const auto& file : AssetDatabase::GetFiles())
	{
		myCachedAssetsFilePaths.push_back(DirectoryEntry(file->myPath, false));
	}
}

void Engine::AssetBrowserWindow::ScrollSelectedItemVisible()
{
	myScrollSelectedItemVisible = true;
}

void Engine::AssetBrowserWindow::UpdateImplicitSearchTimer(const float aDeltaTime)
{
	myImplicitSearchTimeoutTimerSec -= aDeltaTime;

	if (myImplicitSearchTimeoutTimerSec < 0.f)
	{
		myImplicitSearchTerm.clear();
		myImplicitSearchTimeoutTimerSec = 0.f;
	}
}

void Engine::AssetBrowserWindow::UpdateImplicitSearch()
{
	if (myIsSearching || myIsRenamingSelectedFile)
	{
		return;
	}

	for (int i = ImGui::GetKeyIndex(ImGuiKey_A);
		i < ImGui::GetKeyIndex(ImGuiKey_Z);
		++i)
	{
		if (ImGui::IsKeyPressed(i, false))
		{
			// reset timer
			myImplicitSearchTimeoutTimerSec = ourImplicitSearchTimeoutDurationSec;

			myImplicitSearchTerm += static_cast<char>(i);

			for (int i = 0; i < myCurrentDirEntries.size(); ++i)
			{
				auto filename = myCurrentDirEntries[i].myPath.filename().string();

				std::string sub = filename.substr(0, std::min(myImplicitSearchTerm.size(), filename.size()));
				sub = StringUtilities::ToLower(sub);

				if (sub == StringUtilities::ToLower(myImplicitSearchTerm))
				{
					SetSelectedFileByPath(myCurrentDirEntries[i].myPath);
					ScrollSelectedItemVisible();
					break;
				}
			}

		}
	}
}

void Engine::AssetBrowserWindow::FileChangedNotificationCallback()
{
	RefreshCachedCurrentDirEntries();
	RefreshCachedAssetsFilePaths();
}

bool Engine::AssetBrowserWindow::ReimportModel(const std::filesystem::path& aPath)
{
	// Get the respective FBX
	std::filesystem::path fbxPath = aPath;
	fbxPath.replace_extension(FileType::Extension_FBX);

	auto modelRef = GResourceManager->CreateRef<ModelResource>(aPath.string());
	modelRef->Load();

	if (modelRef && modelRef->IsValid())
	{
		Assimp::Importer myImporter;
		LoaderModel loaderModel(&myImporter);

		if (!loaderModel.Init(fbxPath.string()))
		{
			LOG_ERROR(LogType::Editor) << "Unable to open " << fbxPath;
			return false;
		}

		// Get the model materials on each mesh
		const auto& meshes = modelRef->Get().GetMeshDatas();

		if (meshes.size() != loaderModel.GetMeshes().size())
		{
			LOG_ERROR(LogType::Editor) << "Unable to reimport because they have different amount of meshes";
			return false;
		}

		// Create a loader model with those materials
		for (size_t meshIndex = 0; meshIndex < meshes.size(); ++meshIndex)
		{
			loaderModel.GetMeshes()[meshIndex].myMaterialPath = meshes[meshIndex].myDefaultMaterial->GetPath();
		}

		// Export the loader model like normal
		FlatbufferModel::ConvertAndSaveFlatbufferModel(
			loaderModel,
			aPath.string());

		// funkar reimport nu?
	}
	else
	{
		LOG_ERROR(LogType::Editor) << "Failed to load the model " << aPath;
		return false;
	}

	return true;
}

const std::vector<Engine::DirectoryEntry>& Engine::AssetBrowserWindow::GetCachedAssetsFilePaths() const
{
	return myCachedAssetsFilePaths;
}

void Engine::AssetBrowserWindow::GotoDirectory(const std::filesystem::path& aPath)
{
	if (myIsSearching)
	{
		StopSearching();
	}

	myCurrentDirectoryAbsolute = aPath;
	myCurrentDirectoryRelative = std::filesystem::relative(myCurrentDirectoryAbsolute);

	RefreshCachedCurrentDirEntries();
	//const auto& temp = AssetDatabase::GetDirectory(myCurrentDirectoryRelative);

	//if (myCurrentDirChangeNotificationHandle != nullptr)
	//{
	//	BOOL result = FindCloseChangeNotification(myCurrentDirChangeNotificationHandle);
	//	assert(result);
	//	myCurrentDirChangeNotificationHandle = nullptr;
	//}

	//myCurrentDirChangeNotificationHandle = FindFirstChangeNotification(
	//	aPath.wstring().c_str(),
	//	FALSE,
	//	FILE_NOTIFY_CHANGE_FILE_NAME |
	//	FILE_NOTIFY_CHANGE_DIR_NAME |
	//	FILE_NOTIFY_CHANGE_ATTRIBUTES |
	//	FILE_NOTIFY_CHANGE_SIZE |
	//	FILE_NOTIFY_CHANGE_LAST_WRITE |
	//	FILE_NOTIFY_CHANGE_SECURITY);

	//if (myCurrentDirChangeNotificationHandle == INVALID_HANDLE_VALUE)
	//{
	//	LOG_ERROR(LogType::Editor) << "Failed to start watching the path: " <<
	//		myCurrentDirChangeNotificationHandle;
	//}

	// TODO: scroll to top

	mySelectedPathIndex = -1;
}

void Engine::AssetBrowserWindow::SetSelectedFile(const int aIndex/*const std::filesystem::path& aPathIncludingFilename*/)
{
	// mySelectedFileName = aPathIncludingFilename.filename();
	// mySelectedFilePath = aPathIncludingFilename;

	if (myIsRenamingSelectedFile)
	{
		ApplyRenaming();
	}

	mySelectedPathIndex = aIndex;

	if (myIsSearching)
	{
		mySelectedSomethingDuringSearch = true;

		myDirectoryIndexSelectedDuringSearch = aIndex;

		// myDirectorySelectedDuringSearch = DirectoryEntry(aPathIncludingFilename, std::filesystem::is_directory(aPathIncludingFilename));
	}
}

bool Engine::AssetBrowserWindow::SetSelectedFileByPath(const std::filesystem::path& aPathIncludingFilename)
{
	int index = -1;

	for (int i = 0; i < myCurrentDirEntries.size(); ++i)
	{
		if (myCurrentDirEntries[i].myPath == aPathIncludingFilename)
		{
			index = i;
			break;
		}
	}

	if (index == -1)
	{
		return false;
	}

	SetSelectedFile(index);

	return true;
}

std::filesystem::path Engine::AssetBrowserWindow::GetSelectedFileFullRelativePath() const
{
	return myCurrentDirEntries[mySelectedPathIndex].myPath;
}
