#include "pch.h"
#include "VFXEditorEvent.h"

bool VFXEditorEvent::StartDraw()
{
	ImGui::PushID(myId);
	if (ImGui::CollapsingHeader(myTitle.c_str()))
	{
		ImGui::Indent(16);
		DrawEventHeader();

		return true;
	}
	ImGui::PopID();
	return false;
}

void VFXEditorEvent::EndDraw()
{
	ImGui::Indent(-16);
	ImGui::PopID();
}

void VFXEditorEvent::DrawEventHeader()
{
	if (ImGui::Button("Remove Event"))
	{
		myIsDeleted = true;
	}
	ImGui::Spacing();

	ImGui::InputText("Title", &myTitle);
	ImGui::InputFloat("Spawn Time", &myKey.time, 0.f);
}

eEditorEventType VFXEditorEvent::GetType() const
{
	return myType;
}

bool VFXEditorEvent::GetDelete() const
{
	return myIsDeleted;
}

void VFXEditorEvent::Setup()
{
	static int id = 0;
	const auto stringId = std::to_string(id++);
	strcpy_s(myId, 8, stringId.c_str());
}
