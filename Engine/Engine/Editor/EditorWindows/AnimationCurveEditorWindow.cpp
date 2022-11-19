#include "pch.h"
#include "AnimationCurveEditorWindow.h"
#include "Engine\Editor\Editor.h"

Engine::AnimationCurveEditorWindow::AnimationCurveEditorWindow(Editor& aEditor) 
	: EditorWindow(aEditor, Flags::CanBeClosed | Flags::CanBeUnsaved)
{
}

bool Engine::AnimationCurveEditorWindow::Init(const AnimationCurveRef& aCurve)
{
	myCurveEditor.Init(aCurve);

	return true;
}

void Engine::AnimationCurveEditorWindow::Draw(const float aDeltaTime)
{
	myCurveEditor.Draw();
}
