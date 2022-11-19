#include "pch.h"
#include "SpotLightComponent.h"
#include "../GameObject.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Renderer/Scene/RendererScene.h"
#include "../Engine.h"
#include "Engine/Reflection/Reflector.h"
#include "Engine/Renderer/GraphicsEngine.h"
#include "Engine/Renderer/Texture/FullscreenTextureFactory.h"

Engine::SpotLightComponent::SpotLightComponent(
	GameObject* aGameObject,
	const bool aCastShadows)
	: Component(aGameObject)
{
	myIsCastingShadows = aCastShadows;

	InitLight();
}

Engine::SpotLightComponent::~SpotLightComponent()
{
}

void Engine::SpotLightComponent::InitLight()
{
	if (myIsCastingShadows)
	{
		//const auto resolution = Vec2ui(128, 128);

		// In radians, max pi/2
		// meaning, max FOV of 180.f bcuz the pi/2 is the angle from 
		// a line straight out from spotlight
		// NOTE(filip): the shadow becomes very weird looking when it approaches 180 deg fov
		float fovDegrees = (myOuterAngle * 2.f) * Math::RadToDeg;

		// Must clamp, otherwise weird results
		if (fovDegrees > 160.f)
		{
			fovDegrees = 160.f;
		}

		myShadowCamera.InitPerspective(fovDegrees, myShadowMapResolution.CastTo<float>(), 1.f, 2000.f);

		myShadowMap = GetEngine().
			GetGraphicsEngine().
			GetFullscreenTextureFactory().
			CreateShadowMap(myShadowMapResolution.CastTo<unsigned int>());
	}
}

void Engine::SpotLightComponent::SetColor(const Vec3f& aColor)
{
	myColor = aColor;
}

void Engine::SpotLightComponent::SetRange(const float aRange)
{
	myRange = aRange;
}

void Engine::SpotLightComponent::SetInnerAngle(const float aInnerAngle)
{
	myInnerAngle = aInnerAngle;
}

void Engine::SpotLightComponent::SetOuterAngle(const float aOuterAngle)
{
	myOuterAngle = aOuterAngle;
}

void Engine::SpotLightComponent::SetIntensity(const float aIntensity)
{
	myIntensity = aIntensity;
}

const Vec3f& Engine::SpotLightComponent::GetColor() const
{
	return myColor;
}

float Engine::SpotLightComponent::GetRange() const
{
	return myRange;
}

float Engine::SpotLightComponent::GetInnerAngle() const
{
	return myInnerAngle;
}

float Engine::SpotLightComponent::GetOuterAngle() const
{
	return myOuterAngle;
}

float Engine::SpotLightComponent::GetIntensity() const
{
	return myIntensity;
}

void Engine::SpotLightComponent::Execute(eEngineOrder aOrder)
{
}

void Engine::SpotLightComponent::Render()
{
	// mySpotLight.SetPosition(myGameObject->GetTransform().GetPositionWorld());
	// mySpotLight.SetRotation(myGameObject->GetTransform().GetRotationWorld());

	if (myIsCastingShadows)
	{
		myShadowCamera.SetPosition(myGameObject->GetTransform().GetPosition());
		myShadowCamera.SetRotationQuaternion(myGameObject->GetTransform().GetRotation());
	}

	myGameObject->GetScene()->GetRendererScene().RenderSpotLight(*this);

	const auto debugDrawColor = C::Vector4f(GetColor().x, GetColor().y, GetColor().z, 1.f);

	myGameObject->GetScene()->GetRendererScene().GetDebugDrawer().DrawSphere3D(
		DebugDrawFlags::Spotlights,
		myGameObject->GetTransform().GetPosition(),
		GetRange(),
		0.f,
		debugDrawColor);
}

void Engine::SpotLightComponent::Reflect(Reflector& aReflector)
{
	Component::Reflect(aReflector);

	aReflector.Reflect(myColor, "Color", ReflectionFlags_IsColor);
	aReflector.SetNextItemRange(0.f, 1000000.f);
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

	aReflector.SetNextItemSpeed(0.01f);
	aReflector.SetNextItemRange(0.f, Math::PI * 0.5f);
	aReflector.Reflect(myInnerAngle, "InnerAngle");
	aReflector.SetNextItemSpeed(0.01f);
	aReflector.SetNextItemRange(0.f, Math::PI * 0.5f);
	aReflector.Reflect(myOuterAngle, "OuterAngle");
	aReflector.SetNextItemSpeed(1000.f);
	aReflector.SetNextItemRange(0.f, 10000000000.f);
	aReflector.Reflect(myIntensity, "Intensity");

	const auto castShadowResult = aReflector.Reflect(myIsCastingShadows, "Cast Shadows");

	if (castShadowResult & ReflectorResult_Changed)
	{
		InitLight();
	}

	aReflector.Reflect(myShadowMapResolution, "ShadowMapResolution");
}

Engine::Camera& Engine::SpotLightComponent::GetCamera()
{
	return myShadowCamera;
}

Engine::FullscreenTexture& Engine::SpotLightComponent::GetShadowMap()
{
	return myShadowMap;
}

bool Engine::SpotLightComponent::IsCastingShadows() const
{
	return myIsCastingShadows;
}

const Vec2f& Engine::SpotLightComponent::GetShadowMapResolution() const
{
	return myShadowMapResolution;
}
