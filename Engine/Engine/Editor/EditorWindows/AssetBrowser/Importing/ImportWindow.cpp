#include "pch.h"
#include "ImportWindow.h"
#include "CreationStates\CreationState.h"
#include "Engine\Editor\Editor.h"
#include "Engine\Engine.h"
#include "Engine\Renderer\GraphicsEngine.h"
#include "CreationStates\ModelImportState.h"
#include "CreationStates\AnimationCreationState.h"

Engine::ImportWindow::ImportWindow(Editor& aEditor)
	: EditorWindow(aEditor, Flags::CanBeClosed)
{
	GetEngine().GetGraphicsEngine().GetWindowHandler().AddObserver(EventType::DraggedFileIntoWindow, this);
}

Engine::ImportWindow::~ImportWindow()
{
	GetEngine().GetGraphicsEngine().GetWindowHandler().RemoveObserver(EventType::DraggedFileIntoWindow, this);
}

void Engine::ImportWindow::EnterImportState(const ResourceImportState& aCreationState, const std::filesystem::path& aPath)
{
	myCreationStateType = aCreationState;

	switch (aCreationState)
	{
		case ResourceImportState::Model:
			myImportState = MakeOwned<ModelImportState>(aPath, myEditor);
			break;
		case ResourceImportState::AnimationClip:
			myImportState = MakeOwned<AnimationCreationState>(aPath, myEditor);
			break;
		case ResourceImportState::None:
			myImportState = nullptr;
			break;
		default:
			assert(false);
			break;
	}
}

void Engine::ImportWindow::EnterImportState(const ResourceImportState& aCreationState)
{
	myCreationStateType = aCreationState;

	switch (aCreationState)
	{
		case ResourceImportState::Model:
			myImportState = MakeOwned<ModelImportState>();
			break;
		case ResourceImportState::AnimationClip:
			myImportState = MakeOwned<AnimationCreationState>(myEditor);
			break;
		case ResourceImportState::None:
			myImportState = nullptr;
			break;
		default:
			assert(false);
			break;
	}
}

std::string Engine::ImportWindow::GetName() const
{
	return "Import";
}

void Engine::ImportWindow::Receive(const EventType aEventType, const std::any& aValue)
{
	if (aEventType == EventType::DraggedFileIntoWindow)
	{
		const auto stringPaths = std::any_cast<std::vector<std::string>>(aValue);

		if (myImportState && !stringPaths.empty())
		{
			std::vector<std::filesystem::path> paths;

			for (const auto& pathString : stringPaths)
			{
				paths.push_back(std::filesystem::path(pathString));
			}

			myImportState->ParseFiles(paths);
		}
	}
}

void Engine::ImportWindow::Draw(const float aDeltaTime)
{
	ImGui::Dummy(ImVec2(0, 1));
	ImGui::Separator();
	ImGui::Dummy(ImVec2(0, 1));

	if (ImGui::BeginChild("##importfbxstate"))
	{
		if (myImportState)
		{
			if (!myImportState->Draw())
			{
				Close();
			}
		}
	}
	ImGui::EndChild();
}

