#pragma once

#include "Component.h"
#include "Engine\Renderer\PostProcessing\Fog\Fog.h"
#include "Engine\Renderer\PostProcessing\TonemappingSettings.h"
#include "Engine\Renderer\PostProcessing\Bloom\BloomSettings.h"
#include "Engine\Renderer\PostProcessing\SSAO\SSAOSettings.h"
#include "Engine\Renderer\PostProcessing\FXAASettings.h"

namespace Engine
{
	class PostProcessComponent : public Component
	{
	public:
		COMPONENT_SINGLETON(PostProcessComponent, "Post Process");

		PostProcessComponent() = default;
		PostProcessComponent(GameObject* aGameObject);

		void Render() override;

		void Reflect(Reflector& aReflector) override;

		BloomSettings2& GetBloomSettingsRef();
		FogData& GetFogSettingsRef();
		TonemappingSettings& GetTonemappingSettingsRef();

	private:
		bool myFogEnabled = true;
		FogData myFogData;

		FXAASettings myFXAASettings;

		BloomSettings2 myBloomSettings;

		TonemappingSettings myTonemappingSettings;

		// SSAOSettings mySSAOSettings;
	};
}
