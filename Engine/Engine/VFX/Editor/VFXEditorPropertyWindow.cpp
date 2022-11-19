#include "pch.h"
#include "VFXEditorPropertyWindow.h"

void VFXEditorPropertyWindow::Tick()
{
	ImGui::Begin("VFX Property");

	if (myPropertyDrawFunction)
		myPropertyDrawFunction();

	ImGui::End();
}
 
void VFXEditorPropertyWindow::Set(const std::function<void()>& aPropertyDrawFunction)
{
	myPropertyDrawFunction = aPropertyDrawFunction;
}

void VFXEditorPropertyWindow::Clear()
{
	myPropertyDrawFunction = nullptr;
}

VFXEditorPropertyWindow& VFXEditorPropertyWindow::Instance()
{
	static VFXEditorPropertyWindow propertyWindow;
	return propertyWindow;
}
