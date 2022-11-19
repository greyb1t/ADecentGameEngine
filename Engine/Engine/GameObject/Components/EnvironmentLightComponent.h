#pragma once

#include "Component.h"
// #include "Engine/Renderer/Lights/EnvironmentLight.h"

#include "Engine/ResourceManagement/ResourceRef.h"
#include "Engine/Renderer/Camera/Camera.h"
#include "Engine/Renderer/Texture/FullscreenTexture.h"
#include "Engine/Renderer/Texture/FullscreenTextureCube.h"

namespace Engine
{
	constexpr int NumCascades = 4;
	// Used to calculate an offset at the cascades to ensure we sample the correct pixels when smoothing the shadows.
	constexpr int SoftShadowPixelSize = 1;
	constexpr int ShadowMapSize = 2048;

	class EnvironmentLightComponent : public Component
	{
		struct CascadeData
		{
			float nearPlane;
			float farPlane;
			Vec2f resolution;
		};

	public:
		COMPONENT(EnvironmentLightComponent, "EnvironmentLightComponent");

		EnvironmentLightComponent() = default;
		EnvironmentLightComponent(GameObject* aGameObject, const std::string& aCubemapPath);
		EnvironmentLightComponent(const EnvironmentLightComponent&) = default;

		void InitLight();

		void Execute(eEngineOrder aOrder) override;
		void Render() override;

		void Reflect(Reflector& aReflector) override;

		Vec4f GetDirection() const;

		void SetCubemap1(TextureRef aTexture);
		void SetCubemap2(TextureRef aTexture);

		void SetColor(const Vec3f& aColor);
		void SetColorIntensity(const float aIntensity);
		void SetCubemapIntensity(const float aIntensity);

		const Vec3f& GetColor() const;
		float GetColorIntensity() const;
		float GetCubemapIntensity() const;
		Camera& GetCamera();
		FullscreenTexture& GetShadowMap();

		TextureRef GetCubemap();
		TextureRef GetCubemap2();

		bool IsCastingShadows() const;

		// -1 is cubemap, 1 is cubemap2
		void SetCubemapInterpolationValue(const float aValue);
		float GetCubemapInterpolationValue() const;

		// Cascades
		const int GetNumCascades() const;
		const Mat4f& GetCascadeProjection(const int aCascadeIndex) const;
		const Mat4f& GetCascadeView(const int aCascadeIndex) const;
		const float GetCascadeBound(const int aCascadeIndex) const;
		const CascadeData& GetCascadeData(const int aCascadeIndex) const;

		FullscreenTexture& GetCascadeTexture(const int aCascadeIndex);
		Mat4f GetShadowView();

		float GetFolliageCullingDistance() const;

	private:
		void CalculateCascades();
		std::vector<Vec4f> GetFrustumCornersWorldSpace(const Mat4f& aProjection, const Mat4f& aView);

	private:
		TextureRef myCubemap;
		TextureRef myCubemap2;

		// -1 is cubemap 1, 1 is cubemap 2
		float myCubemapInterpolationValue = -1.f;

		float myCubemapIntensity = 1.f;

		Vec3f myColor = { 1.f, 1.f, 1.f };
		float myColorIntensity = 1.f;

		bool myIsCastingShadows = true;
		Camera myShadowCamera;

		// This can be removed when using cascaded shadows
		FullscreenTexture myShadowMap;

		const std::array<float, NumCascades> myCascadeBounds = { 0.03f, 0.1f, 0.3f, 1.0f };
		std::array<FullscreenTexture, NumCascades> myCascadeShadowMaps;

		std::array<Mat4f, NumCascades> myCascadeProjections;
		std::array<Mat4f, NumCascades> myCascadeViews;
		
		std::array<CascadeData, NumCascades> myCascadeData;

		// This is supposed to be in folliageculler component but static didnt work and idk what to do. // William :)
		float myFolliageCullingDistance = 10000.f;

		bool myUpdateCascades = true;
	};
}
