#include "pch.h"
#include "VFXEditorGameObject.h"
#include "imgui_stdlib.h"


// _____________ HELPERS ___________
bool TitleButton(const std::string& title, const std::string& buttonTitle, ImU32 backgroundColor = IM_COL32(90, 20, 145, 155))
{
	ImGui::Separator();
	auto min = ImGui::GetItemRectMin();
	auto max = ImGui::GetItemRectMax();
	min.y += 14.f;
	max.y += 14.f;

	ImGui::GetWindowDrawList()->AddLine(min, max, backgroundColor, 28.f);
	ImGui::Text(title.c_str());
	ImGui::SameLine();

	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 3.f);
	return ImGui::Button(buttonTitle.c_str());

}

void TitleBar(const std::string& title, ImU32 backgroundColor = IM_COL32(128, 0, 128, 155))
{
	ImGui::Separator();
	auto min = ImGui::GetItemRectMin();
	auto max = ImGui::GetItemRectMax();
	min.y += 14.f;
	max.y += 14.f;

	ImGui::GetWindowDrawList()->AddLine(min, max, backgroundColor, 28.f);
	ImGui::Text(title.c_str());
}
// _____________ HELPERS ___________



void PropertyWindow::SetProperty(void* aVariableReference, unsigned aVariableType, EditorEvent* aEvent,
	ePropertyType aPropType)
{
	myVariable = aVariableReference;
	myVariableType = aVariableType;
	myEvent = aEvent;
	myPropertyType = aPropType;
}

void PropertyWindow::ClearProperty()
{
	myPropertyType = ePropertyType::None;
	myVariableType = 0;
	myVariable = nullptr;
	myEvent = nullptr;

}

void PropertyWindow::Draw()
{
	ImGui::Begin("VFX Property");

	if  (myVariable)
	{
		DrawParticle();
	}

	ImGui::End();
}

void PropertyWindow::DrawParticle()
{
	auto particleEnum = static_cast<EditorParticles::SpawnVariables::Enum>(myVariableType);

	switch (particleEnum)
	{
		case EditorParticles::SpawnVariables::Color:
			{
			ImGui::Text("Color");

				auto* color = static_cast<EditorParticles::Color*>(myVariable);
				float col[4];
				col[0] = color->r;
				col[1] = color->g;
				col[2] = color->b;
				col[3] = color->a;
				if (ImGui::ColorPicker4("", col))
				{
					color->r = col[0];
					color->g = col[1];
					color->b = col[2];
					color->a = col[3];
				}

			}
			break;
		case EditorParticles::SpawnVariables::Force:
			{
			ImGui::Text("Force :D");

			}
			break;
		case EditorParticles::SpawnVariables::Space:
			{
			ImGui::Text("Space :D");
				
			}
			break;
		default: ;
	}

}

EditorEvent::EditorEvent()
{
	static int id = 0;
	const auto stringId = std::to_string(id++);
	strcpy_s(myId, 8, stringId.c_str());
}

bool EditorEvent::StartDraw()
{
	ImGui::PushID(myId);
	if (ImGui::CollapsingHeader(myTitle.c_str()))
	{
		ImGui::Indent(16);
		return true;
	}
	ImGui::PopID();
	return false;
}

void EditorEvent::EndDraw()
{
	ImGui::Indent(-16);
	ImGui::PopID();
}

bool EditorEmitter::StartDraw()
{
	if (EditorEvent::StartDraw())
	{
		return true;
	}
	return false;
}

void EditorEmitter::EndDraw()
{
	EditorEvent::EndDraw();
}

bool EditorParticles::StartDraw()
{
	if (EditorEmitter::StartDraw())
	{
		ImGui::Text("Particle system");
		DrawAddVariableSpawn();

		DrawSpawnVariables();

		DrawAddVariableUpdate();
		DrawUpdateVariables();

		return true;
	}
	return false;
}

void EditorParticles::EndDraw()
{
	EditorEmitter::EndDraw();
}

void EditorParticles::DrawAddVariableSpawn()
{
	if (TitleButton("Particle Spawn", "ADD"))
	{
		ImGui::OpenPopup("spawn_add_variable_popup");
	}
	
	if (ImGui::BeginPopup("spawn_add_variable_popup"))
	{
		//TitleBar("Variables");
		//ImGui::Separator();

		for (int i = 0; i < ARRAYSIZE(SpawnVariables::titles); i++)
		{
			if (myStartVariables & (1 << i))
				continue;

			if (ImGui::MenuItem(SpawnVariables::titles[i]))
			{
				AddVariable(static_cast<SpawnVariables::Enum>(1 << i));
			}
		}

		ImGui::EndPopup();
	}

	ImGui::Spacing();
}

void EditorParticles::DrawAddVariableUpdate()
{
	if (TitleButton("Particle Update", "ADD"))
	{
		ImGui::OpenPopup("update_add_variable_popup");
	}

	if (ImGui::BeginPopup("update_add_variable_popup"))
	{
		ImGui::Text("Variables");
		ImGui::Separator();
		for (int i = 0; i < ARRAYSIZE(UpdateVariables::titles); i++)
		{
			if (myUpdateVariables & i)
				continue;

			if (ImGui::Button(UpdateVariables::titles[i]))
			{
				AddVariable(static_cast<UpdateVariables::Enum>(1 << i));
			}
		}

		ImGui::EndPopup();
	}

	ImGui::Spacing();
}

void EditorParticles::DrawSpawnVariables()
{
	for (int i = 0; i < sizeof(unsigned int); i++)
	{
		const auto e = static_cast<SpawnVariables::Enum>(1 << (i));
		if (myStartVariables & e)
		{
			DrawVariable(e);
		}
	}
}

void EditorParticles::DrawUpdateVariables()
{
	for (int i = 0; i < sizeof(unsigned int); i++)
	{
		const auto e = static_cast<UpdateVariables::Enum>(1 << (i));
		if (myUpdateVariables & e)
		{
			DrawVariable(e);
		}
	}
}

void EditorParticles::AddVariable(SpawnVariables::Enum aVariable)
{
	myStartVariables |= aVariable;
}

void EditorParticles::AddVariable(UpdateVariables::Enum aVariable)
{
	myUpdateVariables |= aVariable;
}

void EditorParticles::DrawVariable(SpawnVariables::Enum aVariable)
{
	if (ImGui::Button(SpawnVariables::titles[static_cast<unsigned int>(std::log2(static_cast<unsigned int>(aVariable)))]))
	{
		void* reference = nullptr;
		switch (aVariable)
		{
		case SpawnVariables::Color: 
			reference = &myVariables.color;
			break;
		case SpawnVariables::Force: 
			reference = &myVariables.force;
			break;
		case SpawnVariables::Space: 
			reference = &myVariables.space;
			break;
		default: ;
		}

		assert(reference && "Reference varaible seems to be null! Contact Viktor");

		VFXEditorGameObject::GetPropertyWindow().SetProperty(reference, aVariable, this, PropertyWindow::ParticleSpawn);
	}
	ImGui::SameLine();
	static bool variableActive = true;
	if (ImGui::Checkbox("", &variableActive))
	{
		
	}
}

void EditorParticles::DrawVariable(UpdateVariables::Enum aVariable)
{
	if (ImGui::Button(SpawnVariables::titles[static_cast<unsigned int>(std::log2(static_cast<unsigned int>(aVariable)))]))
	{
		void* reference = nullptr;
		switch (aVariable)
		{
		case UpdateVariables::Color:
			ImGui::Text("Color");
			reference = &myVariables.color;
			break;
		case UpdateVariables::AddForce:
			ImGui::Text("Add Force");
			reference = &myVariables.force;
			break;
		case UpdateVariables::Space:
			ImGui::Text("Space");
			reference = &myVariables.space;
			break;
		default:
			ImGui::Text("ERROR DRAWING VARIABLE");

		}

		assert(reference && "Reference varaible seems to be null! Contact Viktor");

		VFXEditorGameObject::GetPropertyWindow().SetProperty(reference, aVariable, this, PropertyWindow::ParticleSpawn);
	}
	ImGui::SameLine();
	static bool variableActive = true;
	if (ImGui::Checkbox("", &variableActive))
	{

	}
}

void VFXEditorGameObject::AddEvent(EditorEvent* aEvent)
{
	myEvents.emplace_back(aEvent);
}

void VFXEditorGameObject::Update(float aDeltaTime)
{
	bool isOpen = true;

	ImGui::Begin("VFX Editor");

	static float vfxDuration = 1.f;
	ImGui::Text("Duration");
	ImGui::SameLine();
	ImGui::InputFloat("", &vfxDuration);

	// Add Emitter
	{
		if (ImGui::Button("Add Event"))
			ImGui::OpenPopup("add_event_popup");
	}

	DrawAddEventPopup();

	RenderEvents();

	ImGui::End();

	GetPropertyWindow().Draw();

	ImGui::ShowDemoWindow(&isOpen);
}

void VFXEditorGameObject::RenderEvents()
{
	for (auto& e : myEvents)
	{
		if (e->StartDraw())
		{
			e->EndDraw();
		}
	}
	ImGui::Indent(0);
}

void VFXEditorGameObject::DrawAddEventPopup()
{
	if (ImGui::BeginPopup("add_event_popup"))
	{
		ImGui::Text("Events");
		ImGui::Separator();

		if (ImGui::BeginMenu("Emitters.."))
		{
			if (ImGui::MenuItem("Particles"))
			{
				AddEvent(DBG_NEW EditorParticles());
			}

			if (ImGui::MenuItem("Physic Particles"))
			{

			}

			if (ImGui::MenuItem("Objects"))
			{

			}

			ImGui::EndMenu();
		}

		ImGui::MenuItem("Action");

		ImGui::EndPopup();
	}
}

PropertyWindow& VFXEditorGameObject::GetPropertyWindow()
{
	static PropertyWindow prop;
	return prop;
}
