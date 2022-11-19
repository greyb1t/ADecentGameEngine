#include "pch.h"
#include "VFXEditorParticlesEvent.h"

#include "VFXEditor.h"
#include "VFXEditorHelpers.h"
#include "VFXEditorPropertyWindow.h"
#include "Engine/Editor/DragDropConstants.h"
#include "Engine/Editor/Editor.h"
#include "Engine/Editor/FileTypes.h"
#include "Engine/ResourceManagement/Resources/AnimationCurveResource.h"
#include "Engine/ResourceManagement/Resources/MaterialResource.h"
#include "Engine/ResourceManagement/Resources/TextureResource.h"
#include "Engine/Editor/EditorWindows/Inspector/InspectorWindow.h"
#include "Engine/Editor/EditorWindows/AssetBrowser/AssetBrowserWindow.h"
#include "Engine/Renderer/Texture/Texture2D.h"

Engine::VFXEditorParticlesEvent::VFXEditorParticlesEvent()
	: myImguiReflector(GetEngine().GetEditor(), nullptr, nullptr)
{
	myType = eEditorEventType::ParticleEmitter;
	Setup();

	// TODO: Remove this it's only for debugging
	myEmitterDesc.base.geometry = EmitterGeometry::Sphere(300);
}

bool Engine::VFXEditorParticlesEvent::StartDraw()
{
	bool result = VFXEditorEvent::StartDraw();
	if (result)
	{
		Header();

		ImGui::PushID("Spawn");
		DrawAddVariableSpawn();
		DrawSpawnVariables();
		ImGui::PopID();

		ImGui::PushID("Update");
		DrawAddVariableUpdate();
		DrawUpdateVariables();
		ImGui::PopID();
	}

	return result;
}

void Engine::VFXEditorParticlesEvent::EndDraw()
{
	VFXEditorEvent::EndDraw();
}

void Engine::VFXEditorParticlesEvent::Header()
{
	VFXEditorHelpers::TitleBar("Particle Emitter", 21.5f, IM_COL32(55, 55, 55, 255));
	ImGui::Spacing();
	ImGui::DragFloat("Lifetime", &myEmitterDesc.base.duration);
	ImGui::Checkbox("Keep alive", &myEmitterDesc.base.keepAlive);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Keeps the particle emitter alive until all particles are dead.");

	DrawTransform();

	VFXEditorHelpers::DrawSpace(myEmitterDesc.base.space);

	DrawTimeline();
	DrawGeometry();

}

void Engine::VFXEditorParticlesEvent::DrawAddVariableSpawn()
{
	if (VFXEditorHelpers::TitleButton("Particle Spawn", "ADD"))
	{
		ImGui::OpenPopup("spawn_add_variable_popup");
	}

	if (ImGui::BeginPopup("spawn_add_variable_popup"))
	{
		for (int i = 0; i < ARRAYSIZE(VFX::ParticleStartVariables::titles); i++)
		{
			if (myParticleDesc.startVariables & (1 << i))
				continue;

			if (ImGui::MenuItem(VFX::ParticleStartVariables::titles[i]))
			{
				AddVariable(static_cast<VFX::ParticleStartVariables::eVariables>(1 << i));
			}
		}

		ImGui::EndPopup();
	}

	ImGui::Spacing();
}

void Engine::VFXEditorParticlesEvent::DrawAddVariableUpdate()
{
	if (VFXEditorHelpers::TitleButton("Particle Update", "ADD"))
	{
		ImGui::OpenPopup("update_add_variable_popup");
	}

	if (ImGui::BeginPopup("update_add_variable_popup"))
	{
		for (int i = 0; i < ARRAYSIZE(VFX::ParticleUpdateVariables::titles); i++)
		{
			if (myParticleDesc.updateVariables & (1 << i))
				continue;

			if (ImGui::MenuItem(VFX::ParticleUpdateVariables::titles[i]))
			{
				AddVariable(static_cast<VFX::ParticleUpdateVariables::eVariables>(1 << i));
			}
		}

		ImGui::EndPopup();
	}

	ImGui::Spacing();
}

void Engine::VFXEditorParticlesEvent::DrawSpawnVariables()
{
	for (int i = 0; i < sizeof(unsigned int) * 8; i++)
	{
		const auto e = static_cast<VFX::ParticleStartVariables::eVariables>(1 << (i));
		if (myParticleDesc.startVariables & e)
		{
			DrawVariable(e);
		}
	}
}

void Engine::VFXEditorParticlesEvent::DrawUpdateVariables()
{
	for (int i = 0; i < sizeof(unsigned int) * 8; i++)
	{
		const auto e = static_cast<VFX::ParticleUpdateVariables::eVariables>(1 << (i));
		if (myParticleDesc.updateVariables & e)
		{
			DrawVariable(e);
		}
	}
}

void Engine::VFXEditorParticlesEvent::DrawTransform()
{
	if (ImGui::Button("Transform"))
	{
		VFXEditorPropertyWindow::Instance().Set([&]()
			{
				VFXEditorHelpers::DrawTransform(myEmitterDesc.base.position, myEmitterDesc.base.rotation);
			});
	}
	auto* obj = VFXEditor::GetInstance().GetGameObject();
	if (!obj)
		return;

	auto fwd = myEmitterDesc.base.rotation.ToMatrix().GetForward();
	GDebugDrawer->DrawLine3D(DebugDrawFlags::Particles,
		obj->GetTransform().GetPosition(),
		obj->GetTransform().GetPosition() + fwd * 100.f,
		0,
		Vec4f(1, 1, 1, 1));
}

void Engine::VFXEditorParticlesEvent::DrawGeometry()
{
	if (ImGui::Button("Geometry"))
	{
		VFXEditorPropertyWindow::Instance().Set([&]()
			{
				VFXEditorHelpers::DrawGeometry(myEmitterDesc.base.geometry);
			});
	}

	{
		auto* obj = VFXEditor::GetInstance().GetGameObject();
		if (!obj)
			return;

		switch (myEmitterDesc.base.geometry.GetShape())
		{
		case eEmitShape::POINT:
		{

		}
		break;
		case eEmitShape::SPHERE:
		{
			GDebugDrawer->DrawSphere3D(DebugDrawFlags::Particles,
				obj->GetTransform().GetPosition(),
				myEmitterDesc.base.geometry.properties.sphere.radius,
				0,
				Vec4f(1, 1, 1, 1));
			if (myEmitterDesc.base.geometry.properties.sphere.minRadius > 0)
			{
				GDebugDrawer->DrawSphere3D(DebugDrawFlags::Particles,
					obj->GetTransform().GetPosition(),
					myEmitterDesc.base.geometry.properties.sphere.minRadius,
					0,
					Vec4f(.5f, .5f, .5f, 1));
			}
		}
		break;
		case eEmitShape::BOX:
		{
			GDebugDrawer->DrawCube3D(DebugDrawFlags::Particles,
				obj->GetTransform().GetPosition(),
				obj->GetTransform().GetRotation().EulerAngles(),
				myEmitterDesc.base.geometry.properties.box.halfSize,
				0,
				Vec4f(1, 1, 1, 1));
		}
		break;
		case eEmitShape::CONE:
		{
			GDebugDrawer->DrawCircle3D(DebugDrawFlags::Particles,
				obj->GetTransform().GetPosition(),
				30,
				0,
				Vec4f(1, 0, 0, 1));
		}
		break;
		default:;
		}

	}
}

void Engine::VFXEditorParticlesEvent::DrawTimeline()
{
	if (ImGui::Button("Timeline"))
	{
		VFXEditorPropertyWindow::Instance().Set([&]()
			{
				VFXEditorHelpers::DrawTimeline(myEmitterDesc.base.timeline);
			});
	}
}

void Engine::VFXEditorParticlesEvent::AddVariable(VFX::ParticleStartVariables::eVariables aVariable)
{
	myParticleDesc.startVariables |= aVariable;
}

void Engine::VFXEditorParticlesEvent::AddVariable(VFX::ParticleUpdateVariables::eVariables aVariable)
{
	myParticleDesc.updateVariables |= aVariable;
}

void Engine::VFXEditorParticlesEvent::DrawVariable(VFX::ParticleStartVariables::eVariables aVariable)
{
	ImGui::PushID("StartVariables_" + aVariable);
	if (ImGui::Button(VFX::ParticleStartVariables::titles[static_cast<unsigned>(
		std::log2(static_cast<unsigned>(aVariable)))]))
	{
		SetPropertyVariable(aVariable);
	}
	ImGui::SameLine(0, 10);
	if (ImGui::Button("-"))
	{
		myParticleDesc.startVariables &= ~aVariable;
	}
	ImGui::PopID();
}

void Engine::VFXEditorParticlesEvent::DrawVariable(VFX::ParticleUpdateVariables::eVariables aVariable)
{
	ImGui::PushID("UpdateVariables_" + aVariable);
	if (ImGui::Button(VFX::ParticleUpdateVariables::titles[static_cast<unsigned>(
		std::log2(static_cast<unsigned>(aVariable)))]))
	{
		SetPropertyVariable(aVariable);
	}
	ImGui::SameLine(0, 10);
	if (ImGui::Button("-"))
	{
		myParticleDesc.updateVariables &= ~aVariable;
	}
	ImGui::PopID();
}

void Engine::VFXEditorParticlesEvent::SetPropertyVariable(VFX::ParticleStartVariables::eVariables aVariable)
{
	switch (aVariable)
	{
	case VFX::ParticleStartVariables::LIFETIME:
	{
		VFXEditorPropertyWindow::Instance().Set([&, aVariable]()
			{
				ImGui::Text(VFX::ParticleStartVariables::titles[static_cast<unsigned int>(std::log2(static_cast<unsigned int>(aVariable)))]);
				ImGui::InputFloat("LifeTime", &myParticleDesc.lifetime);
				ImGui::InputFloat("Modifier", &myParticleDesc.lifetimeModifier);
			});
	}
	break;
	case VFX::ParticleStartVariables::COLOR:
	{
		VFXEditorPropertyWindow::Instance().Set([&, aVariable]()
			{
				ImGui::Text(VFX::ParticleStartVariables::titles[static_cast<unsigned int>(std::log2(static_cast<unsigned int>(aVariable)))]);

				VFXEditorHelpers::DrawSetting(myParticleDesc.colorSetting, VFX::eLERP);

				Vec4f& color = myParticleDesc.color;
				float colors[4]{ color.x, color.y, color.z, color.w };
				if (ImGui::ColorPicker4("Color", colors))
				{
					color = Vec4f(colors[0], colors[1], colors[2], colors[3]);
				}

				if (myParticleDesc.colorSetting & VFX::ePropertySetting::eLERP)
				{
					Vec4f& endColor = myParticleDesc.endColor;
					float endColors[4]{ endColor.x, endColor.y, endColor.z, endColor.w };
					if (ImGui::ColorPicker4("End color", endColors))
					{
						endColor = Vec4f(endColors[0], endColors[1], endColors[2], endColors[3]);
					}
				}

				ImGui::InputFloat("Emissive", &myParticleDesc.colorIntensity);
			});
	}
	break;
	case VFX::ParticleStartVariables::SIZE:
	{
		if (!mySizeCurve || mySizeCurve->GetPath() != myParticleDesc.sizeCurve)
		{
			mySizeCurve = GResourceManager->CreateRef<AnimationCurveResource>(myParticleDesc.sizeCurve);
		}
		mySizeCurve->Load();
		VFXEditorPropertyWindow::Instance().Set([&, aVariable]()
			{
				ImGui::Text(VFX::ParticleStartVariables::titles[static_cast<unsigned int>(std::log2(static_cast<unsigned int>(aVariable)))]);

				VFXEditorHelpers::DrawSetting(myParticleDesc.sizeSetting, VFX::eLERP | VFX::eMODIFIER);

				ImGui::InputFloat2("Size", &myParticleDesc.size.x);

				if (myParticleDesc.sizeSetting & VFX::ePropertySetting::eMODIFIER)
				{
					bool uniform = myParticleDesc.sizeModifier.y < 0;
					Vec2f modifier = myParticleDesc.sizeModifier;

					if (uniform)
						modifier.y = modifier.x;

					if (ImGui::InputFloat2("Modifier", &modifier.x))
					{
						if (uniform)
						{
							myParticleDesc.sizeModifier.x = modifier.x;
						} else
						{
							myParticleDesc.sizeModifier = modifier;
						}
					}

					if (ImGui::Checkbox("    Uniform", &uniform))
					{
						myParticleDesc.sizeModifier.y = uniform ? -1 : myParticleDesc.sizeModifier.x;
					}
				}

				if (myParticleDesc.sizeSetting & VFX::ePropertySetting::eLERP)
				{
					if (myImguiReflector.Reflect(
						mySizeCurve, "Size Curve")
						& ReflectorResult_Changed)
					{
						myParticleDesc.sizeCurve = mySizeCurve->GetPath();
					}
				}

			});
	}
	break;
	case VFX::ParticleStartVariables::ROTATION:
	{
		VFXEditorPropertyWindow::Instance().Set([&, aVariable]()
			{
				ImGui::Text(VFX::ParticleStartVariables::titles[static_cast<unsigned int>(std::log2(static_cast<unsigned int>(aVariable)))]);
				ImGui::DragFloat("Rotation", &myParticleDesc.startRotation.rotation);
				ImGui::DragFloat("Modifier", &myParticleDesc.startRotation.modifier);
			});
	}
	break;
	case VFX::ParticleStartVariables::VELOCITY:
	{
		VFXEditorPropertyWindow::Instance().Set([&, aVariable]()
			{
				ImGui::Text(VFX::ParticleStartVariables::titles[static_cast<unsigned int>(std::log2(static_cast<unsigned int>(aVariable)))]);

				ImGui::InputFloat("Velocity", &myParticleDesc.startVelocity.force);

				VFXEditorHelpers::DrawSetting(myParticleDesc.startVelocity.settings, VFX::eMODIFIER);
				if (myParticleDesc.startVelocity.settings & VFX::eMODIFIER)
				{
					ImGui::DragFloat("Velocity Modifier", &myParticleDesc.startVelocity.modifier);
				}

			});
	}
	break;
	case VFX::ParticleStartVariables::TEXTURE:
	{
		if (!myTextureRef || myTextureRef->GetPath() != myParticleDesc.texturePath)
		{
			myTextureRef = GResourceManager->CreateRef<TextureResource>(myParticleDesc.texturePath);
		}
		myTextureRef->Load();

		VFXEditorPropertyWindow::Instance().Set([&, aVariable]()
			{
				ImGui::Text(VFX::ParticleStartVariables::titles[static_cast<unsigned int>(std::log2(static_cast<unsigned int>(aVariable)))]);
				if (myImguiReflector.Reflect(myTextureRef, "Texture") & ReflectorResult_Changed)
				{
					myParticleDesc.texturePath = myTextureRef->GetPath();
					myTextureRef->Load();
					myEmitterDesc.myRenderData.myTexture = myTextureRef;
				}
			});
	}
	break;
	case VFX::ParticleStartVariables::MATERIAL:
	{
		if (!myMaterialRef || myMaterialRef->GetPath() != myParticleDesc.materialPath)
		{
			myMaterialRef = GResourceManager->CreateRef<MaterialResource>(myParticleDesc.materialPath);
		}
		myMaterialRef->Load();

		if (!myMaterialCurveRef1 || myMaterialCurveRef1->GetPath() != myParticleDesc.materialShaderRefPath1)
		{
			myMaterialCurveRef1 = GResourceManager->CreateRef<AnimationCurveResource>(myParticleDesc.materialShaderRefPath1);
		}
		if (!myMaterialCurveRef2 || myMaterialCurveRef2->GetPath() != myParticleDesc.materialShaderRefPath2)
		{
			myMaterialCurveRef2 = GResourceManager->CreateRef<AnimationCurveResource>(myParticleDesc.materialShaderRefPath2);
		}
		if (!myMaterialCurveRef3 || myMaterialCurveRef3->GetPath() != myParticleDesc.materialShaderRefPath3)
		{
			myMaterialCurveRef3 = GResourceManager->CreateRef<AnimationCurveResource>(myParticleDesc.materialShaderRefPath3);
		}
		if (!myMaterialCurveRef4 || myMaterialCurveRef4->GetPath() != myParticleDesc.materialShaderRefPath4)
		{
			myMaterialCurveRef4 = GResourceManager->CreateRef<AnimationCurveResource>(myParticleDesc.materialShaderRefPath4);
		}
		myMaterialCurveRef1->Load();
		myMaterialCurveRef2->Load();
		myMaterialCurveRef3->Load();
		myMaterialCurveRef4->Load();


		VFXEditorPropertyWindow::Instance().Set([&, aVariable]()
			{
				ImGui::Text(VFX::ParticleStartVariables::titles[static_cast<unsigned int>(std::log2(static_cast<unsigned int>(aVariable)))]);

				if (myImguiReflector.Reflect(myMaterialRef, MaterialType::Particle, "Material") & ReflectorResult_Changed)
				{
					myParticleDesc.materialPath = myMaterialRef->GetPath();
				}

				if (myImguiReflector.Reflect(
					myMaterialCurveRef1, "Shader Curve 1") 
					& ReflectorResult_Changed)
				{
					myParticleDesc.materialShaderRefPath1 = myMaterialCurveRef1->GetPath();
				}
				if (myImguiReflector.Reflect(
					myMaterialCurveRef2, "Shader Curve 2") 
					& ReflectorResult_Changed)
				{
					myParticleDesc.materialShaderRefPath2 = myMaterialCurveRef2->GetPath();
				}
				if (myImguiReflector.Reflect(
					myMaterialCurveRef3, "Shader Curve 3") 
					& ReflectorResult_Changed)
				{
					myParticleDesc.materialShaderRefPath3 = myMaterialCurveRef3->GetPath();
				}
				if (myImguiReflector.Reflect(
					myMaterialCurveRef4, "Shader Curve 4") 
					& ReflectorResult_Changed)
				{
					myParticleDesc.materialShaderRefPath4 = myMaterialCurveRef4->GetPath();
				}
			});
	}
	break;
	case VFX::ParticleStartVariables::MAXPARTICLES:
	{
		VFXEditorPropertyWindow::Instance().Set([&, aVariable]()
			{
				ImGui::Text(VFX::ParticleStartVariables::titles[static_cast<unsigned int>(std::log2(static_cast<unsigned int>(aVariable)))]);
				ImGui::InputInt("Max Particles", &myParticleDesc.maxParticles);
			});
	}
	break;
	default:
		VFXEditorPropertyWindow::Instance().Set([&, aVariable]()
			{
				ImGui::Text((std::string("Variable error setting ") + VFX::ParticleStartVariables::titles[static_cast<unsigned int>(std::log2(static_cast<unsigned int>(aVariable)))]).c_str());
				ImGui::Text("Not implemented");
			});
	}

}

void Engine::VFXEditorParticlesEvent::SetPropertyVariable(VFX::ParticleUpdateVariables::eVariables aVariable)
{
	switch (aVariable)
	{
	case VFX::ParticleUpdateVariables::GRAVITY:
		VFXEditorPropertyWindow::Instance().Set([&, aVariable]()
			{
				ImGui::Text(VFX::ParticleUpdateVariables::titles[static_cast<unsigned int>(std::log2(static_cast<unsigned int>(aVariable)))]);
				ImGui::InputFloat3("Gravity", &myParticleDesc.gravity.x);
			});
		break;
	case VFX::ParticleUpdateVariables::ROTATE:
		VFXEditorPropertyWindow::Instance().Set([&, aVariable]()
			{
				ImGui::Text(VFX::ParticleUpdateVariables::titles[static_cast<unsigned int>(std::log2(static_cast<unsigned int>(aVariable)))]);
				ImGui::DragFloat("Speed", &myParticleDesc.rotate.speed);
				ImGui::DragFloat("Speed Modifier", &myParticleDesc.rotate.modifier);
			});
		break;
	case VFX::ParticleUpdateVariables::DRAG:
		VFXEditorPropertyWindow::Instance().Set([&, aVariable]()
			{
				ImGui::Text(VFX::ParticleUpdateVariables::titles[static_cast<unsigned int>(std::log2(static_cast<unsigned int>(aVariable)))]);
				ImGui::DragFloat("Drag Constant", &myParticleDesc.drag.drag);
				ImGui::DragFloat("Air Density", &myParticleDesc.drag.airDensity);
			});
		break;
	case VFX::ParticleUpdateVariables::ATTRACT:
		VFXEditorPropertyWindow::Instance().Set([&, aVariable]()
			{
				ImGui::Text(VFX::ParticleUpdateVariables::titles[static_cast<unsigned int>(std::log2(static_cast<unsigned int>(aVariable)))]);

				const char* attractionLocations[] = { "Emitter Position", "Global" };
				int settingIndex = static_cast<int>(myParticleDesc.attraction.setting);
				if (ImGui::Combo("Emit Direction", &settingIndex, attractionLocations, IM_ARRAYSIZE(attractionLocations)))
				{
					myParticleDesc.attraction.setting = static_cast<VFX::eAttractionSetting>(settingIndex);
				}
				
				ImGui::DragFloat("Force", &myParticleDesc.attraction.force);
				ImGui::DragFloat("Range", &myParticleDesc.attraction.range);
				ImGui::DragFloat("Destroy Range", &myParticleDesc.attraction.collectRange);
			});
		break;
	case VFX::ParticleUpdateVariables::EMIT_ON_MOVE:
		VFXEditorPropertyWindow::Instance().Set([&, aVariable]()
			{
				ImGui::Text(VFX::ParticleUpdateVariables::titles[static_cast<unsigned int>(std::log2(static_cast<unsigned int>(aVariable)))]);

				ImGui::DragFloat("Units Per Particle", &myParticleDesc.emitOnMove.units);
				ImGui::DragFloat("Units Modifier", &myParticleDesc.emitOnMove.modifier);
			});
		break;
	case VFX::ParticleUpdateVariables::NOISE:
		VFXEditorPropertyWindow::Instance().Set([&, aVariable]()
			{
				ImGui::Text(VFX::ParticleUpdateVariables::titles[static_cast<unsigned int>(std::log2(static_cast<unsigned int>(aVariable)))]);

				const char* attractionLocations[] = { "Soft", "Hard" };
				int settingIndex = static_cast<int>(myParticleDesc.noise.setting);
				if (ImGui::Combo("Noise Type", &settingIndex, attractionLocations, IM_ARRAYSIZE(attractionLocations)))
				{
					myParticleDesc.noise.setting = static_cast<VFX::eNoiseType>(settingIndex);
				}

				ImGui::DragFloat3("Noise Force", &myParticleDesc.noise.force.x);
				ImGui::DragFloat3("Noise Scroll Speed", &myParticleDesc.noise.scrollSpeed.x);
				ImGui::DragFloat("Noise Size", &myParticleDesc.noise.size);
			});
		break;
	case VFX::ParticleUpdateVariables::WRAP:
		VFXEditorPropertyWindow::Instance().Set([&, aVariable]()
			{
				ImGui::Text(VFX::ParticleUpdateVariables::titles[static_cast<unsigned int>(std::log2(static_cast<unsigned int>(aVariable)))]);

				ImGui::Checkbox("Wrap Position", &myParticleDesc.wrapPosition);
			});
		break;
	case VFX::ParticleUpdateVariables::ACCELERATION:
	{
		if (!myAccelerationCurve || myAccelerationCurve->GetPath() != myParticleDesc.acceleration.curve)
		{
			myAccelerationCurve = GResourceManager->CreateRef<AnimationCurveResource>(myParticleDesc.acceleration.curve);
		}
		myAccelerationCurve->Load();

		VFXEditorPropertyWindow::Instance().Set([&, aVariable]()
			{
				ImGui::Text(VFX::ParticleStartVariables::titles[static_cast<unsigned int>(std::log2(static_cast<unsigned int>(aVariable)))]);

				VFXEditorHelpers::DrawSetting(myParticleDesc.acceleration.settings, VFX::eLERP);

				ImGui::InputFloat("Acceleration", &myParticleDesc.acceleration.speed);

				if (myParticleDesc.acceleration.settings & VFX::ePropertySetting::eLERP)
				{
					if (myImguiReflector.Reflect(
						myAccelerationCurve, "Acceleration Curve")
						& ReflectorResult_Changed)
					{
						myParticleDesc.acceleration.curve = myAccelerationCurve->GetPath();
					}
				}
			});
	}
	break;
	default:
		VFXEditorPropertyWindow::Instance().Set([&, aVariable]()
			{
				ImGui::Text((std::string("Variable error setting ") + VFX::ParticleUpdateVariables::titles[static_cast<unsigned int>(std::log2(static_cast<unsigned int>(aVariable)))]).c_str());
				ImGui::Text("Not implemented");
			});
	}
}

const VFX::ParticleEmitterDescription& Engine::VFXEditorParticlesEvent::GetDescription()
{
	myEmitterDesc.particleDescription = myParticleDesc;

	return myEmitterDesc;
}

void Engine::VFXEditorParticlesEvent::SetDescription(const VFX::ParticleEmitterDescription& aDescription)
{
	myEmitterDesc = aDescription;
	myParticleDesc = aDescription.particleDescription;
}
