#pragma once

#include "Component.h"
#include "Engine\Renderer\Texture\FullscreenTextureCube.h"
#include "Engine\ResourceManagement\ResourceRef.h"
#include "Engine\Renderer\Camera\Camera.h"
#include "Engine\Renderer\Texture\CubemapTexture.h"

namespace Engine
{
	class ReflectionCaptureComponent : public Component
	{
	public:
		COMPONENT_COPYABLE(ReflectionCaptureComponent, "Reflection Capture");

		void Start() override;

		void Execute(eEngineOrder aOrder) override;

		void Render() override;

		void Reflect(Reflector& aReflector) override;

		Camera& GetCamera(const int aSide);
		FullscreenTextureCube& GetCubemapTexture();
		CubemapTexture& GetCubemapTextureSmooth();

	private:
		void InitMoney();

	private:
		// The box size
		// Vec3f mySize = Vec3f(1.f, 1.f, 1.f);

		bool myIsDirty = false;

		FullscreenTextureCube myGeneratedCubemap;
		TextureRef myGeneratedCubemapRef;
		TextureRef myGeneratedCubemapRefPrev;
		bool myGenerateCubemap = false;
		std::array<Camera, 6> myGeneratedCubemapCams = { };
		bool myToggleMainCubemap = false;

		CubemapTexture myCubemapTextureSmooth;
		TextureRef mySmoothCubemapRef;
	};
}