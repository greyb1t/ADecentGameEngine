#include "pch.h"
#include "VFXEditorScene.h"

#include "VFXParticleAttraction.h"
#include "Engine/Shortcuts.h"
#include "Engine/GameObject/Components/VFXComponent.h"
//#include "Engine/Reflection/ImguiReflector.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"
#include "Engine/VFX/VFXUtilities.h"
#include "Engine/VFX/Editor/VFXEditorPropertyWindow.h"

Engine::VFXEditorScene::VFXEditorScene()
{
}

Engine::Scene::Result Engine::VFXEditorScene::OnRuntimeInit()
{
	VFXEditor::GetInstance().Init();

	VFXEditor::GetInstance().ObserveSave([&](){ Play(); });

	return FolderScene::OnRuntimeInit();

}

Engine::VFXEditorScene* Engine::VFXEditorScene::Clone() const
{
	return DBG_NEW VFXEditorScene(*this);
}

void Engine::VFXEditorScene::Play()
{
	VFXEditor::GetInstance().ReloadResource();
	if (auto obj = vfxObj.lock())
	{
		obj->Destroy();
	}

	vfxObj = AddGameObject<GameObject>()->GetWeak();

	auto desc = VFXEditor::GetInstance().GetDescription();
	auto* vfx = vfxObj.lock()->AddComponent<VFXComponent>(desc);
	vfx->Play();
}

void Engine::VFXEditorScene::Update(TimeStamp ts)
{
	FolderScene::Update(ts);

	VFXEditor::GetInstance().Tick();
	VFXEditorPropertyWindow::Instance().Tick();


	ImGui::Begin("VFX Player");


	//if (GetEngine().GetInputManager().IsKeyDown(Common::KeyCode::R))
	//{
	//	if (auto obj = vfxObj.lock())
	//	{
	//		obj->GetTransform().Move({ 0,0,500 });
	//	}
	//}

	static float time = 0;
	if (moving)
		time += Time::DeltaTime;
	if (!vfxObj.expired())
	{
		if (moving)
			vfxObj.lock()->GetTransform().SetPosition({ sinf(time * sinusSpeed) * sinusRange, 0, cosf(time * sinusSpeed) * sinusRange });

		vfxObj.lock()->GetTransform().SetRotation(vfxObj.lock()->GetTransform().GetRotation() * Quatf(Vec3f(rotX, rotY, rotZ) * Math::Deg2Rad * Time::DeltaTime));
	}

	//ImguiReflector imguiReflector = ImguiReflector(GetEngine().GetEditor(), nullptr, nullptr);
	//if (imguiReflector.Reflect(myVFXRef, "VFX File") & ReflectorResult_Changed)
	//{
	//	myVFXRef->Load();
	//}

	auto& vfxEditor = VFXEditor::GetInstance();
	//auto VfxRef = vfxEditor.GetVFXRef();
	if ((ImGui::Button("Play(Q)") || GetEngine().GetInputManager().IsKeyDown(Common::KeyCode::Q)))
	{
		Play();
	}
	ImGui::SameLine();
	if (ImGui::Button("Stop(E)") || GetEngine().GetInputManager().IsKeyDown(Common::KeyCode::E))
	{
		if (!vfxObj.expired())
		{
			if (auto* vfx = vfxObj.lock()->GetComponent<VFXComponent>())
			{
				vfx->Stop();
			}
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Force Stop(R)") || GetEngine().GetInputManager().IsKeyDown(Common::KeyCode::R))
	{
		if (auto obj = vfxObj.lock())
		{
			obj->Destroy();
		}
	}

	ImGui::Checkbox("Auto Play", &autoPlay);
	if (autoPlay)
	{
		if (!vfxObj.expired())
		{
			auto comp = vfxObj.lock()->GetComponent<VFXComponent>();
			if (comp)
			{
				if (comp->GetVFX().GetState() == eVFXState::STOP)
				{
					autoPlayTimer += Time::DeltaTime;
					if (autoPlayTimer > .5f /*&& (VfxRef && VfxRef->IsValid())*/)
					{
						vfxObj = AddGameObject<GameObject>()->GetWeak();

						auto* vfx = vfxObj.lock()->AddComponent<VFXComponent>(vfxEditor.GetDescription());
						vfx->Play();
						autoPlayTimer = 0;
					}
				}
			}
		}
	}


	ImGui::Checkbox("Move", &moving);
	if (moving)
	{
		ImGui::DragFloat("Speed", &sinusSpeed);
		ImGui::DragFloat("Radius", &sinusRange);
	}
	
	ImGui::DragFloat("Rotation X", &rotX);
	ImGui::DragFloat("Rotation Y", &rotY);
	ImGui::DragFloat("Rotation Z", &rotZ);

	ImGui::Spacing();
	ImGui::Text("Attraction point");
	ImGui::Checkbox("Draw", &myAttraction);
	ImGui::DragFloat3("Position", &myAttractionPoint.x);
	VFXParticleAttraction::SetAttractionPoint(myAttractionPoint);

	if (myAttraction)
	{
		GDebugDrawer->DrawSphere3D(DebugDrawFlags::Particles,
			myAttractionPoint, 10,
			0,
			Vec4f(1, 0, 0, 1));
	}

	if (ImGui::Button("Destroy"))
	{
		vfxObj.lock()->Destroy();
	}

	ImGui::End();
}
