#include "pch.h"
#include "VFXEditor.h"

#include "VFXEditorParticlesEvent.h"
#include "Engine/Engine.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/VFXComponent.h"
#include "Engine/Scene/Scene.h"
#include "VFXEditorHelpers.h"
#include "VFXEditorPropertyWindow.h"
#include "Engine/VFX/VFXUtilities.h"
#include "Engine/Editor/Editor.h"
#include "../Flatbuffer/vfxsystem_generated.h"
#include "Common/FileIO.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"
#include "Engine/Scene/FolderScene.h"

Engine::VFXEditor::VFXEditor()
{
}

void Engine::VFXEditor::Init()
{
	myGameObject = nullptr;
	CreateDummyObject();
}

void Engine::VFXEditor::CreateDummyObject()
{
	if (myGameObject)
		myGameObject->Destroy();

	myGameObject = GetEngine().GetEditor().GetActiveScene()->AddGameObject<GameObject>();
	myGameObject->SetName("VFX Editor Object");
}

void Engine::VFXEditor::Tick()
{
	ImGui::Begin("VFX Editor");
	DrawHeader();
	DrawAddEvent();

	RenderEvents();

	DrawFooter();
	ImGui::End();

	if (GetEngine().GetInputManager().IsKeyPressed(Common::KeyCode::LeftControl))
	{
		if (GetEngine().GetInputManager().IsKeyDown(Common::KeyCode::S))
		{
			Save();
		}
		
	}

	// Draw VFX position
	GDebugDrawer->DrawCircle3D(DebugDrawFlags::Particles, { 0, 0, 0 }, 100, 0, Vec4f(1,.3f,.3f,1));
}

void Engine::VFXEditor::CreateSystem()
{
	auto obj = GetScene()->AddGameObject<GameObject>();

	const auto& desc = GetDescription();
	auto* vfx = obj->AddComponent<VFXComponent>(desc);
	vfx->Play();
}

void Engine::VFXEditor::Save()
{
	const auto& description = GetDescription();
	VFX::Save(description);
	if (myVFXRef && myVFXRef->IsValid())
		myVFXRef->Load();

	if (mySaveObserver)
		mySaveObserver();
}

void Engine::VFXEditor::Load(const std::string& aFilePath)
{
	VFXEditorPropertyWindow::Instance().Clear();

	CreateDummyObject();
	myDescription = VFX::Load(aFilePath);
	RegenerateEditor();
}

void Engine::VFXEditor::ReloadResource()
{
	if (myVFXRef && myVFXRef->IsValid())
	{
		myVFXRef->Unload();
		myVFXRef->Load();
	}
}

void Engine::VFXEditor::ObserveSave(const std::function<void()>& aObserver)
{
	mySaveObserver = aObserver;
}

const VFX::VFXDescription& Engine::VFXEditor::GetDescription()
{
	myDescription.particleEmitters.Clear();
	myDescription.prefabEmitters.Clear();

	for (int i = 0; i < myParticleEmitterEvents.size(); i++)
	{
		auto& emitter = myParticleEmitterEvents[i];
		const auto& key = emitter->GetKey();
		switch (emitter->GetType())
		{
		case eEditorEventType::ParticleEmitter:
			{
				auto* particlesEvent = reinterpret_cast<VFXEditorParticlesEvent*>(emitter);

				const VFX::ParticleEmitterDescription& desc = particlesEvent->GetDescription();
				myDescription.Add(desc, key);
			}
			break;
		case eEditorEventType::PhysicsParticleEmitter: break;
		case eEditorEventType::PhysicsObjectEmitter: break;
		default: ;
		}
	}
	return myDescription;
}
//
//VFXRef Engine::VFXEditor::GetVFXRef()
//{
//	return myVFXRef;
//}

GameObject* Engine::VFXEditor::GetGameObject()
{
	return myGameObject;
}

Engine::VFXEditor& Engine::VFXEditor::GetInstance()
{
	static VFXEditor instance;
	return instance;
}

void Engine::VFXEditor::RegenerateEditor()
{
	DeleteEvents();

	for (auto& key : myDescription.particleEmitters.GetKeys())
	{
		AddParticleEmitter(key.data, key);
	}
}

void Engine::VFXEditor::AddParticleEmitter()
{
 	VFXEditorParticlesEvent* editorEvent = DBG_NEW VFXEditorParticlesEvent();
	myParticleEmitterEvents.emplace_back(editorEvent);
}

void Engine::VFXEditor::AddPrefabEmitter()
{
	VFXEditorParticlesEvent* editorEvent = DBG_NEW VFXEditorParticlesEvent();
	myParticleEmitterEvents.emplace_back(editorEvent);
}

void Engine::VFXEditor::AddParticleEmitter(const VFX::ParticleEmitterDescription& aDescription, const Timeline::Key& aKey)
{
	VFXEditorParticlesEvent* editorEvent = DBG_NEW VFXEditorParticlesEvent();
	myParticleEmitterEvents.emplace_back(editorEvent);
	editorEvent->SetKey(aKey);
	editorEvent->SetDescription(aDescription);
}

void Engine::VFXEditor::DrawHeader()
{
	VFXEditorHelpers::TitleBar("VFX EDITOR");

	ImguiReflector myImguiReflector = ImguiReflector(GetEngine().GetEditor(), nullptr, nullptr);

	if (myImguiReflector.Reflect(myVFXRef, "VFX") & ReflectorResult_Changed)
	{
		myVFXRef->Load();
		Load(myVFXRef->GetPath());
	}

	ImGui::Spacing();
	if (ImGui::Button("Save"))
	{
		Save();
	}
	//ImGui::SameLine();
	/*if (ImGui::Button("Load"))
	{
		const auto paths = FileIO::BrowseFileDialog({ L"vfx" }, L"vfx");
		assert(paths.size() <= 1);

		if (!paths.empty())
		{
			Load(paths[0].ToString());
		}
	}*/
	ImGui::SameLine();
	if (ImGui::Button("New"))
	{
		VFXEditorPropertyWindow::Instance().Clear();
		myDescription = VFX::VFXDescription();
		RegenerateEditor();
	}

	//if (ImGui::Button("Play"))
	//{
	//	CreateSystem();
	//}

	ImGui::Spacing();

	ImGui::InputText("Title", &myDescription.title);
	ImGui::InputFloat("Duration", &myDescription.duration);
	ImGui::Checkbox("Looping", &myDescription.looping);
	VFXEditorHelpers::DrawSpace(myDescription.space);
}

void Engine::VFXEditor::DrawFooter()
{
}

void Engine::VFXEditor::RenderEvents()
{
	for (int i = 0; i < myParticleEmitterEvents.size(); i++)
	{
		auto& e = myParticleEmitterEvents[i];
		if (e->StartDraw())
			e->EndDraw();
	}
}

void Engine::VFXEditor::DrawAddEvent()
{
	if (ImGui::Button("Add Event"))
		ImGui::OpenPopup("add_event_popup");

	if (ImGui::BeginPopup("add_event_popup"))
	{
		ImGui::Text("Events");
		ImGui::Separator();

		if (ImGui::BeginMenu("Emitters.."))
		{
			if (ImGui::MenuItem("Particles"))
			{
				AddParticleEmitter();
			}

			if (ImGui::MenuItem("Prefab"))
			{
				AddPrefabEmitter();
			}

			ImGui::EndMenu();
		}

		//ImGui::MenuItem("Action");

		ImGui::EndPopup();
	}
}

void Engine::VFXEditor::DeleteEvents()
{
	for (int i = myParticleEmitterEvents.size() - 1; i >= 0; i--)
	{
		delete myParticleEmitterEvents[i];
		myParticleEmitterEvents[i] = nullptr;
	}
	myParticleEmitterEvents.clear();
}
