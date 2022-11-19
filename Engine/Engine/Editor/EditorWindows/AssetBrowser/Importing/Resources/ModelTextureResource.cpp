#include "pch.h"
#include "ModelTextureResource.h"
#include "..\CreationStates\ModelImportState.h"
#include "Engine\Renderer\ModelLoader\LoaderModel.h"

Engine::ModelTextureResource::ModelTextureResource(const std::filesystem::path& aPath, const ModelTextureType aType)
	: Resource(aPath),
	myType(aType)
{
}

bool Engine::ModelTextureResource::Draw(ModelImportState& aModelCreationState)
{
	// We only support this for 1 model atm
	if (aModelCreationState.myLoaderModels.size() == 1)
	{
		return DrawOneModel(aModelCreationState);
	}
	else if (aModelCreationState.myLoaderModels.size() > 1)
	{
		if (aModelCreationState.GetState() == ModelImportState::State::HoudiniFBXMssImport)
		{
			return DrawMultipleMeshes(aModelCreationState);
		}
	}

	return false;
}

bool Engine::ModelTextureResource::DrawOneModel(ModelImportState& aModelCreationState)
{
	auto& loaderModel = aModelCreationState.myLoaderModels[0];

	// Make relative to bin folder
	const auto relativePath = std::filesystem::relative(GetPath());

	switch (myType)
	{
	case ModelTextureType::Albedo:
	{
		if (loaderModel->GetMeshes().size() == 1)
		{
			loaderModel->myMeshes[0].myAlbedoTexturePath = relativePath.string();
			return true;
		}
		else
		{
			for (int i = 0; i < loaderModel->GetMeshes().size(); ++i)
			{
				ImGui::PushID(i);
				const std::string s = "Albedo for " + loaderModel->GetMeshes()[i].myMeshName;

				if (ImGui::Button(s.c_str()))
				{
					loaderModel->myMeshes[i].myAlbedoTexturePath = relativePath.string();
					ImGui::PopID();
					return true;
				}
				ImGui::PopID();
			}

			if (ImGui::Button("Albedo for all meshes"))
			{
				for (int i = 0; i < loaderModel->GetMeshes().size(); ++i)
				{
					loaderModel->myMeshes[i].myAlbedoTexturePath = relativePath.string();
				}

				return true;
			}
		}
	} break;

	case ModelTextureType::Normal:
	{
		if (loaderModel->GetMeshes().size() == 1)
		{
			loaderModel->myMeshes[0].myNormalTexturePath = relativePath.string();
			return true;
		}
		else
		{
			for (int i = 0; i < loaderModel->GetMeshes().size(); ++i)
			{
				ImGui::PushID(i);
				const std::string s = "Normal for " + loaderModel->GetMeshes()[i].myMeshName;

				if (ImGui::Button(s.c_str()))
				{
					loaderModel->myMeshes[i].myNormalTexturePath = relativePath.string();
					ImGui::PopID();
					return true;
				}
				ImGui::PopID();
			}

			if (ImGui::Button("Normal for all meshes"))
			{
				for (int i = 0; i < loaderModel->GetMeshes().size(); ++i)
				{
					loaderModel->myMeshes[i].myNormalTexturePath = relativePath.string();
				}

				return true;
			}
		}
	} break;

	case ModelTextureType::Material:
	{
		if (loaderModel->GetMeshes().size() == 1)
		{
			loaderModel->myMeshes[0].myMaterialTexturePath = relativePath.string();
			return true;
		}
		else
		{
			for (int i = 0; i < loaderModel->GetMeshes().size(); ++i)
			{
				ImGui::PushID(i);
				const std::string s = "Material for " + loaderModel->GetMeshes()[i].myMeshName;

				if (ImGui::Button(s.c_str()))
				{
					loaderModel->myMeshes[i].myMaterialTexturePath = relativePath.string();
					ImGui::PopID();
					return true;
				}
				ImGui::PopID();
			}

			if (ImGui::Button("Material for all meshes"))
			{
				for (int i = 0; i < loaderModel->GetMeshes().size(); ++i)
				{
					loaderModel->myMeshes[i].myMaterialTexturePath = relativePath.string();
				}

				return true;
			}
		}
	} break;

	default:
		assert(false);
		break;
	}

	return false;
}

bool Engine::ModelTextureResource::DrawMultipleMeshes(
	ModelImportState& aModelCreationState)
{
	std::map<std::string, std::vector<LoaderMesh*>> meshLists;

	for (auto& loaderModel : aModelCreationState.myLoaderModels)
	{
		for (auto& mesh : loaderModel->GetMeshes())
		{
			meshLists[mesh.GetName()].push_back(&mesh);
		}
	}

	for (const auto& [meshName, meshList] : meshLists)
	{
		// Make relative to bin folder
		const auto relativePath = std::filesystem::relative(GetPath());

		switch (myType)
		{
		case ModelTextureType::Albedo:
		{
			ImGui::PushID(meshName.c_str());
			const std::string s = "Albedo for " + meshName;

			if (ImGui::Button(s.c_str()))
			{
				for (auto& mesh : meshList)
				{
					mesh->myAlbedoTexturePath = relativePath.string();
				}
				ImGui::PopID();
				return true;
			}
			ImGui::PopID();
		} break;

		case ModelTextureType::Normal:
		{
			ImGui::PushID(meshName.c_str());
			const std::string s = "Normal for " + meshName;

			if (ImGui::Button(s.c_str()))
			{
				for (auto& mesh : meshList)
				{
					mesh->myNormalTexturePath = relativePath.string();
				}
				ImGui::PopID();
				return true;
			}
			ImGui::PopID();
		} break;

		case ModelTextureType::Material:
		{
			ImGui::PushID(meshName.c_str());
			const std::string s = "Material for " + meshName;

			if (ImGui::Button(s.c_str()))
			{
				for (auto& mesh : meshList)
				{
					mesh->myMaterialTexturePath = relativePath.string();
				}
				ImGui::PopID();
				return true;
			}
			ImGui::PopID();
		} break;

		default:
			assert(false);
			break;
		}
	}

	return false;
}

std::string Engine::ModelTextureTypeToString(const ModelTextureType aModelTextureType)
{
	switch (aModelTextureType)
	{
	case ModelTextureType::Albedo: return "Albedo";
	case ModelTextureType::Normal: return "Normal";
	case ModelTextureType::Material: return "Material";
	default:
		assert(false);
		break;
	}

	return "unhandled case";
}
