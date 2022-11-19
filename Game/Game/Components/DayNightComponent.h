#pragma once

#include "Engine\GameObject\Components\Component.h"


class DayNightComponent : public Component
{
	COMPONENT(DayNightComponent, "DayNightComponent");

	struct EnvironmentLightSettings : Engine::Reflectable
	{
		std::vector<Vec3f> myColors{};

		AnimationCurveRef myIntensity;
		AnimationCurveRef myCubeIntensity;

		AnimationCurveRef myRotationController;
		AnimationCurveRef myColorController;

		void Reflect(Engine::Reflector& aReflector) override
		{
			aReflector.Reflect(myColors, "Colors", Engine::ReflectionFlags_IsColor);

			aReflector.Reflect(myIntensity, "Intensity");
			aReflector.Reflect(myCubeIntensity, "Cube Intensity");

			aReflector.Header("Controllers");
			aReflector.Reflect(myRotationController, "Rotation Controller");
			aReflector.Reflect(myColorController, "Color Controller");
		}
	};

	struct BloomSettings : Engine::Reflectable
	{
		AnimationCurveRef myIntensity;
		AnimationCurveRef mySampleScale;
		AnimationCurveRef myThreshold;
		AnimationCurveRef mySoftThreshold;
		AnimationCurveRef myIterationsOffset;

		void Reflect(Engine::Reflector& aReflector) override
		{
			aReflector.Reflect(myIntensity, "Intensity");
			aReflector.Reflect(mySampleScale, "Sample Scale");
			aReflector.Reflect(myThreshold, "Threshold");
			aReflector.Reflect(mySoftThreshold, "Soft Threshold");
			aReflector.Reflect(myIterationsOffset, "Iterations Offset");
		}
	};

	struct FogSettings : Engine::Reflectable
	{
		std::vector<Vec3f> myColors{};
		AnimationCurveRef myColorController;

		AnimationCurveRef myStartDistance;
		AnimationCurveRef myEndDistance;
		AnimationCurveRef myStrength;
		AnimationCurveRef myHeightOffset;
		AnimationCurveRef myHeightStrength;

		void Reflect(Engine::Reflector& aReflector) override
		{
			aReflector.Reflect(myColors, "Colors", Engine::ReflectionFlags_IsColor);
			aReflector.Reflect(myColorController, "Color Controller");
			aReflector.Reflect(myStartDistance, "Start Distance");
			aReflector.Reflect(myEndDistance, "End Distance");
			aReflector.Reflect(myStrength, "Strength");
			aReflector.Reflect(myHeightOffset, "Height Fog Offset");
			aReflector.Reflect(myHeightStrength, "Height Fog Strenght");
		}
	};


public:
	DayNightComponent();

	void Reflect(Engine::Reflector& aReflector) override;

	void Execute(Engine::eEngineOrder aOrder) override;
	void Render() override;

private:
	void UpdateEnvironment();
	void DoLightRotation();

private:
	bool myIsDay = true;
	float myTimeProgress = 0.f;

	float myDayDuration = 25.f;
	float myNightDuration = 25.f;

	EnvironmentLightSettings myEnvLightSettings{};
	BloomSettings myBloomSettings{};
	FogSettings myFogSettings{};
	
	std::vector<Vec3f> myEnvLightRotations =
	{
		{-15.f, 0.f, 0.f},
		{0.f, -90.f, 60.f},
		{15.f, -180.f, 0.f},
		{0.f, 90.f, -60.f},
		{-15.f, 0.f, 0.f}
	};

	//std::vector<Vec3f> myEnvLightRotations =
	//{
	//	{0, 0, 0},
	//	{-90, 0, 0},
	//	{-90, 0, 0},
	//	{-180, 0, 0}
	//};
};