#include "pch.h"
#include "PostProcessComponent.h"
#include "Engine/Engine.h"
#include "Engine/Renderer/GraphicsEngine.h"
#include "Engine/Renderer/Texture/FullscreenTextureFactory.h"
#include "Engine/ResourceManagement/ResourceManager.h"
#include "Engine/ResourceManagement/Resources/TextureResource.h"
#include "Engine/Reflection/Reflector.h"
#include "Engine/GameObject/GameObject.h"

Engine::PostProcessComponent::PostProcessComponent(
	GameObject* aGameObject)
	: Component(aGameObject)
{
}

void Engine::PostProcessComponent::Render()
{
	auto& pp = myGameObject->GetScene()->GetRendererScene().GetPostProcessData();

	if (myFogEnabled)
	{
		pp.myFogData = myFogData;
	}

	pp.myFXAASettings = myFXAASettings;
	pp.myBloomSettings = myBloomSettings;
	pp.myTonemappingData = myTonemappingSettings;
	// pp.mySSAOSettings = mySSAOSettings;
}

void Engine::PostProcessComponent::Reflect(Reflector& aReflector)
{
	aReflector.Reflect(myFogEnabled, "Fog");

	if (myFogEnabled)
	{
		aReflector.Header("Fog");

		aReflector.Reflect(myFogData.myColor, "Fog Color", ReflectionFlags_IsColor);
		aReflector.Reflect(myFogData.myStart, "Fog Start Distance");
		aReflector.Reflect(myFogData.myEnd, "Fog End Distance");

		aReflector.SetNextItemRange(0.f, 1.f);
		aReflector.SetNextItemSpeed(0.01f);
		aReflector.Reflect(myFogData.myFogStrength, "Fog Strength");

		aReflector.Reflect(myFogData.myHeightFogOffset, "Height Fog Offset");
		aReflector.Reflect(myFogData.myHeightFogStrength, "Height Fog Strength");

		aReflector.Separator();
	}

	aReflector.Reflect(myFXAASettings, "FXAA");

	aReflector.Reflect(myBloomSettings, "Bloom");

	aReflector.Reflect(myTonemappingSettings, "Tone mapping");

	// aReflector.Reflect(mySSAOSettings, "SSAO");
}

Engine::BloomSettings2& Engine::PostProcessComponent::GetBloomSettingsRef()
{
	return myBloomSettings;
}

Engine::FogData& Engine::PostProcessComponent::GetFogSettingsRef()
{
	return myFogData;
}

Engine::TonemappingSettings& Engine::PostProcessComponent::GetTonemappingSettingsRef()
{
	return myTonemappingSettings;
}
