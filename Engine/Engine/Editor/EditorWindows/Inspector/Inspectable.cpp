#include "pch.h"
#include "Inspectable.h"
#include "InspectorWindow.h"

Engine::Inspectable::Inspectable(Editor& aEditor, InspectorWindow& aInspectorWindow) : myEditor(aEditor),
myInspectorWindow(aInspectorWindow)
{

}

Engine::Inspectable::~Inspectable()
{

}

void Engine::Inspectable::Draw()
{
	ImGui::TextColored(ImVec4(0, 1, 0, 1), "Inspecting: %s", GetInspectingType().c_str());

	ImGui::Separator();
}

void Engine::Inspectable::Save()
{
}
