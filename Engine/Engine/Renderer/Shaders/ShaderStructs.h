#pragma once

#include "Engine/Renderer/Lights/LightConstants.h"
#include "ShaderConstants.h"

namespace Engine
{
	struct AnimationBufferData
	{
		Mat4f myBones[ShaderConstants::MaxBoneCount];
	};

	struct SpotLightBufferData
	{
		Mat4f myLightView;
		Mat4f myLightProjection;
		C::Vector4f myPosition;
		C::Vector4f myDirection;
		C::Vector4f myColorAndIntensity;
		float myRange = 0.f;
		float myInnerAngle = 0.f;
		float myOuterAngle = 0.f;
		float trash;
		int myIsCastingShadows = 0;
		Vec2f myShadowMapResolution;
		int myShit;
		// int myShit[3] = { };
	};

	struct MaterialData
	{
		float myEmissionIntensity = 1.f;
		Vec3f myTrash;
	};

	// THIS MUST MATCH ObjectBuferData_ForwardRenderer in its layout
	struct ObjectBuferData_DeferredRenderer
	{
		Mat4f myToWorld;

		Vec4f myMainColor;
		Vec4f myAdditiveColor;

		int myHasBones;
		int myTrash[3];
	};

	// THIS MUST MATCH ObjectBuferData_DeferredRenderer in its layout
	struct ObjectBuferData_ForwardRenderer
	{
		Mat4f myToWorld;

		Vec4f myMainColor;
		Vec4f myAdditiveColor;

		int myHasBones;
		int myTrash1;

		unsigned int myPointLightCount = 0;
		unsigned int mySpotLightCount = 0;

		struct PointLightData
		{
			C::Vector4f myPosition;
			C::Vector4f myColorAndIntensity;
			float myRange = 0.f;
			Vec3f garbage;
		} myPointLights[MaxPointLightCount] = {};

		struct SpotLightData
		{
			C::Vector4f myPosition;
			C::Vector4f myDirection;
			C::Vector4f myColorAndIntensity;
			float myRange = 0.f;
			float myInnerAngle = 0.f;
			float myOuterAngle = 0.f;
			float trash;
		} mySpotLights[MaxSpotLightCount] = {};
	};

	struct ShadowData_ForwardRenderer
	{
		std::array<Mat4f, 4> myEnvLightViews;
		std::array<Mat4f, 4> myEnvProjs;
	};
}