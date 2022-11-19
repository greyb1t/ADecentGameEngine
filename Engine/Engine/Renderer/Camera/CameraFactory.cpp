#include "pch.h"
#include "CameraFactory.h"
#include "..\WindowHandler.h"

namespace Engine
{
	CameraFactory::CameraFactory(WindowHandler& aWindowHandler)
		: myWindowHandler(aWindowHandler)
	{
	}

	CameraFactory::~CameraFactory()
	{
	}

	Camera CameraFactory::CreatePlayerCamera(const float aHorizontalFovDegrees)
	{
		Camera camera;

		camera.InitPerspective(
			aHorizontalFovDegrees,
			myWindowHandler.GetRenderingSize().CastTo<float>(),
			5.f,
			25000.f);

		return camera;
	}

	Camera CameraFactory::CreateUICamera()
	{
		Camera camera;

		camera.InitOrthographic(myWindowHandler.GetTargetSize().CastTo<float>(), 1.f, 25000.f);

		return camera;
	}

	Camera CameraFactory::CreateEnvironmentLightCamera(const Vec2ui& aResolution)
	{
		Camera camera;

		camera.InitOrthographic(aResolution.CastTo<float>(), 1.f, 25000.f);

		return camera;
	}

	Camera CameraFactory::CreateSpotlightCamera(const Vec2ui& aResolution)
	{
		Camera camera;

		// same fov as main camera?
		// same resolution as main camera and window baby
		camera.InitPerspective(90.f, aResolution.CastTo<float>(), 1.f, 25000.f);

		return camera;
	}

	Camera CameraFactory::CreatePointlightCamera(const Vec2ui& aResolution)
	{
		Camera camera;

		// Resolution must be same width and height, otherwise its no cube
		// FOV must be 90 to cover an entire cube side
		camera.InitPerspective(90.f, aResolution.CastTo<float>(), 1.f, 25000.f);

		return camera;
	}
}