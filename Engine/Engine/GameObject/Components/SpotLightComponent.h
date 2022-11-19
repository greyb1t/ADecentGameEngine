#pragma once

#include "Component.h"

#include "Engine/Renderer/Texture/FullscreenTexture.h"
#include "Engine/Renderer/Camera/Camera.h"

namespace Engine
{
	class SpotLight;
}

namespace Engine
{
	class SpotLightComponent : public Component
	{
	public:
		COMPONENT(SpotLightComponent, "SpotLightComponent");

		SpotLightComponent() = default;
		SpotLightComponent(GameObject* aGameObject, const bool aCastShadows);
		~SpotLightComponent();

		void InitLight();

		void SetColor(const Vec3f& aColor);
		void SetRange(const float aRange);
		void SetInnerAngle(const float aInnerAngle);
		void SetOuterAngle(const float aOuterAngle);
		void SetIntensity(const float aIntensity);

		const Vec3f& GetColor() const;
		float GetRange() const;
		float GetInnerAngle() const;
		float GetOuterAngle() const;
		float GetIntensity() const;

		void Execute(eEngineOrder aOrder) override;
		void Render() override;

		void Reflect(Reflector& aReflector) override;

		Camera& GetCamera();
		FullscreenTexture& GetShadowMap();

		bool IsCastingShadows() const;

		const Vec2f& GetShadowMapResolution() const;

	private:
		Vec3f myColor = Vec3f(1.f, 1.f, 1.f);
		float myRange = 600.f;
		float myInnerAngle = 0.f;
		float myOuterAngle = 1.f;
		float myIntensity = 100000.f;

		bool myIsCastingShadows = false;
		Camera myShadowCamera;
		FullscreenTexture myShadowMap;
		Vec2f myShadowMapResolution = Vec2f(1024.f, 1024.f);
	};
}