#include "pch.h"
#include "InspectableSelection.h"
#include "Engine\Reflection\ImguiReflector.h"
#include "Engine\GameObject\GameObject.h"
#include "Engine\Editor\ImGuiHelper.h"
#include "Engine\Editor\Editor.h"
#include "Engine\Scene\FolderScene.h"

Engine::InspectableSelection::InspectableSelection(
	Editor& aEditor,
	InspectorWindow& aInspectorWindow,
	const Selection& aSelection)
	: Inspectable(aEditor, aInspectorWindow),
	myInspectGameObject(aEditor, false, { }),
	mySelection(&aSelection)
{
}

void Engine::InspectableSelection::Draw()
{
	Inspectable::Draw();

	auto& selection = myEditor.GetActiveScene2().GetSelection();

	if (selection.GetCount() == 1)
	{
		auto gameObject = selection.GetAtWeak(0);

		if (auto g = gameObject.lock())
		{
			myInspectGameObject.DrawInspectedGameObject(*myEditor.GetActiveScene(), g);
		}
	}
	else
	{
		// If multiple gameobjects are selected
		// only draw a transform so we can transform them all together
	}

	if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootWindow))
	{
		// UpdateKeypresses();
	}
}
