#include "pch.h"
#include "AnimationCreationState.h"
#include "Engine\Renderer\ModelLoader\LoaderModel.h"
#include "..\Resources\UnknownResource.h"
#include "..\Resources\ModelTextureResource.h"
#include "Common\TimerTemp.h"
#include "Engine\Engine.h"
#include "Engine\ResourceManagement\ResourceManager.h"
#include "Engine\GameObject\GameObject.h"
#include "Engine\GameObject\Components\ModelComponent.h"
#include "Engine\Renderer\Animation\AnimationClip.h"
#include "Engine\Renderer\ModelLoader\Binary\G6Anim\G6AnimSerializerAnimationClip.h"
#include "Engine\Renderer\ModelLoader\Binary\G6Anim\G6AnimDeserializer.h"
#include "Engine\Editor\FileTypes.h"
#include "Engine\Editor\Editor.h"
#include "Engine\Editor\DragDropConstants.h"
#include "Engine/ResourceManagement/Resources/ModelResource.h"
#include "Engine/ResourceManagement/Resources/TextureResource.h"
#include "Engine/Editor/EditorWindows/AssetBrowser/AssetBrowserWindow.h"
#include "Engine/Editor/EditorWindows/Inspector/InspectorWindow.h"
#include "Engine/Renderer/Texture/Texture2D.h"
#include "Engine\Animation\AnimationTransformResult.h"
#include "Engine\Animation\State\AnimationMachine.h"

Engine::AnimationCreationState::AnimationCreationState(const std::filesystem::path& aPath, Editor& aEditor)
	: myEditor(aEditor)
{
	auto animClip = MakeOwned<AnimationClip>();

	if (!animClip->Init(aPath.string()))
	{
		LOG_ERROR(LogType::Resource) << "Failed to parse the animatino from assimp " << aPath.string();
		return;
	}

	AnimationClipEntry clipEntry;
	clipEntry.myClip = std::move(animClip);
	clipEntry.myAnimationClipPath = Path(aPath);

	myAnimationClips.push_back(std::move(clipEntry));
}

Engine::AnimationCreationState::AnimationCreationState(Editor& aEditor)
	: myEditor(aEditor)
{
}

Engine::AnimationCreationState::~AnimationCreationState()
{
}

bool Engine::AnimationCreationState::Draw()
{
	if (!myAnimationClips.empty())
	{
		if (myEditor.DrawReflectedResource(
			myAttachedFbx,
			"Model",
			DragDropConstants::Model,
			FileType::Extension_Model))
		{
			// TODO: bake in the bone name index in each OurNode
			myAttachedFbx->Load();
		}

		if (ImGui::Button("Export all"))
		{
			if (myAttachedFbx && myAttachedFbx->IsValid())
			{
				for (auto& clipEntry : myAnimationClips)
				{
					ExportAnimationClipEntry(clipEntry, myAttachedFbx);
				}

				myAnimationClips.clear();

				return false;
			}
		}

		ImGui::Separator();

		for (auto it = myAnimationClips.begin(); it != myAnimationClips.end();)
		{
			auto& clipEntry = *it;

			ImGui::Text("File: %s", clipEntry.myAnimationClipPath.GetStem().ToString().c_str());

			ImGui::Text("Duration Ticks: %f", clipEntry.myClip->GetDurationInTicks());
			ImGui::Text("Duration Seconds: %f", clipEntry.myClip->GetDurationSeconds());

			ImGui::Text("Ticks Per Second (FPS): %f", clipEntry.myClip->GetTicksPerSecond());

			ImGui::Text("Bone Count: %d", clipEntry.myClip->GetBoneNodes().size());

			//static ImGuiTableFlags flags =
			//	ImGuiTableFlags_SizingFixedFit |
			//	ImGuiTableFlags_RowBg |
			//	ImGuiTableFlags_Borders |
			//	ImGuiTableFlags_Resizable |
			//	ImGuiTableFlags_Reorderable |
			//	ImGuiTableFlags_Hideable;

			//if (ImGui::BeginTable("Meshes", 2, flags))
			//{
			//	ImGui::TableSetupColumn("Mesh name", ImGuiTableColumnFlags_WidthFixed);
			//	ImGui::TableSetupColumn("Textures", ImGuiTableColumnFlags_WidthStretch);
			//	ImGui::TableHeadersRow();

			//	for (int i = 0; i < myLoaderModel->GetMeshes().size(); ++i)
			//	{
			//		ImGui::PushID(i);

			//		const auto& loaderMesh = myLoaderModel->GetMeshes()[i];

			//		ImGui::TableNextRow();

			//		for (int column = 0; column < 2; column++)
			//		{
			//			ImGui::TableSetColumnIndex(column);

			//			switch (column)
			//			{
			//				case 0:
			//					ImGui::Text(loaderMesh.GetName().c_str());
			//					break;
			//				case 1:
			//					DrawTextureTable(loaderMesh);
			//					break;
			//				default:
			//					break;
			//			}
			//		}

			//		ImGui::PopID();
			//	}
			//	ImGui::EndTable();
			//}

			ImGui::PushID(&clipEntry);
			if (ImGui::Button("Export"))
			{
				if (myAttachedFbx && myAttachedFbx->IsValid())
				{
					ExportAnimationClipEntry(clipEntry, myAttachedFbx);

					it = myAnimationClips.erase(it);

					//if (myAnimationClips.empty())
					//{
					//	return false;
					//}
				}
			}
			else
			{
				it++;
			}

			ImGui::PopID();

			ImGui::Separator();
		}

		if (myAnimationClips.empty())
		{
			return false;
		}
	}
	else
	{
		ImGui::TextColored(ImVec4(0, 1, 0, 1), "Drag animation FBX here");
	}

	return true;
}

void Engine::AnimationCreationState::ParseFiles(const std::vector<std::filesystem::path>& aPaths)
{
	for (const auto& path : aPaths)
	{
		const std::string extension = StringUtilities::ToLower(path.extension().string());

		if (extension == ".fbx")
		{
			auto animClip = MakeOwned<AnimationClip>();

			if (!animClip->Init(path.string()))
			{
				LOG_ERROR(LogType::Resource) << "Failed to parse the animatino from assimp " << path.string();
				return;
			}

			AnimationClipEntry clipEntry;
			clipEntry.myClip = std::move(animClip);
			clipEntry.myAnimationClipPath = Path(path);

			myAnimationClips.push_back(std::move(clipEntry));
		}
		else if (extension == FileType::Extension_Model)
		{
			myAttachedFbx = GResourceManager->CreateRef<ModelResource>(path.string());
			myAttachedFbx->Load();
		}
		else
		{
			// unknown file extension
		}
	}
}

ImVec4 Engine::AnimationCreationState::GetColorFromPath(const std::string& aPath) const
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

bool Engine::AnimationCreationState::ExportAnimationClipEntry(
	AnimationClipEntry& aClipEntry,
	const ModelRef& aModel)
{
	//AnimationTransformResult bindPose;
	//AnimationMachine::GenerateBindPose(aModel->Get(), *aClipEntry.myClip, bindPose);
	//
	//if (bindPose.myEntries.size() != aClipEntry.myClip->GetBoneNodes().size())
	//{
	//	int test = 0;
	//}

	G6AnimSerializerAnimationClip serializer;

	if (!serializer.Serialize(*aClipEntry.myClip, aModel->Get()))
	{
		LOG_ERROR(LogType::Editor) << "Failed to serialize animation clip";
		return true;
	}

	aClipEntry.myAnimationClipPath.ChangeExtension(FileType::Extension_AnimationClip);

	const auto& buffer = serializer.GetBuffer();

	FileIO::WriteBinaryFile(aClipEntry.myAnimationClipPath, buffer);

	START_TIMER(tim);

	// TRY LOADING IT AFTER WRITING TO ENSURE NO ISSUE OCCURED
	G6AnimDeserializer deserializer;

	if (!deserializer.Deserialize(aClipEntry.myAnimationClipPath))
	{
		LOG_ERROR(LogType::Editor) << "Failed to deserialize: " << aClipEntry.myAnimationClipPath;
	}

	float ms = END_TIMER_GET_RESULT_MS(tim);

	LOG_INFO(LogType::Filip) << "Ms: " << ms;

	return true;
}
