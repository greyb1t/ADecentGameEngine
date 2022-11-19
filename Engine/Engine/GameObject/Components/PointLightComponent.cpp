#include "pch.h"
#include "PointLightComponent.h"
#include "..\Engine.h"
#include "Engine/Renderer\GraphicsEngine.h"
#include "..\GameObject.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Renderer/Scene/RendererScene.h"
#include "..\DebugManager\DebugDrawer.h"
#include "Engine/Reflection/Reflector.h"
#include "Engine/Renderer/Texture/FullscreenTextureFactory.h"

Engine::PointLightComponent::PointLightComponent(GameObject* aGameObject, const bool aCastShadows)
	: Component(aGameObject)
{
	myIsCastingShadows = aCastShadows;

	// myPointLight = GetEngine().GetGraphicsEngine().GetLightFactory().CreatePointLight(aCastShadows);

	// Set default values
	{
		SetColor(Vec3f(1.f, 1.f, 1.f));
		SetRange(400.f);
		SetIntensity(160000.f);
	}

	InitLight();
}

Engine::PointLightComponent::~PointLightComponent()
{
}

void Engine::PointLightComponent::InitLight()
{
	if (myIsCastingShadows)
	{
		auto& shadowCameras = myShadowCameras;

		for (int sideIndex = 0; sideIndex < 6; ++sideIndex)
		{
			Vec3f rot;

			const float deg90 = Math::PI * 0.5f;

			// NOTE: The order for these are VERY important, find the order
			// by searching google "directx 11 cube map faces"
			// and an image pops up from d3d9 that shows it
			// https://docs.microsoft.com/en-us/windows/win32/direct3d9/cubic-environment-mapping
			switch (sideIndex)
			{
			case 0:
				// +X
				rot = Vec3f(0, -deg90, 0);
				break;
			case 1:
				// -X
				rot = Vec3f(0, deg90, 0);
				break;
			case 2:
				// +Y
				rot = Vec3f(deg90, 0, 0);
				break;
			case 3:
				// -Y
				rot = Vec3f(-deg90, 0, 0);
				break;
			case 4:
				// +Z
				rot = Vec3f(0, 0, 0);
				break;
			case 5:
				// -Z
				rot = Vec3f(0, deg90 * 2.f, 0);
				break;
			default:
				assert(false && "unhandled");
				break;
			}

			const auto resolution = Vec2ui(128, 128);

			// Resolution must be same width and height, otherwise its no cube
			// FOV must be 90 to cover an entire cube side
			shadowCameras[sideIndex].InitPerspective(
				90.f, resolution.CastTo<float>(), 1.f, 2500.f);

			shadowCameras[sideIndex].SetRotationEuler(rot);
		}

		myShadowMap
			= GetEngine().GetGraphicsEngine().GetFullscreenTextureFactory().CreateShadowMapCube(
				{ 128, 128 });
	}
}

void Engine::PointLightComponent::SetColor(const Vec3f& aColor)
{
	myColor = aColor;
}

void Engine::PointLightComponent::SetRange(const float aRange)
{
	myRange = aRange;
}

void Engine::PointLightComponent::SetIntensity(const float aIntensity)
{
	myIntensity = aIntensity;
}

float Engine::PointLightComponent::GetRange() const
{
	return myRange;
}

const Vec3f& Engine::PointLightComponent::GetColor() const
{
	return myColor;
}

float Engine::PointLightComponent::GetIntensity() const
{
	return myIntensity;
}

void Engine::PointLightComponent::Execute(eEngineOrder aOrder)
{
}

void Engine::PointLightComponent::Render()
{
	// TODO: Dont do this every frame
	// myPointLight.SetPosition(myGameObject->GetTransform().GetPositionWorld());

	// TODO: Find a better way to do this!
	if (myIsCastingShadows)
	{
		for (auto& cam : myShadowCameras)
		{
			cam.SetPosition(myGameObject->GetTransform().GetPosition());
		}
	}

	myGameObject->GetScene()->GetRendererScene().RenderPointLight(*this);

	const auto debugDrawColor = C::Vector4f(GetColor().x, GetColor().y, GetColor().z, 1.f);

	myGameObject->GetScene()->GetRendererScene().GetDebugDrawer().DrawSphere3D(
		DebugDrawFlags::Pointlights,
		myGameObject->GetTransform().GetPosition(),
		20.f,
		0.f,
		debugDrawColor);

	myGameObject->GetScene()->GetRendererScene().GetDebugDrawer().DrawSphere3D(
		DebugDrawFlags::Pointlights,
		myGameObject->GetTransform().GetPosition(),
		GetRange(),
		0.f,
		debugDrawColor);

	myGameObject->GetScene()->GetRendererScene().GetDebugDrawer().DrawSphere3D(
		DebugDrawFlags::Pointlights,
		myGameObject->GetTransform().GetPosition(),
		20.f,
		0.f,
		{ 1.f, 0.f, 0.f, 1.f },
		false);
}

void Engine::PointLightComponent::Reflect(Reflector& aReflector)
{
	Component::Reflect(aReflector);

	aReflector.Reflect(myColor, "Color", ReflectionFlags_IsColor);

	if (aReflector.Reflect(myRange, "Range") & ReflectorResult_HoveringOverItem)
	{
		if (myGameObject)
		{
			myGameObject->GetScene()->GetRendererScene().GetDebugDrawer().DrawSphere3D(
				DebugDrawFlags::Always,
				myGameObject->GetTransform().GetPosition(),
				GetRange(),
				0.f);
		}
	}

	aReflector.Reflect(myIntensity, "Intensity");

	const auto castShadowResult = aReflector.Reflect(myIsCastingShadows, "Cast Shadows");

	if (castShadowResult & ReflectorResult_Changed)
	{
		InitLight();
	}
}

std::array<Engine::Camera, 6>& Engine::PointLightComponent::GetCameras()
{
	return myShadowCameras;
}

Engine::FullscreenTextureCube* Engine::PointLightComponent::GetShadowMap()
{
	return &myShadowMap;
}

bool Engine::PointLightComponent::IsCastingShadows() const
{
	return myIsCastingShadows;
}
