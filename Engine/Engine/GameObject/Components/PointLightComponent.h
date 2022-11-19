#pragma once

#include "Component.h"
#include "Engine\Renderer\Texture\FullscreenTextureCube.h"
#include "Engine\Renderer\Camera\Camera.h"

namespace Engine
{
	class PointLightComponent : public Component
	{
	public:
		COMPONENT(PointLightComponent, "PointLightComponent");

		PointLightComponent() = default;
		PointLightComponent(GameObject* aGameObject, const bool aCastShadows);
		~PointLightComponent();

		void InitLight();

		void SetColor(const Vec3f& aColor);
		void SetRange(const float aRange);
		void SetIntensity(const float aIntensity);

		float GetRange() const;
		const Vec3f& GetColor() const;
		float GetIntensity() const;

		void Execute(eEngineOrder aOrder) override;
		void Render() override;

		void Reflect(Reflector& aReflector) override;

		std::array<Camera, 6>& GetCameras();
		FullscreenTextureCube* GetShadowMap();

		bool IsCastingShadows() const;

	private:
		Vec3f myColor = Vec3f(1.f, 1.f, 1.f);
		float myRange = 600.f;
		float myIntensity = 100000.f;

		bool myIsCastingShadows = false;
		std::array<Camera, 6> myShadowCameras = {};
		FullscreenTextureCube myShadowMap;
	};
}