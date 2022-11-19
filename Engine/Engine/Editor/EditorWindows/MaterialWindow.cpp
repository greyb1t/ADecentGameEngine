#include "pch.h"
#include "MaterialWindow.h"
#include "Engine/ResourceManagement/Resources/MaterialResource.h"
#include "Engine/ResourceManagement/Resources/TextureResource.h"
#include "Engine/Editor/EditorWindows/AssetBrowser/AssetBrowserWindow.h"
#include "Engine/Editor/EditorWindows/Inspector/InspectorWindow.h"
#include "Engine/Renderer/Texture/Texture2D.h"
#include "Engine/Editor/ImGuiHelper.h"
#include "Engine/Renderer/Material/MeshMaterial.h"
#include "Engine/Editor/Editor.h"
#include "Engine/Renderer/Material/DecalMaterial.h"
#include "Engine/Editor/DragDropConstants.h"
#include "Engine/Editor/FileTypes.h"

Engine::MaterialWindow::MaterialWindow(Editor& aEditor)
	: EditorWindow(aEditor, Flags::CanBeUnsaved)
{

}

void Engine::MaterialWindow::OpenMaterial(const MaterialRef& aMaterialResource)
{
	myMaterialResource = aMaterialResource;
}

std::string Engine::MaterialWindow::GetName() const
{
	return "Material Editor";
}

void Engine::MaterialWindow::Draw(const float aDeltaTime)
{
	/*if (!myMaterialResource || !myMaterialResource->IsValid())
	{
		ImGui::Text("No material selected, select one in the asset browser");
		return;
	}

	auto material = myMaterialResource->Get();

	ImGuiHelper::AlignedWidget("Path", ourAlignPercent);
	ImGui::Text("%s", myMaterialResource->GetPath().c_str());

	ImGui::Separator();

	ImGuiHelper::AlignedWidget("Name", ourAlignPercent);
	ImGui::InputText("##name", &material->myName);

	switch (material->GetType())
	{
		case Renderer::MaterialType::Mesh:
			break;
		case Renderer::MaterialType::Sprite:
			break;
		case Renderer::MaterialType::Decal:
			if (myMaterialResource->GetAsDecalMaterial())
			{
				DrawDecalMaterial(aDeltaTime, *myMaterialResource->GetAsDecalMaterial());
			}
			break;
		default:
			assert(false);
			break;
	}

	if (ImGui::Button("Save"))
	{
		Save();
	}*/
}

void Engine::MaterialWindow::DrawDecalMaterial(const float aDeltaTime, DecalMaterial& aDecalMaterial)
{
	//myEditor.DrawReflectedResource(
	//	aDecalMaterial.myAlbedoTexture,
	//	"Albedo",
	//	DragDropConstants::Texture,
	//	FileType::Extension_DDS);

	//myEditor.DrawReflectedResource(
	//	aDecalMaterial.myMaterialTexture,
	//	"Material",
	//	DragDropConstants::Texture,
	//	FileType::Extension_DDS);

	//myEditor.DrawReflectedResource(
	//	aDecalMaterial.myNormalTexture,
	//	"Normal",
	//	DragDropConstants::Texture,
	//	FileType::Extension_DDS);
}

void Engine::MaterialWindow::Save()
{
	/*if (!myMaterialResource || !myMaterialResource->IsValid())
	{
		return;
	}

	EditorWindow::Save();

	auto material = myMaterialResource->Get();

	FileIO::RemoveReadOnly(myMaterialResource->GetPath());

	std::ofstream file(myMaterialResource->GetPath());

	if (file.is_open())
	{
		file << std::setw(4) << material->ToJson();
		file.close();
	}
	else
	{
		LOG_ERROR(LogType::Engine) << "Failed to save material";
	}*/
}

