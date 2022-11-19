#include "pch.h"
#include "VisualScriptWindow.h"

#include "Engine/GraphManager/GraphInstance.h"
#include "Engine/GraphManager/GraphManager.h"
#include "Engine/Editor/Editor.h"

Engine::VisualScriptWindow::VisualScriptWindow(Editor& aEditor)
	: EditorWindow(aEditor, Flags::CanBeClosed | Flags::CanBeUnsaved)
{
}

Engine::VisualScriptWindow::~VisualScriptWindow()
{
}

bool Engine::VisualScriptWindow::InitExistingInstance(GraphInstance* aGraphInstance, const Path& aPath)
{
	myGraphManager = MakeOwned<GraphManager>(*this, aGraphInstance, aPath);
	myGraphManager->Load();

	return true;
}

bool Engine::VisualScriptWindow::InitNewInstance(Owned<GraphInstance> aInstance, const Path& aSavePath)
{
	myInstance = std::move(aInstance);

	myGraphManager = MakeOwned<GraphManager>(*this, myInstance.get(), aSavePath);
	myGraphManager->Load();

	return true;
}

void Engine::VisualScriptWindow::Draw(const float aDeltaTime)
{
	ZoneNamedN(zone1, "NodeGraphWindow::Update", true);

	myGraphManager->PreFrame(aDeltaTime);

	myGraphManager->ConstructEditorTreeAndConnectLinks();

	myGraphManager->PostFram();

	// ImGui::End();
}

ImGuiWindowFlags Engine::VisualScriptWindow::GetWindowFlags() const
{
	return ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse |
		ImGuiWindowFlags_NoBringToFrontOnFocus;
}

void Engine::VisualScriptWindow::OnEditorChangeMode(const EditorMode aMode)
{
	if (aMode == EditorMode::Editing)
	{
		// Must close this window if we go into Editing mode and we have an instance open
		if (myGraphManager->IsEditingInstance())
		{
			myEditor.CloseExtraWindow(GetName());
		}
	}
}
