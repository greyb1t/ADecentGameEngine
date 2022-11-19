#pragma once

namespace Engine
{
	class EnvironmentLightComponent;
}

namespace Engine
{
	// All shaders are using the same framebuffer and it is only modified ONCE
	// at the beginning of a frame
	// Register: b0
	class Directx11Framework;
	class Camera;
	class EnvironmentLight;
	struct FogData;

	class FrameBufferTempName
	{
	public:
		bool Init(Directx11Framework& aFramework);

		void UpdateCamera(const Camera& aCamera);
		void UpdateEnvironmentLight(const EnvironmentLightComponent& aEnvironmentLight);
		void UpdateFog(const FogData& aFogData);
		void UpdateShadowStrength(const float aShadowStrength);
		void UpdatePlayerPos(const Vec3f& aPlayerPos);

		void Bind();

		void DrawDebugMenu();

	private:
		void MapToGPU();

	private:
		Directx11Framework* myFramework = nullptr;

		struct BufferData
		{
			Mat4f myToCamera;
			Mat4f myFromCameraToWorld;

			Mat4f myToProjection;
			Mat4f myFromProjectionToCamera;

			C::Vector4f myCameraPosition;
			C::Vector4f myToDirectionalLightDirection;
			C::Vector4f myDirectionalLightColorAndIntensity;
			C::Vector4f myFogColor = { 10.f / 255.f, 14.f / 255.f, 46.f / 255.f, 1.f };

			float myFogStart = 0.f;
			float myFogEnd = 26800.f;
			float myFogStrength = 1.f;
			float myCubemapIntensity = 1.f;

			float myCubemapInterpolationValue = -1.f;
			float myCameraNearPlane = 0.f;
			float myCameraFarPlane = 0.f;
			float myShadowStrength = 1.f;

			float myHeightFogOffset = 0.f;
			float myHeightFogStrength = 10.f;

			float myThrash[2];

			Vec3f myPlayerPosition;
			float myThrash22;
		} myBufferData;

		ID3D11Buffer* myBuffer = nullptr;
	};

}