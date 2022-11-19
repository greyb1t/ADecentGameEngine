#include "pch.h"
#include "ModelImportState.h"
#include "Engine\Renderer\ModelLoader\LoaderModel.h"
#include "Engine\Renderer\ModelLoader\Binary\LoaderModelBinary.h"
#include "..\Resources\UnknownResource.h"
#include "..\Resources\ModelTextureResource.h"
#include "Common\TimerTemp.h"
#include "Engine\Engine.h"
#include "Engine\ResourceManagement\ResourceManager.h"
#include "Engine\ResourceManagement\Resources\ModelResource.h"
#include "Engine\GameObject\GameObject.h"
#include "Engine\GameObject\Components\ModelComponent.h"
#include "Engine/Renderer/ModelLoader/Binary/FlatbufferModel.h"
#include "Engine\Editor\ImGuiHelper.h"
#include "Engine\Editor\DragDropConstants.h"
#include "Engine\ResourceManagement\Resources\TextureResource.h"
#include "Engine\Editor\FileTypes.h"
#include "Engine\Editor\Editor.h"
#include "Engine\Renderer\GraphicsEngine.h"
#include "Engine\Renderer\Material\MaterialFactory.h"
#include "Engine\ResourceManagement\Resources\MaterialResource.h"
#include "Engine\Editor/EditorWindows/AssetBrowser/AssetBrowserWindow.h"
#include "Engine\Editor/EditorWindows/Inspector/InspectorWindow.h"
#include "Engine/Renderer/Texture/Texture2D.h"

Engine::ModelImportState::ModelImportState(const std::filesystem::path& aPath, Editor& aEditor)
	: myEditor(&aEditor)
{
	Owned<Assimp::Importer> myImporter = MakeOwned<Assimp::Importer>();

	auto loaderModel = MakeOwned<LoaderModel>(myImporter.get());

	if (!loaderModel->Init(aPath.string()))
	{
		LOG_ERROR(LogType::Resource) << "Failed to parse the model from assimp " << aPath.string();

		return;
	}

	for (int i = 0; i < loaderModel->myMeshes.size(); ++i)
	{
		const std::string* materialPath = GetEngine().GetGraphicsEngine().GetMaterialFactory().ConvertMaterialNameToPath(loaderModel->myMeshes[i].myMaterialStem);
		if (materialPath)
		{
			auto r = GResourceManager->CreateRef<MaterialResource>(*materialPath);
			r->Load();
			myMaterialRefs.push_back(r);

			loaderModel->myMeshes[i].myMaterialPath = r->GetPath();
		}
		else
		{
			myMaterialRefs.push_back(GetEngine().GetGraphicsEngine().GetMaterialFactory().GetDefaultMeshMaterialDeferred());

			loaderModel->myMeshes[i].myMaterialPath = GetEngine().GetGraphicsEngine().GetMaterialFactory().GetDefaultMeshMaterialDeferred()->GetPath();
		}
	}

	myLoaderModels.push_back(std::move(loaderModel));
}

Engine::ModelImportState::ModelImportState()
{
}

Engine::ModelImportState::~ModelImportState()
{
}

bool Engine::ModelImportState::Draw()
{
	// Handle the unknown files
	if (!myResourceToHandle.empty())
	{
		ImGui::OpenPopup("HandleUnknownResource");
	}

	ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_Appearing);
	if (ImGui::BeginPopupModal("HandleUnknownResource"))
	{
		const auto& resource = myResourceToHandle.back();

		if (resource->Draw(*this))
		{
			// When we handled it, remove it and close popup
			myResourceToHandle.pop_back();
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	switch (myState)
	{
	case State::Default:
		return DrawDefault();
		break;
	case State::MassImport:
		return DrawMassImport();
		break;
	case State::HoudiniFBXMssImport:
		return DrawHoudiniFBXMassImport();
		break;
	default:
		break;
	}

	return true;
}

void Engine::ModelImportState::ParseFiles(const std::vector<std::filesystem::path>& aPaths)
{
	for (const auto& path : aPaths)
	{
		const std::string extension = StringUtilities::ToLower(path.extension().string());

		if (extension == ".dds")
		{
			const std::string name = StringUtilities::ToLower(path.stem().string());

			if (StringUtilities::EndsWith(name, "_c"))
			{
				myResourceToHandle.push_back(MakeOwned<ModelTextureResource>(path, ModelTextureType::Albedo));
			}
			else if (StringUtilities::EndsWith(name, "_n"))
			{
				myResourceToHandle.push_back(MakeOwned<ModelTextureResource>(path, ModelTextureType::Normal));
			}
			else if (StringUtilities::EndsWith(name, "_m"))
			{
				myResourceToHandle.push_back(MakeOwned<ModelTextureResource>(path, ModelTextureType::Material));
			}
			else
			{
				myResourceToHandle.push_back(MakeOwned<UnknownResource>(path));
			}
		}
		else if (extension == ".fbx")
		{
			if (myState == State::MassImport)
			{
				Owned<Assimp::Importer> myImporter = MakeOwned<Assimp::Importer>();

				auto loaderModel = MakeOwned<LoaderModel>(myImporter.get());

				if (!loaderModel->Init(path.string()))
				{
					loaderModel = nullptr;

					LOG_ERROR(LogType::Resource) << "Failed to parse the model from assimp " << path.string();

					return;
				}

				const auto originalPath = std::filesystem::path(loaderModel->GetPath());
				auto destinationPath = originalPath.parent_path();
				destinationPath.append(originalPath.stem().string() + FileType::Extension_Model);

				FlatbufferModel::ConvertAndSaveFlatbufferModel(*loaderModel, destinationPath.string());
			}
			else if (myState == State::HoudiniFBXMssImport)
			{
				Owned<Assimp::Importer> myImporter = MakeOwned<Assimp::Importer>();

				auto loaderModel = MakeOwned<LoaderModel>(myImporter.get());

				if (!loaderModel->Init(path.string()))
				{
					LOG_ERROR(LogType::Resource) << "Failed to parse the model from assimp " << path.string();

					return;
				}

				myLoaderModels.push_back(std::move(loaderModel));
			}
			else
			{
				Owned<Assimp::Importer> myImporter = MakeOwned<Assimp::Importer>();

				auto loaderModel = MakeOwned<LoaderModel>(myImporter.get());

				if (!loaderModel->Init(path.string()))
				{
					LOG_ERROR(LogType::Resource) << "Failed to parse the model from assimp " << path.string();

					return;
				}

				myLoaderModels.push_back(std::move(loaderModel));
			}

			//Owned<Assimp::Importer> myImporter = MakeOwned<Assimp::Importer>();

			//myLoaderModel = MakeOwned<Renderer::LoaderModel>(myImporter.get());

			//if (!myLoaderModel->Init(path.string()))
			//{
			//	LOG_ERROR(LogType::Resource) << "Failed to parse the model from assimp " << path.string();
			//	return;
			//}

			// TODO: Show the model while making it
			/*
			auto fbxResource = GetEngine().GetResourceManager().GetResourceNow<Renderer::FbxResource>(
				path.string(),
				Renderer::ResourceId(path.string()));

			if (fbxResource->IsValid())
			{
				if (myModelGameObject)
				{
					myModelGameObject->Destroy();
				}

				myModelGameObject = myMainScene.AddGameObject<GameObject>();
				auto modelComponent = myModelGameObject->AddComponent<Engine::ModelComponent>(path.string());
			}
			*/
		}
		else
		{
			// unknown file extension
		}
	}
}

Engine::ModelImportState::State Engine::ModelImportState::GetState() const
{
	return myState;
}

ImVec4 Engine::ModelImportState::GetColorFromPath(const std::string& aPath) const
{
	if (aPath.empty())
	{
		return ImVec4(1, 0, 0, 1);
	}
	else
	{
		return ImVec4(0, 1, 0, 1);
	}
}

void Engine::ModelImportState::DrawTextureTable(LoaderMesh& aLoaderMesh)
{
	static ImGuiTableFlags flags =
		ImGuiTableFlags_SizingFixedFit |
		ImGuiTableFlags_RowBg |
		ImGuiTableFlags_Borders |
		ImGuiTableFlags_Resizable |
		ImGuiTableFlags_Reorderable |
		ImGuiTableFlags_Hideable;

	if (ImGui::BeginTable("Textures", 3, flags))
	{
		ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed);
		ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed);
		ImGui::TableSetupColumn("Path", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableHeadersRow();

		for (int i = 0; i < static_cast<int>(ModelTextureType::Count); i++)
		{
			ImGui::PushID(i);

			ImGui::TableNextRow();

			std::filesystem::path texturePath;

			// TODO: Can store these in an array of std::string instead of individual variables
			switch (static_cast<ModelTextureType>(i))
			{
			case ModelTextureType::Albedo:
				texturePath = aLoaderMesh.myAlbedoTexturePath;
				break;
			case ModelTextureType::Normal:
				texturePath = aLoaderMesh.myNormalTexturePath;
				break;
			case ModelTextureType::Material:
				texturePath = aLoaderMesh.myMaterialTexturePath;
				break;
			default:
				break;
			}

			const auto textColor = GetColorFromPath(texturePath.string());

			for (int column = 0; column < 3; column++)
			{
				ImGui::TableSetColumnIndex(column);

				switch (column)
				{
				case 0:
				{
					const std::string textureTypeString =
						ModelTextureTypeToString(static_cast<ModelTextureType>(i));

					ImGui::TextColored(textColor, "%s", textureTypeString.c_str());
				} break;

				case 1:
				{
					ImGui::GetItemRectMin();
					ImGui::GetItemRectMax();

					auto pos = ImGui::GetCursorPos();
					ImGui::InvisibleButton("dasdas", ImGui::GetItemRectSize());

					if (auto payload = ImGuiHelper::BeginDragDropTarget(DragDropConstants::Texture))
					{
						char path[255] = {};
						memcpy(path, payload->Data, payload->DataSize);

						auto textureResource = GResourceManager->CreateRef<TextureResource>(path);
						textureResource->Load();

						if (textureResource->IsValid())
						{
							switch (static_cast<ModelTextureType>(i))
							{
							case ModelTextureType::Albedo:
								aLoaderMesh.myAlbedoTexturePath = path;
								break;
							case ModelTextureType::Normal:
								aLoaderMesh.myNormalTexturePath = path;
								break;
							case ModelTextureType::Material:
								aLoaderMesh.myMaterialTexturePath = path;
								break;
							default:
								break;
							}
						}
					}

					ImGui::SetCursorPos(pos);
					ImGui::TextColored(textColor, "%s", texturePath.stem().string().c_str());
				} break;

				case 2:
				{
					ImGui::TextColored(textColor, "%s", texturePath.string().c_str());
				} break;

				default:
					break;
				}
			}

			ImGui::PopID();
		}
		ImGui::EndTable();
	}
}

bool Engine::ModelImportState::DrawDefault()
{
	if (!myLoaderModels.empty())
	{
		for (int i = 0; i < myLoaderModels.size(); ++i)
		{
			auto& loaderModel = myLoaderModels[i];

			ImGui::Text(loaderModel->GetPath().c_str());

			static ImGuiTableFlags flags =
				ImGuiTableFlags_SizingFixedFit |
				ImGuiTableFlags_RowBg |
				ImGuiTableFlags_Borders |
				ImGuiTableFlags_Resizable |
				ImGuiTableFlags_Reorderable |
				ImGuiTableFlags_Hideable;

			if (ImGui::BeginTable("Meshes", 2, flags))
			{
				ImGui::TableSetupColumn("Mesh name", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("Textures", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableHeadersRow();

				for (int i = 0; i < loaderModel->GetMeshes().size(); ++i)
				{
					ImGui::PushID(i);

					auto& loaderMesh = loaderModel->GetMeshes()[i];

					ImGui::TableNextRow();

					for (int column = 0; column < 2; column++)
					{
						ImGui::TableSetColumnIndex(column);

						switch (column)
						{
						case 0:
							ImGui::Text(loaderMesh.GetName().c_str());
							break;
						case 1:
						{
							ImGui::Text("MaterialStem: %s", loaderMesh.myMaterialStem.c_str());

							ImGui::SameLine();

							if (myEditor->DrawReflectedResource(
								myMaterialRefs[i],
								"Material",
								DragDropConstants::MeshMaterial,
								FileType::Extension_MeshMaterial))
							{
								loaderMesh.myMaterialPath = myMaterialRefs[i]->GetPath();
								// myMaterialRefs[i] = myMaterialRefs[i];
							}

							//const std::string* materialPath = GetEngine().GetGraphicsEngine().GetMaterialFactory().ConvertMaterialNameToPath(loaderMesh.myMaterialStem);
							//if (materialPath)
							//{
							//	MaterialRef matRef =
							//		GResourceManager->CreateRef<MaterialResource>(*materialPath);

							//	if (myEditor->DrawReflectedResource(
							//		matRef,
							//		"Material",
							//		DragDropConstants::MeshMaterial,
							//		FileType::Extension_MeshMaterial))
							//	{
							//		loaderMesh.myMaterialPath = matRef->GetPath();
							//		myMaterialRefs[i] = matRef;
							//	}
							//}
							//else
							//{
							//	MaterialRef matRef =
							//		GetEngine().GetGraphicsEngine().GetMaterialFactory().GetDefaultMeshMaterialDeferred();

							//	if (myEditor->DrawReflectedResource(
							//		matRef,
							//		"Material",
							//		DragDropConstants::MeshMaterial,
							//		FileType::Extension_MeshMaterial))
							//	{
							//		loaderMesh.myMaterialPath = matRef->GetPath();
							//		myMaterialRefs[i] = matRef;
							//	}
							//}
							// DrawTextureTable(loaderMesh);
						} break;
						default:
							break;
						}
					}

					ImGui::PopID();
				}
				ImGui::EndTable();
			}

			if (ImGui::Button("Export"))
			{
				START_TIMER(tim);

				const auto originalPath = std::filesystem::path(loaderModel->GetPath());
				auto destinationPath = originalPath.parent_path();
				destinationPath.append(originalPath.stem().string() + FileType::Extension_Model);

				FlatbufferModel::ConvertAndSaveFlatbufferModel(*loaderModel, destinationPath.string());

				float ms = END_TIMER_GET_RESULT_MS(tim);

				LOG_INFO(LogType::Filip) << "Ms: " << ms;

				myLoaderModels.erase(myLoaderModels.begin() + i);
				break;
			}
		}
	}
	else
	{
		ImGui::TextColored(ImVec4(0, 1, 0, 1), "Drag model FBX here");

		if (ImGui::Button("Mass Import FBXes without textures"))
		{
			myState = State::MassImport;
		}

		if (ImGui::Button("Import Houdini FBX"))
		{
			myState = State::HoudiniFBXMssImport;
		}
	}

	return true;
}

bool Engine::ModelImportState::DrawMassImport()
{
	if (!myLoaderModels.empty())
	{
	}
	else
	{
		ImGui::TextColored(ImVec4(0, 1, 0, 1), "Drag all FBXes here");
	}

	return true;
}

bool Engine::ModelImportState::DrawHoudiniFBXMassImport()
{
	if (!myLoaderModels.empty())
	{
		std::map<std::string, std::vector<LoaderMesh*>> meshLists;

		for (auto& loaderModel : myLoaderModels)
		{
			for (auto& mesh : loaderModel->GetMeshes())
			{
				meshLists[mesh.GetName()].push_back(&mesh);
			}
		}

		for (auto& [meshName, meshList] : meshLists)
		{
			static ImGuiTableFlags flags =
				ImGuiTableFlags_SizingFixedFit |
				ImGuiTableFlags_RowBg |
				ImGuiTableFlags_Borders |
				ImGuiTableFlags_Resizable |
				ImGuiTableFlags_Reorderable |
				ImGuiTableFlags_Hideable;

			if (ImGui::BeginTable(meshName.c_str(), 1, flags))
			{
				ImGui::TableSetupColumn(("Mesh Name: " + meshName).c_str(), ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableHeadersRow();

				for (int i = 0; i < meshList.size(); ++i)
				{
					ImGui::PushID(i);

					auto& loaderMesh = meshList[i];

					ImGui::TableNextRow();

					for (int column = 0; column < 1; column++)
					{
						ImGui::TableSetColumnIndex(column);

						switch (column)
						{
						case 0:
							ImGui::Text(loaderMesh->GetLoaderModel().GetPath().c_str());
							break;
						default:
							break;
						}
					}

					ImGui::PopID();
				}

				// last row manually
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);

				static ImGuiTableFlags flags =
					ImGuiTableFlags_SizingFixedFit |
					ImGuiTableFlags_RowBg |
					ImGuiTableFlags_Borders |
					ImGuiTableFlags_Resizable |
					ImGuiTableFlags_Reorderable |
					ImGuiTableFlags_Hideable;

				if (ImGui::BeginTable("Textures", 3, flags))
				{
					ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed);
					ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed);
					ImGui::TableSetupColumn("Path", ImGuiTableColumnFlags_WidthStretch);
					ImGui::TableHeadersRow();

					for (int i = 0; i < static_cast<int>(ModelTextureType::Count); i++)
					{
						ImGui::PushID(i);

						ImGui::TableNextRow();

						std::filesystem::path texturePath;

						// TODO: Can store these in an array of std::string instead of individual variables
						switch (static_cast<ModelTextureType>(i))
						{
						case ModelTextureType::Albedo:
							texturePath = meshList[0]->myAlbedoTexturePath;
							break;
						case ModelTextureType::Normal:
							texturePath = meshList[0]->myNormalTexturePath;
							break;
						case ModelTextureType::Material:
							texturePath = meshList[0]->myMaterialTexturePath;
							break;
						default:
							break;
						}

						const auto textColor = GetColorFromPath(texturePath.string());

						for (int column = 0; column < 3; column++)
						{
							ImGui::TableSetColumnIndex(column);

							switch (column)
							{
							case 0:
							{
								const std::string textureTypeString =
									ModelTextureTypeToString(static_cast<ModelTextureType>(i));

								ImGui::TextColored(textColor, "%s", textureTypeString.c_str());
							} break;

							case 1:
							{
								ImGui::GetItemRectMin();
								ImGui::GetItemRectMax();

								auto pos = ImGui::GetCursorPos();
								ImGui::InvisibleButton("dasdas", ImGui::GetItemRectSize());

								if (auto payload = ImGuiHelper::BeginDragDropTarget(DragDropConstants::Texture))
								{
									char path[255] = {};
									memcpy(path, payload->Data, payload->DataSize);

									auto textureResource = GResourceManager->CreateRef<TextureResource>(path);
									textureResource->Load();

									if (textureResource->IsValid())
									{
										switch (static_cast<ModelTextureType>(i))
										{
										case ModelTextureType::Albedo:
											for (auto& m : meshList)
											{
												m->myAlbedoTexturePath = path;
											}
											break;
										case ModelTextureType::Normal:
											for (auto& m : meshList)
											{
												m->myNormalTexturePath = path;
											}
											break;
										case ModelTextureType::Material:
											for (auto& m : meshList)
											{
												m->myMaterialTexturePath = path;
											}
											break;
										default:
											break;
										}
									}
								}

								ImGui::SetCursorPos(pos);
								ImGui::TextColored(textColor, "%s", texturePath.stem().string().c_str());
							} break;

							case 2:
							{
								ImGui::TextColored(textColor, "%s", texturePath.string().c_str());
							} break;

							default:
								break;
							}
						}

						ImGui::PopID();
					}
					ImGui::EndTable();
				}

				ImGui::EndTable();
			}
		}

		if (ImGui::Button("Export All"))
		{
			START_TIMER(tim);

			for (auto& loaderModel : myLoaderModels)
			{
				const auto originalPath = std::filesystem::path(loaderModel->GetPath());
				auto destinationPath = originalPath.parent_path();
				destinationPath.append(originalPath.stem().string() + FileType::Extension_Model);

				FlatbufferModel::ConvertAndSaveFlatbufferModel(*loaderModel, destinationPath.string());
			}

			float ms = END_TIMER_GET_RESULT_MS(tim);

			LOG_INFO(LogType::Filip) << "Ms: " << ms;

			myLoaderModels.clear();
		}
	}
	else
	{
		ImGui::TextColored(ImVec4(0, 1, 0, 1), "Drag houdini model FBX here");
	}

	return true;
}
