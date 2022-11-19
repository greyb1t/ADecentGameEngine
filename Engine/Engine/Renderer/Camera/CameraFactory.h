#pragma once

#include "Camera.h"

namespace Engine
{
	class WindowHandler;

	class CameraFactory
	{
	public:
		CameraFactory(WindowHandler& aWindowHandler);
		CameraFactory(const CameraFactory&) = delete;
		~CameraFactory();

		// OMG WHAT THE FUCK
		Camera CreatePlayerCamera(const float aHorizontalFovDegrees);

		Camera CreateUICamera();

		Camera CreateEnvironmentLightCamera(const Vec2ui& aResolution);
		Camera CreateSpotlightCamera(const Vec2ui& aResolution);
		Camera CreatePointlightCamera(const Vec2ui& aResolution);

	private:
		WindowHandler& myWindowHandler;
	};
}
