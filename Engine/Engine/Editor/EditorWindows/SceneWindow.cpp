#include "pch.h"
#include "SceneWindow.h"

#include "Engine/Engine.h"
#include "Engine/Editor/Editor.h"
#include "Engine/GameObject/Components/EditorCameraMovement.h"
#include "Engine/Scene/FolderScene.h"
#include "LogWindow.h"
#include "Engine/Renderer/GraphicsEngine.h"
#include "Engine/Editor/ImGuiHelper.h"
#include "Common/Random.h"
#include "Engine/Renderer/Texture/Texture2D.h"
#include "Engine/ResourceManagement/Resources/TextureResource.h"
#include "Engine/Renderer/RenderManager.h"
#include "Engine/Scene/SceneManager.h"
#include "Engine/GameObject/GameObject.h"

Engine::SceneWindow::SceneWindow(
	Editor& aEditor,
	ResourceReferences& aResourceReferences)
	: EditorWindow(aEditor, Flags::CanBeUnsaved)
{
	std::string welcomeMessages[] =
	{
		"i cannot think of anything fun to write",
		"help me",
		"bootleg unity editor",
		"faster than unreal",
		"Google Chrome",
		"russian dolls are so full of themselves",
		"gray hair gang",
		"dog",
		"im so fresh you can...",
		"aka get money engine",
		"Work hard get money, #truth",
		"despacito",
		"manekmabackmapoosiandmakrack",
		"whiteboards are remarkable",
		"gucci hang",
		"snyggt byggt, frasig karra",
		"\"It just works\" - Todd Howard",
		"Sir, this is a wingdings...",
		"Add me on LinkedIn",
		"Nuvarande betyg: IG",
		"Nuvarande betyg: VG",
		"Resolution <OFF>",
		"Is it fika time yet?",
		"Powered by Dog",
		"Dont forget to burndown",
		"Risk of Pain",
		"Risk of Fart",
		"Risk for overscope",
		"Det e lungt mannen",
		"Pet dogs",
		"I love my Dog",
		"Barely running",
		"gucci brorsan",
		"Smarrigt",
		"Suspenders are obligatory on fridays",
		"Dont forget to BURNDOWN",
		"HAHAHA",
		"Cringe mannen",
		"Tack for att du laste detta",
		"Runtime scene save = Save prefab!",
		"Big error, big booty",
		"Priolistariteringslistan",
		"No nuts in the classroom!",
		"Potato?"
	};

	myWelcomeMessage = welcomeMessages[Random::RandomInt(0, std::size(welcomeMessages) - 1)];

	myEditorTexture = GResourceManager->CreateRef<TextureResource>(
		"Assets/Engine/Editor/ShrekMike.dds");

	myEditorPics.push_back(GResourceManager->CreateRef<TextureResource>(
		"Assets/Engine/Editor/ShrekMike.dds"));

	myEditorPics.push_back(GResourceManager->CreateRef<TextureResource>(
		"Assets/Engine/Editor/Bjorn.dds"));

	myEditorPics.push_back(GResourceManager->CreateRef<TextureResource>(
		"Assets/Engine/Editor/CogDog.dds"));

	myEditorPics.push_back(GResourceManager->CreateRef<TextureResource>(
		"Assets/Engine/Editor/Compiles.dds"));

	myEditorPics.push_back(GResourceManager->CreateRef<TextureResource>(
		"Assets/Engine/Editor/David.dds"));

	myEditorPics.push_back(GResourceManager->CreateRef<TextureResource>(
		"Assets/Engine/Editor/Pontus.dds"));

	myEditorPics.push_back(GResourceManager->CreateRef<TextureResource>(
		"Assets/Engine/Editor/Pontus2.dds"));

	myEditorPics.push_back(GResourceManager->CreateRef<TextureResource>(
		"Assets/Engine/Editor/William.dds"));

	myEditorPics.push_back(GResourceManager->CreateRef<TextureResource>(
		"Assets/Engine/Editor/DoItForHer.dds"));

	myEditorPics.push_back(GResourceManager->CreateRef<TextureResource>(
		"Assets/Engine/Editor/Filip.dds"));

	myEditorPics.push_back(GResourceManager->CreateRef<TextureResource>(
		"Assets/Engine/Editor/Jacky.dds"));

	myEditorPics.push_back(GResourceManager->CreateRef<TextureResource>(
		"Assets/Engine/Editor/Hello.dds"));

	myEditorPics.push_back(GResourceManager->CreateRef<TextureResource>(
		"Assets/Engine/Editor/FuckYou.dds"));

	myEditorPics.push_back(GResourceManager->CreateRef<TextureResource>(
		"Assets/Engine/Editor/Tommy.dds"));

	myEditorPics.push_back(GResourceManager->CreateRef<TextureResource>(
		"Assets/Engine/Editor/WingDings.dds"));

	myEditorPics.push_back(GResourceManager->CreateRef<TextureResource>(
		"Assets/Engine/Editor/NoFace.dds"));

	myEditorPics.push_back(GResourceManager->CreateRef<TextureResource>(
		"Assets/Engine/Editor/BlamePontus.dds"));

	myEditorPics.push_back(GResourceManager->CreateRef<TextureResource>(
		"Assets/Engine/Editor/buddies.dds"));

	myEditorPics.push_back(GResourceManager->CreateRef<TextureResource>(
		"Assets/Engine/Editor/godis.dds"));

	myEditorPics.push_back(GResourceManager->CreateRef<TextureResource>(
		"Assets/Engine/Editor/grab.dds"));

	myEditorPics.push_back(GResourceManager->CreateRef<TextureResource>(
		"Assets/Engine/Editor/kings.dds"));

	myEditorPics.push_back(GResourceManager->CreateRef<TextureResource>(
		"Assets/Engine/Editor/Pathetic.dds"));

	myEditorPics.push_back(GResourceManager->CreateRef<TextureResource>(
		"Assets/Engine/Editor/pog.dds"));


	myEditorTexture = myEditorPics[Random::RandomInt(0, std::size(myEditorPics) - 1)];
}

void Engine::SceneWindow::Draw(const float aDeltaTime)
{
	ZoneNamedN(zone1, "SceneWindow::Update", true);

	if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
	{
		if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F), false))
		{
			myEditor.MoveEditorCamToSelection();
		}
	}

	if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
	{
		ImGui::SetNextWindowFocus();
	}

	if (ImGui::BeginChild("##SceneDropField", ImGui::GetContentRegionAvail(), false, 0))
	{
		FolderScene* scene = myEditor.GetActiveScene();

		//ImGui::CaptureKeyboardFromApp(false);
		// ImGui::GetIO().WantCaptureMouse = false;

		if (scene != nullptr)
		{
			DrawPlayStopButton();

			ImGui::SameLine();

			DrawPauseResumeButton();

			ImGui::SameLine();

			DrawSceneRenderDebugOutputCombo();

			ImGui::SameLine();

			DrawCameraSpeed();

			ImGui::SameLine();

			DrawGridCheckbox();

			ImGui::SameLine();

			float snapping = myEditor.GetActiveScene2().GetSnapping();
			ImGui::SetNextItemWidth(150.f);
			if (ImGui::DragFloat("Snapping", &snapping, 1.f, 0.f, 1000.f))
			{
				myEditor.GetActiveScene2().SetSnapping(snapping);
			}

			// Must update it because, DrawPlayStopButton()
			// can possible delete the scene pointer and it becomes invalid
			scene = myEditor.GetActiveScene();

			// Calculate the image size with the aspect ratio
			const float aspectRatio = GetEngine().GetWindowSize().y / GetEngine().GetWindowSize().x;

			ImVec2 calcedSize{ ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().x * aspectRatio };

			D3D11_TEXTURE2D_DESC desc;
			scene->GetRendererScene().GetResultOutputTexture().GetTextureTest()->GetDesc(&desc);

			const auto rendererSceneDebugOutput = scene->GetRendererScene().GetRendererDebugOutput();

			if (rendererSceneDebugOutput == RendererSceneDebugOutput::Nothing)
			{
				// Draw imgui scene texture
				ImGui::Image(scene->GetRendererScene().GetResultOutputTexture().GetSRV(),
					calcedSize);
			}
			else
			{
				ImGui::Image(scene->GetRendererScene().GetDebugGBufferTexture().GetSRV(),
					calcedSize);
			}

			//{
			//	// Draw recent log in bottom right corner
			//	const auto& lines = myEditor.GetLogWindow().GetLines();
			//
			//	const int maxRecentMessages = 1;
			//
			//	for (int i = 0; i < maxRecentMessages; ++i)
			//	{
			//		auto drawList = ImGui::GetWindowDrawList();
			//		drawList->AddText(
			//			ImVec2(ImGui::GetItemRectMin().x, ImGui::GetItemRectMax().y),
			//			IM_COL32(255, 0, 0, 255),
			//			lines[i].myLine.c_str());
			//	}
			//}

			// ImGui::SaveIniSettingsToDisk// 

			myWindowItemRectMin = Vec2f(ImGui::GetItemRectMin().x, ImGui::GetItemRectMin().y);
			myWindowItemRectSize = Vec2f(ImGui::GetItemRectSize().x, ImGui::GetItemRectSize().y);

			auto windowPosRelativeToWindow = ImGui::GetItemRectMin();

			// When we have virtual viewports enabled, the coordinate system is changed
			// therefore, convert screen space coordinate to make it relative to window
			if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				windowPosRelativeToWindow.x -= ImGui::GetMainViewport()->Pos.x;
				windowPosRelativeToWindow.y -= ImGui::GetMainViewport()->Pos.y;
			}

			myWindowPosition = { windowPosRelativeToWindow.x, windowPosRelativeToWindow.y };
			myWindowSize = { myWindowItemRectSize.x, myWindowItemRectSize.y };

			ImVec2 imagePos = ImGui::GetItemRectMin();
			ImVec2 mousePos = ImGui::GetMousePos();
			Vec2f mouseViewportRelative{ mousePos.x - imagePos.x, mousePos.y - imagePos.y };

			// do not let us change gizmos stuff when flying to avoid keybind collisions
			if (!myEditor.GetEditorCamMovement().IsFlying() && ImGui::IsWindowFocused() && myEditor.GetMode() == ::Engine::EditorMode::Editing)
			{
				myEditor.GetActiveScene2().UpdateGizmosKeyBinds();

				if (ImGui::IsWindowFocused() && ImGui::IsMouseReleased(0) && !ImGuizmo::IsUsing())
				{
					Vec2f normalizedCoordinates{ mouseViewportRelative.x / calcedSize.x, mouseViewportRelative.y / calcedSize.y };

					bool isInsideViewport =
						normalizedCoordinates.x >= 0.f &&
						normalizedCoordinates.x <= 1.f &&
						normalizedCoordinates.y >= 0.f &&
						normalizedCoordinates.y <= 1.f;

					if (isInsideViewport)
					{
						UUID uuid = scene->GetRendererScene().GetUUIDFromSelectionTexture(normalizedCoordinates);

						if (static_cast<int32_t>(uuid) > 0)
						{
							const Shared<GameObject>& sharedGameObject = scene->FindGameObject(uuid)->GetWeak().lock();
							myEditor.PushCommand(myEditor.CreateCommand_SelectGameObjects({ sharedGameObject }));
							myEditor.FinishCommand();
						}
						else
						{
							myEditor.PushCommand(myEditor.CreateCommand_SelectGameObjects({}));
							myEditor.FinishCommand();
						}
					}
				}
			}

			myEditor.GetActiveScene2().Update();

			// Call the editor execute selected
			{
				auto& selection = myEditor.GetActiveScene2().GetSelection();

				for (int i = 0; i < selection.GetCount(); ++i)
				{
					if (GameObject* g = selection.GetAt(i))
					{
						for (auto& c : g->myComponents)
						{
							c->EditorExecuteSelected();
						}
					}
				}
			}

			// Must be called after EditorExecuteSelected()
			// because in that function we can create a gizmos, and
			// then an issue would arise because we would have drawn this gizmos before
			// drawing that gizmos
			myEditor.GetActiveScene2().UpdateGizmos(
				myWindowItemRectMin.x,
				myWindowItemRectMin.y,
				myWindowItemRectSize.x,
				myWindowItemRectSize.y);

			// auto p = myEditor.GetActiveScene2().GetScene()->GetMousePosRelativeToSceneWindowNormalized();
			// LOG_INFO(LogType::Filip) << "x: " << p.x << ", y: " << p.y;
		}
		else
		{
			DrawWelcomeWidget();
		}
	}
	ImGui::EndChild();

	// Make the imgui window that shows the scene
	// ignore the inputs so we can interact with game
	if (ImGui::IsItemHovered())
	{
		ImGui::GetIO().WantCaptureMouse = false;
		ImGui::GetIO().WantCaptureKeyboard = false;
	}
}

const Vec2f& Engine::SceneWindow::GetWindowPosition() const
{
	return myWindowPosition;
}

const Vec2f& Engine::SceneWindow::GetWindowSize() const
{
	return myWindowSize;
}

const Vec2f& Engine::SceneWindow::GetWindowItemRectMin() const
{
	return myWindowItemRectMin;
}

const Vec2f& Engine::SceneWindow::GetWindowItemRectSize() const
{
	return myWindowItemRectSize;
}

void Engine::SceneWindow::DrawPlayStopButton()
{
	switch (myEditor.GetMode())
	{
	case EditorMode::Editing:
	{
		if (ImGui::Button("Play"))
		{
			myEditor.ChangeMode(EditorMode::Playing);
		}

		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("Play | F5");
		}
	}
	break;

	case EditorMode::Playing:
	{
		if (ImGui::Button("Stop"))
		{
			myEditor.ChangeMode(EditorMode::Editing);
		}

		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("Stop | F5");
		}
	}
	break;

	default:
		break;
	}
}

void Engine::SceneWindow::DrawPauseResumeButton()
{
	if (Scene* runtimeScene = myEditor.GetRuntimeScene())
	{
		if (runtimeScene->ShouldUpdate())
		{
			if (ImGui::Button("Pause"))
			{
				runtimeScene->SetShouldUpdate(false);
			}
		}
		else
		{
			if (ImGui::Button("Resume"))
			{
				runtimeScene->SetShouldUpdate(true);
			}
		}
	}
}

void Engine::SceneWindow::DrawRecentScenes()
{
	static ImGuiTableFlags flags =
		ImGuiTableFlags_SizingFixedFit |
		ImGuiTableFlags_Borders |
		ImGuiTableFlags_Resizable |
		ImGuiTableFlags_Reorderable |
		ImGuiTableFlags_Hideable;

	if (ImGui::BeginTable("debugdrawflags", 1, flags))
	{
		ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);

		auto& recentScenes = myEditor.GetRecentScenes();

		for (size_t i = 0; i < recentScenes.size(); ++i)
		{
			ImGui::PushID(i);

			ImGui::TableNextRow();

			for (int column = 0; column < 1; column++)
			{
				ImGui::TableSetColumnIndex(column);

				switch (column)
				{
				case 0:
				{
					ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));
					if (ImGui::Selectable(
						recentScenes[i].ToString().c_str(),
						i == mySelectedRecentSceneIndex,
						0,
						ImVec2(0.f, 30.f)))
					{
						mySelectedRecentSceneIndex = i;
					}

					if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemHovered())
					{
						myEditor.OpenScene(recentScenes[i]);
					}
					ImGui::PopStyleVar();
				}
				break;
				default:
					assert(false);
					break;
				}
			}

			ImGui::PopID();
		}
		ImGui::EndTable();
	}
}

void Engine::SceneWindow::DrawWelcomeWidget()
{
	const auto avail = ImGui::GetContentRegionAvail();

	const ImVec2 welcomeChildSize(500.f, 500.f);

	const auto pos = ImGui::GetCursorPos();
	const auto halfWelcomeSize = ImVec2(
		(avail.x - welcomeChildSize.x) * 0.5f,
		(avail.y - welcomeChildSize.y) * 0.5f);

	ImGui::SetCursorPos(ImVec2(pos.x + halfWelcomeSize.x, pos.y + halfWelcomeSize.y));

	if (ImGui::BeginChild("##welcome", welcomeChildSize, true))
	{
		ImGui::Image(myEditorTexture->Get().GetSRV(), ImVec2(welcomeChildSize.x, 250.f));

		ImGui::PushFont(GraphicsEngine::ourFont24);
		ImGuiHelper::TextCentered(myWelcomeMessage);
		ImGui::PopFont();

		DrawRecentScenes();

		ImGui::EndChild();
	}
}

void Engine::SceneWindow::DrawCameraSpeed()
{
	auto& basicMovement = myEditor.GetEditorCamMovement();

	ImGui::SetNextItemWidth(150.f);
	float speed = basicMovement.GetSpeed();
	if (ImGui::SliderFloat("CamSpeed", &speed, 5.f, 5000.f))
	{
		basicMovement.SetSpeed(speed);
	}
}

void Engine::SceneWindow::DrawGridCheckbox()
{
	ImGui::Checkbox("Grid", &myEditor.GetActiveScene2().myDrawGrid);
}

void Engine::SceneWindow::DrawSceneRenderDebugOutputCombo()
{
	FolderScene* scene = myEditor.GetActiveScene();

	const auto rendererSceneDebugOutput = scene->GetRendererScene().GetRendererDebugOutput();

	ImGui::SetNextItemWidth(120);
	if (ImGui::BeginCombo("##renderdebugoutput", RendererSceneDebugOutputStrings[static_cast<int>(rendererSceneDebugOutput)].c_str()))
	{
		for (int i = 0; i < static_cast<int>(RendererSceneDebugOutput::Count); ++i)
		{
			if (ImGui::Selectable(RendererSceneDebugOutputStrings[i].c_str(), static_cast<int>(rendererSceneDebugOutput) == i))
			{
				scene->GetRendererScene().SetRendererDebugOutput(static_cast<RendererSceneDebugOutput>(i));
			}
		}

		ImGui::EndCombo();
	}
}