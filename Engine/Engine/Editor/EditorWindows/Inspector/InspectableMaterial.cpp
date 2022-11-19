#include "pch.h"
#include "InspectableMaterial.h"
#include "Engine/ResourceManagement/Resources/MaterialResource.h"
#include "Engine/Renderer/Material/Material.h"
#include "Engine/Editor/ImGuiHelper.h"
#include "Engine/Editor/Editor.h"
#include "Engine/Renderer/Material/DecalMaterial.h"
#include "Engine/Editor/DragDropConstants.h"
#include "Engine/Editor/FileTypes.h"
#include "Engine/ResourceManagement/Resources/MaterialResource.h"
#include "Engine/ResourceManagement/Resources/TextureResource.h"
#include "Engine/ResourceManagement/Resources/VertexShaderResource.h"
#include "Engine/ResourceManagement/Resources/GeometryShaderResource.h"
#include "Engine/ResourceManagement/Resources/PixelShaderResource.h"
#include "Engine/Editor/EditorWindows/AssetBrowser/AssetBrowserWindow.h"
#include "Engine/Editor/EditorWindows/Inspector/InspectorWindow.h"
#include "Engine/Renderer/Texture/Texture2D.h"
#include "Engine/Renderer/Material/MeshMaterial.h"
#include "Engine/Paths.h"

Engine::InspectableMaterial::InspectableMaterial(
	Editor& aEditor,
	InspectorWindow& aInspectorWindow,
	const MaterialRef& aMaterialResource)
	: Inspectable(aEditor, aInspectorWindow),
	myMaterialResource(aMaterialResource)
{
}

void Engine::InspectableMaterial::Draw()
{
	Inspectable::Draw();

	if (!myMaterialResource || !myMaterialResource->IsValid())
	{
		ImGui::Text("No material selected, select one in the asset browser");
		return;
	}

	auto material = myMaterialResource->Get();

	ImGuiHelper::AlignedWidget("Path", ourAlignPercent);
	ImGui::Text("%s", myMaterialResource->GetPath().c_str());

	ImGui::Separator();

	// ImGuiHelper::AlignedWidget("Name", ourAlignPercent);
	// ImGui::InputText("##name", &material->myName);

	DrawMaterial(*myMaterialResource->Get());

	switch (material->GetType())
	{
	case MaterialType::Mesh:
		if (myMaterialResource->GetAsMeshMaterial())
		{
			DrawMeshMaterial(*myMaterialResource->GetAsMeshMaterial());
		}
		break;
	case MaterialType::Sprite:
		break;
	case MaterialType::Decal:
		if (myMaterialResource->GetAsDecalMaterial())
		{
			DrawDecalMaterial(*myMaterialResource->GetAsDecalMaterial());
		}
		break;
	default:
		break;
	}

	if (ImGui::Button("Save"))
	{
		myInspectorWindow.Save();
	}
}

void Engine::InspectableMaterial::Save()
{
	if (!myMaterialResource || !myMaterialResource->IsValid())
	{
		return;
	}

	auto material = myMaterialResource->Get();

	FileIO::RemoveReadOnly(myMaterialResource->GetPath());

	std::ofstream file(myMaterialResource->GetPath());

	if (file.is_open())
	{
		file << std::setw(4) << material->ToJson();
		file.close();

		// Reload
		myMaterialResource->Unload();
		myMaterialResource->Load();
	}
	else
	{
		LOG_ERROR(LogType::Engine) << "Failed to save material";
	}
}

void Engine::InspectableMaterial::DrawMaterial(Material& aMaterial)
{
	ImGuiHelper::AlignedWidget("Is Transparent", Editor::ourAlignPercent);
	ImGui::Checkbox("##istransparent", &aMaterial.myIsTransparent);

	ImGuiHelper::AlignedWidget("Cast Shadows", Editor::ourAlignPercent);
	ImGui::Checkbox("##castshadows", &aMaterial.myIsCastingShadows);

	for (size_t passIndex = 0; passIndex < aMaterial.GetPasses().size(); ++passIndex)
	{
		auto& pass = aMaterial.GetPasses()[passIndex];

		if (ImGui::CollapsingHeader(("Pass " + std::to_string(passIndex)).c_str(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			/*
				std::vector<EffectConstantBufferDesc> myConstantBuffers;

				EffectStencilDesc myStencil;
			*/

			DrawRenderTypeCombo(*pass);

			DrawVertexShaderInput(aMaterial, *pass);

			DrawPixelShaderInput(*pass);

			if (aMaterial.GetType() == MaterialType::Sprite)
			{
				DrawGeometryShaderInput(*pass);
			}

			DrawRasterizerStateCombo(*pass);

			DrawBlendStateCombo(*pass);

			ImGuiHelper::AlignedWidget("Instanced", ourAlignPercent);

			bool isInstanced = std::any_of(
				pass->myDesc.myShaderDefines.begin(),
				pass->myDesc.myShaderDefines.end(),
				[](const std::string& aDefine)
				{
					return aDefine == "INSTANCED";
				}
			);

			if (ImGui::Checkbox("##instanced", &isInstanced))
			{
				if (isInstanced)
				{
					// add define
				}
				else
				{
					// remove define
				}
			}

			ImGuiHelper::AlignedWidget("Emission Intensity", ourAlignPercent);
			ImGui::DragFloat("##emissionintensity", &pass->myDesc.myEmissionIntensity, 0.1f, 0.f, 1000.f);

			DrawTextureList(*pass);

			DrawConstantBufferList(*pass);
		}
	}
}

void Engine::InspectableMaterial::DrawRenderTypeCombo(EffectPass& aPass)
{
	ImGuiHelper::AlignedWidget("Renderer Type", ourAlignPercent);

	if (ImGui::BeginCombo(
		"##renderertype",
		RendererTypeStrings[static_cast<int>(aPass.myDesc.myRenderingType)].c_str()))
	{
		for (int i = 0; i < static_cast<int>(RendererType::Count); ++i)
		{
			if (ImGui::Selectable(
				RendererTypeStrings[i].c_str(),
				static_cast<int>(aPass.myDesc.myRenderingType) == i))
			{
				aPass.myDesc.myRenderingType = static_cast<RendererType>(i);

				switch (aPass.myDesc.myRenderingType)
				{
				case RendererType::Forward:
				{
					aPass.myDesc.myPixelShaderNamePath = "Assets/Shaders/Engine/PixelShader" + FileType::Extension_PixelShader;
				} break;
				case RendererType::Deferred:
				{
					aPass.myDesc.myPixelShaderNamePath = "Assets/Shaders/Engine/Deferred/GBuffer" + FileType::Extension_PixelShader;
				} break;
				default:
					break;
				}
			};
		}

		ImGui::EndCombo();
	}
}

void Engine::InspectableMaterial::DrawPixelShaderInput(EffectPass& aPass)
{
	auto ps = GResourceManager->CreateRef<PixelShaderResource>(aPass.myDesc.myPixelShaderNamePath);
	ps->Load();

	if (myEditor.DrawReflectedResource(
		ps,
		"Pixel Shader",
		DragDropConstants::PixelShader,
		FileType::Extension_PixelShader,
		nullptr))
	{
		aPass.myDesc.myPixelShaderNamePath = ps->GetPath();
	}
}

void Engine::InspectableMaterial::DrawGeometryShaderInput(EffectPass& aPass)
{
	auto gs = GResourceManager->CreateRef<GeometryShaderResource>(aPass.myDesc.myGeometryShaderNamePath);
	gs->Load();

	if (myEditor.DrawReflectedResource(
		gs,
		"Geometry Shader",
		DragDropConstants::GeometryShader,
		FileType::Extension_GeometryShader,
		nullptr))
	{
		aPass.myDesc.myGeometryShaderNamePath = gs->GetPath();
	}
}

void Engine::InspectableMaterial::DrawVertexShaderInput(Material& aMaterial, EffectPass& aPass)
{
	// TODO(filip): Now we choose the vertex type based on the MATERIAL
	// should we instead couple the vertextype with the .vshader file?

	int elemCount = -1;
	D3D11_INPUT_ELEMENT_DESC const* elements = nullptr;

	switch (aMaterial.GetType())
	{
	case MaterialType::Mesh:
		elemCount = DefaultVertex::ourElementCount;
		elements = DefaultVertex::ourInputElements;
		break;
	case MaterialType::Sprite:
		elemCount = SpriteVertex::ourElementCount;
		elements = SpriteVertex::ourInputElements;
		break;
	case MaterialType::Decal:
		// NOTE(filip): atm decals have same vertex type as any mesh
		// this is really bad, fix in future
		elemCount = DefaultVertex::ourElementCount;
		elements = DefaultVertex::ourInputElements;
		break;
	case MaterialType::Particle:
		elemCount = ParticleVertex::ourElementCount;
		elements = ParticleVertex::ourInputElements;
		break;
	default:
		assert(false);
		break;
	}

	if (elements)
	{
		auto vs = GResourceManager->CreateRef<VertexShaderResource>(aPass.myDesc.myVertexShaderNamePath, elements, elemCount);
		vs->Load();

		if (myEditor.DrawReflectedResource(
			vs,
			"Vertex Shader",
			DragDropConstants::VertexShader,
			FileType::Extension_VertexShader,
			nullptr,
			elements,
			elemCount,
			std::vector<ShaderDefines>()))
		{
			aPass.myDesc.myVertexShaderNamePath = vs->GetPath();
		}
	}
	else
	{
		ImGui::TextColored(ImVec4(1, 0, 0, 1), "CALL FILIP HES CRYING");
	}
}

void Engine::InspectableMaterial::DrawRasterizerStateCombo(EffectPass& aPass)
{
	ImGuiHelper::AlignedWidget("Rasterizer State", ourAlignPercent);

	if (ImGui::BeginCombo(
		"##rasterizerstate",
		RasterizerStateStrings[aPass.myDesc.myRasterizerState].c_str()))
	{
		for (int i = 0; i < RasterizerState_Count; ++i)
		{
			if (ImGui::Selectable(
				RasterizerStateStrings[i].c_str(),
				aPass.myDesc.myRasterizerState == i))
			{
				aPass.myDesc.myRasterizerState = static_cast<RasterizerState>(i);
			};
		}

		ImGui::EndCombo();
	}
}

void Engine::InspectableMaterial::DrawBlendStateCombo(EffectPass& aPass)
{
	ImGuiHelper::AlignedWidget("Blend State", ourAlignPercent);

	if (ImGui::BeginCombo(
		"##blendstate",
		BlendStateStrings[aPass.myDesc.myBlendState].c_str()))
	{
		for (int i = 0; i < BlendState_Count; ++i)
		{
			if (ImGui::Selectable(
				BlendStateStrings[i].c_str(),
				aPass.myDesc.myBlendState == i))
			{
				aPass.myDesc.myBlendState = static_cast<BlendState>(i);
			};
		}

		ImGui::EndCombo();
	}
}

void Engine::InspectableMaterial::DrawTextureList(EffectPass& aPass)
{
	if (ImGui::BeginChild("##texturelistchild", ImVec2(0.f, 200.f), true))
	{
		for (size_t textureIndex = 0; textureIndex < aPass.myDesc.myTextures.size(); ++textureIndex)
		{
			auto& texture = aPass.myDesc.myTextures[textureIndex];

			ImGui::PushID(&texture);

			if (ImGuiHelper::RedRemoveButton("##remove", 64.f))
			{
				aPass.myDesc.myTextures.erase(aPass.myDesc.myTextures.begin() + textureIndex);
				--textureIndex;
				ImGui::PopID();
				continue;
			}

			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Remove");
			}

			ImGui::SameLine();

			TextureRef textureRef;

			if (!texture.myTexturePath.empty())
			{
				textureRef = GResourceManager->CreateRef<TextureResource>(texture.myTexturePath);

				textureRef->Load();

				if (textureRef && textureRef->IsValid())
				{
					ImGui::Image(textureRef->Get().GetSRV(), ImVec2(64.f, 64.f));

					if (ImGui::IsItemHovered())
					{
						ImGui::BeginTooltip();
						ImGui::Image(textureRef->Get().GetSRV(), ImVec2(256.f, 256.f));
						ImGui::EndTooltip();
					}
				}
			}

			ImGui::SameLine();

			ImGui::BeginGroup();

			ImGuiHelper::AlignedWidget("Shader Name", ourAlignPercent);
			ImGuiHelper::InputTextHintCheckValidity(
				"##texturename",
				"Name in shader",
				texture.myNameInShader,
				[&aPass, textureIndex](const std::string& aText)
				{
					if (aText.empty())
					{
						return "Cannot be empty";
					}

					for (size_t textureIndexInner = 0;
						textureIndexInner < aPass.myDesc.myTextures.size();
						++textureIndexInner)
					{
						if (textureIndexInner == textureIndex)
						{
							continue;
						}

						if (aPass.myDesc.myTextures[textureIndexInner].myNameInShader == aText)
						{
							return "Name already exists";
						}
					}

					return "";
				});

			if (myEditor.DrawReflectedResource(
				textureRef,
				"Texture",
				{ DragDropConstants::Texture, DragDropConstants::HDRTexture },
				{ FileType::Extension_DDS, FileType::Extension_HDR }))
			{
				aPass.myDesc.myTextures[textureIndex].myTexturePath = textureRef->GetPath();
			}

			ImGui::EndGroup();

			ImGui::PopID();
		}

		if (ImGui::Button("Add Texture"))
		{
			EffectTextureDesc textureDesc;
			textureDesc.myNameInShader = "Unnamed";
			textureDesc.myTexturePath = GetEngine().GetPaths().GetPathString(PathEnum::NoTexture);
			aPass.myDesc.myTextures.push_back(textureDesc);
		}
	}
	ImGui::EndChild();
}

void Engine::InspectableMaterial::DrawDecalMaterial(DecalMaterial& aDecalMaterial)
{
	myEditor.DrawReflectedResource(
		aDecalMaterial.myAlbedoTexture,
		"Albedo",
		DragDropConstants::Texture,
		FileType::Extension_DDS);

	myEditor.DrawReflectedResource(
		aDecalMaterial.myMaterialTexture,
		"Material",
		DragDropConstants::Texture,
		FileType::Extension_DDS);

	myEditor.DrawReflectedResource(
		aDecalMaterial.myNormalTexture,
		"Normal",
		DragDropConstants::Texture,
		FileType::Extension_DDS);
}

void Engine::InspectableMaterial::DrawMeshMaterial(MeshMaterial& aMeshMaterial)
{
	myEditor.DrawReflectedResource(
		aMeshMaterial.myAlbedoTexture,
		"Albedo",
		DragDropConstants::Texture,
		FileType::Extension_DDS);

	myEditor.DrawReflectedResource(
		aMeshMaterial.myMaterialTexture,
		"Material",
		DragDropConstants::Texture,
		FileType::Extension_DDS);

	myEditor.DrawReflectedResource(
		aMeshMaterial.myNormalTexture,
		"Normal",
		DragDropConstants::Texture,
		FileType::Extension_DDS);

	myEditor.DrawReflectedResource(
		aMeshMaterial.myEmissiveTexture,
		"Emissive",
		DragDropConstants::Texture,
		FileType::Extension_DDS);
}

void Engine::InspectableMaterial::DrawConstantBufferList(EffectPass& aPass)
{
	if (ImGui::BeginChild("##constantbuffers", ImVec2(0.f, 200.f), true))
	{
		for (size_t cbufferIndex = 0;
			cbufferIndex < aPass.myDesc.myConstantBuffers.size();
			++cbufferIndex)
		{
			auto& cbuffer = aPass.myDesc.myConstantBuffers[cbufferIndex];

			ImGui::PushID(&cbuffer);


			if (ImGuiHelper::RedRemoveButton("##remove", 0.f))
			{
				aPass.myDesc.myConstantBuffers.erase(aPass.myDesc.myConstantBuffers.begin() + cbufferIndex);
				--cbufferIndex;
				//ImGui::EndGroup();
				ImGui::PopID();
				continue;
			}

			ImGui::SameLine();

			ImGui::BeginGroup();

			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Remove");
			}

			ImGui::SameLine();

			ImGuiHelper::AlignedWidget("Constant Buffer Name", ourAlignPercent);
			ImGuiHelper::InputTextHintCheckValidity(
				"##cbuffername",
				"Name in shader",
				cbuffer.myName,
				[&aPass, cbufferIndex](const std::string& aText)
				{
					if (aText.empty())
					{
						return "Cannot be empty";
					}

					for (size_t cbufferIndexInner = 0;
						cbufferIndexInner < aPass.myDesc.myConstantBuffers.size();
						++cbufferIndexInner)
					{
						if (cbufferIndexInner == cbufferIndex)
						{
							continue;
						}

						if (aPass.myDesc.myConstantBuffers[cbufferIndexInner].myName == aText)
						{
							return "Name already exists";
						}
					}

					return "";
				});

			//// VARIABLES
			ImGui::BeginGroup();
			for (size_t variableIndex = 0;
				variableIndex < cbuffer.myLayout.myVariables.size();
				++variableIndex)
			{
				auto& variables = cbuffer.myLayout.myVariables;
				auto& variable = variables[variableIndex];

				ImGui::PushID(&variable);

				if (ImGuiHelper::RedRemoveButton("##removevariable", 64.f))
				{
					variables.erase(variables.begin() + variableIndex);
					--variableIndex;
					ImGui::PopID();
					continue;
				}

				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltip("Remove");
				}

				ImGui::SameLine();

				ImGui::BeginGroup();

				ImGuiHelper::AlignedWidget("Variable Name", ourAlignPercent);
				ImGuiHelper::InputTextHintCheckValidity(
					"##variablename",
					"Name in shader",
					variable.myName,
					[&variables, variableIndex](const std::string& aText)
					{
						if (aText.empty())
						{
							return "Cannot be empty";
						}

						for (size_t variableIndexInner = 0;
							variableIndexInner < variables.size();
							++variableIndexInner)
						{
							if (variableIndexInner == variableIndex)
							{
								continue;
							}

							if (variables[variableIndexInner].myName == aText)
							{
								return "Name already exists";
							}
						}

						return "";
					});

				ImGuiHelper::AlignedWidget("Default Value", ourAlignPercent);
				ImGui::DragFloat4("##defaultvalue", &variable.myDefaultValue.x);

				ImGui::EndGroup();

				ImGui::PopID();
			}

			if (ImGui::Button("New variable"))
			{
				EffectConstantBufferDesc::Layout::Variable var;
				var.myName = "Unnamed";
				cbuffer.myLayout.myVariables.push_back(var);
			}

			ImGui::EndGroup();

			ImGui::Separator();

			ImGui::EndGroup();

			ImGui::PopID();
		}

		if (ImGui::Button("Add Constant Buffer"))
		{
			EffectConstantBufferDesc cbufferDesc;
			cbufferDesc.myName = "Unnamed";
			aPass.myDesc.myConstantBuffers.push_back(cbufferDesc);
		}
	}
	ImGui::EndChild();
}
