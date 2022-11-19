#include "pch.h"
#include "LandscapeComponent.h"

#include "InfoRigidBodyComponent.h"
#include "InfoRigidStaticComponent.h"
#include "PhysicsUtilities.h"
#include "Engine/Reflection/Reflector.h"
#include "Engine/DebugManager/DebugDrawer.h"
#include "Engine/Scene/Scene.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/ModelComponent.h"
#include "Engine/ResourceManagement/ResourceManager.h"
#include "Engine/ResourceManagement/Resources/ModelResource.h"
#include "Engine/Editor/Editor.h"
#include "Engine/Editor/EditorWindows/Inspector/InspectorWindow.h"
#include "Engine/Renderer/GraphicsEngine.h"
#include "Engine/Renderer/Model/ModelFactory.h"
#include "Engine/ResourceManagement/Resources/TextureResource.h"
#include "Engine/Renderer/Texture/Texture2D.h"
#include "Engine/ResourceManagement/Resources/MaterialResource.h"
#include "Engine/Renderer/GraphicsEngine.h"
#include "Engine/Renderer/Material/MaterialFactory.h"
#include "Physics/RigidBody.h"
#include "Physics/Shape.h"
#include "Collider.h"
#include "RigidStaticComponent.h"

Engine::LandscapeComponent::LandscapeComponent(GameObject* aGameObject)
	: Component(aGameObject)
{

}

void Engine::LandscapeComponent::Reflect(Reflector& aReflector)
{
	aReflector.ReflectLambda([&]
		{
			if (ImGui::ImageButton(
				GetEngine().GetEditor().GetInspectorWindow().GetSearchResourceIcon()->Get().GetSRV(),
				ImVec2(16, 16),
				ImVec2(0, 0),
				ImVec2(1, 1),
				4.f))
			{
				ImGui::OpenPopup("SearchResourcePopup");
			}

			if (GetEngine().IsEditorEnabled())
			{
				std::filesystem::path choosenPath;
				if (GetEngine().GetEditor().DrawSearchResourcePopup({ ".landscape" }, choosenPath))
				{
					myPath = choosenPath.string();
					LoadLandscape(myPath);
				}
			}
		});

	if (aReflector.Reflect(myPath, "Path", ReflectionFlags_ReadOnly) & ReflectorResult_Changed)
	{
		LoadLandscape(myPath);
	}

	if (aReflector.Button("Refresh landscape"))
	{
		LoadLandscape(myPath);
	}
}

void Engine::LandscapeComponent::SetLandscape(const std::string& aPath)
{
	myPath = aPath;
	LoadLandscape(aPath);
}

void Engine::LandscapeComponent::LoadLandscape(const std::string& aPath)
{
	if (aPath == "")
		return;

	if (GetGameObject())
	{
		for (auto& it : GetTransform().GetChildren())
		{
			it->GetGameObject()->Destroy();
		}
	}
	
	if (!std::filesystem::exists(aPath))
	{
		LOG_ERROR(LogType::Components) << "Landscape path does not exists. Path: " << aPath;
		return;
	}
	std::ifstream stream(aPath);
	nlohmann::json json;
	stream >> json;
	stream.close();
	for (int i = 0; i < json.size(); ++i)
	{
		std::filesystem::path path = json[i]["path"];

		auto object = myGameObject->GetScene()->AddGameObject<GameObject>();
		object->GetTransform().SetParent(&GetGameObject()->GetTransform());
		object->SetName(path.stem().string());

		Vec3f offset;
		offset.x = json[i]["offset"]["x"];
		offset.y = json[i]["offset"]["y"];
		offset.z = json[i]["offset"]["z"];
		object->GetTransform().SetPositionLocal(offset);

		auto model = object->AddComponent<ModelComponent>();

		auto var = GResourceManager->CreateRef<ModelResource>(path.string());
		var->Load();

		model->SetModel(path.string());

		if (myGameObject->GetScene()->GetName() == "UEDPIE_0_LVL01_GrasslandsAlpha" 
			|| myGameObject->GetScene()->GetName() == "AlexFinaBana" 
			|| myGameObject->GetScene()->GetName() == "UEDPIE_0_LVL02_GreenHill"
			|| myGameObject->GetScene()->GetName() == "UEDPIE_0_LVL05_Boss"
			|| myGameObject->GetScene()->GetName() == "UEDPIE_0_LVL01_GrassyPlains")
		{
			auto mat = GResourceManager->CreateRef<MaterialResource>(*GetEngine().GetGraphicsEngine().GetMaterialFactory().ConvertMaterialNameToPath("GrasslandsTriplanarDeferred"));
			mat->Load();
			model->SetMaterial("GrasslandsTriplanarDeferred");
		}
		else
		{
			auto mat = GResourceManager->CreateRef<MaterialResource>(*GetEngine().GetGraphicsEngine().GetMaterialFactory().ConvertMaterialNameToPath("DesertTriplanarDeferred"));
			mat->Load();
			model->SetMaterial("DesertTriplanarDeferred");
		}
		

		path.replace_extension(".px");
		if (std::filesystem::exists(path))
		{
			auto* collider = object->AddComponent<RigidStaticComponent>();
			collider->Attach(Shape::TriangleMeshPath(path.string()));
		}
		else
		{
			LOG_ERROR(LogType::Components) << "Landscape: No collider exists for chunk: " << path.stem();
		}
	}
}