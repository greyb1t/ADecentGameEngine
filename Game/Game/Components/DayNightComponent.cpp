#include "pch.h"
#include "DayNightComponent.h"
#include "Engine/Reflection/Reflector.h"
#include "Engine/Time.h"
#include <Engine/GameObject/GameObject.h>
#include "Engine/GameObject/Components/ModelComponent.h"
#include "Common/Log.h"
#include "Engine/GameObject/Components/EnvironmentLightComponent.h"
#include "Engine/AnimationCurve/Curve.h"
#include "Engine/ResourceManagement/Resources/AnimationCurveResource.h"
#include "Engine/GameObject/Components/PostProcessComponent.h"
#include "Engine/Renderer/PostProcessing/Bloom/BloomSettings.h"

template<typename T>
T ControllerLerp(const std::vector<T>& aVector, AnimationCurveRef aController, float aTime)
{
	if (!aController->IsValid())
	{
		LOG_INFO(LogType::Game) << "DayNight: Invalid controller";
		return T{};
	}

	const float controllerValue = aController->Get().Evaluate(aTime);

	if (controllerValue == floor(controllerValue) &&
		controllerValue >= 0 &&
		controllerValue < aVector.size())
	{
		return aVector[controllerValue];
	}

	int minIndex = (int)std::floor(controllerValue);
	int maxIndex = minIndex + 1;

	if (minIndex < 0 || maxIndex > aVector.size() - 1)
	{
		LOG_INFO(LogType::Game) << "DayNight: Controller has invalid bounds.";
		return T{};
	}

	const float lerpValue = controllerValue - (float)minIndex;
	return C::Lerp(aVector[minIndex], aVector[maxIndex], lerpValue);
}

DayNightComponent::DayNightComponent()
{

}

void DayNightComponent::Reflect(Engine::Reflector& aReflector)
{
	Component::Reflect(aReflector);
	aReflector.Reflect(myDayDuration, "Day Duration");
	aReflector.Reflect(myNightDuration, "Night Duration");

	aReflector.Reflect(myEnvLightSettings, "Environment Light Settings");
	aReflector.Reflect(myBloomSettings, "Bloom Settings");
	aReflector.Reflect(myFogSettings, "Fog Settings");
}

void DayNightComponent::Execute(Engine::eEngineOrder aOrder)
{
	myTimeProgress += Time::DeltaTime;

	if (myTimeProgress > myDayDuration + myNightDuration)
		myTimeProgress -= myDayDuration + myNightDuration;

	UpdateEnvironment();
}

void DayNightComponent::Render()
{
	auto* modelComp = GetGameObject()->GetComponent<Engine::ModelComponent>();
	auto& material = modelComp->GetMeshMaterialInstanceByIndex(0);

	auto* envLight = GetGameObject()->GetScene()->GetEnvironmentLight();
	Vec4f lightDir = envLight->GetTransform().Forward().ToVec4(1.f);
	lightDir *= Vec4f(-1.f, -1.f, 1.f, 1);
	material.SetFloat4("mySunDirection", lightDir);

	float val = 0;
	//if (myIsDay)
	//{
	//	if (myDaySettings.myDuration == 0)
	//		return;
	//	const float normProgress = myTimeProgress / myDaySettings.myDuration;
	//	val = CU::Lerp(0.f, 2.f, normProgress);
	//}
	//else
	//{
	//	if (myNightSettings.myDuration == 0)
	//		return;
	//	const float normProgress = myTimeProgress / myNightSettings.myDuration;
	//	val = CU::Lerp(2.f, 4.f, normProgress);
	//}

	if (myTimeProgress < myDayDuration)
	{
		float prog = myTimeProgress / myDayDuration;
		val = C::Lerp(0.f, 2.f, prog);
	}
	else
	{
		float prog = (myTimeProgress - myDayDuration) / myNightDuration;
		val = C::Lerp(2.f, 4.f, prog);
	}

	material.SetFloat4("LerpValue", Vec4f(val, 0, 0, 0));
}

void DayNightComponent::UpdateEnvironment()
{
	DoLightRotation();

	ZoneNamedN(zone1, "DayNightComponent::UpdateEnvironment", true);
	// Environment light.
	{	
		GameObject* envLight = myGameObject->GetScene()->GetEnvironmentLight();
		
		Engine::EnvironmentLightComponent* envLightComp = envLight->GetComponent<Engine::EnvironmentLightComponent>();
		if(myEnvLightSettings.myColorController->IsValid())
			envLightComp->SetColor(ControllerLerp(myEnvLightSettings.myColors, myEnvLightSettings.myColorController, myTimeProgress));

		if (myEnvLightSettings.myIntensity->IsValid())
			envLightComp->SetColorIntensity(myEnvLightSettings.myIntensity->Get().Evaluate(myTimeProgress));

		if (myEnvLightSettings.myCubeIntensity->IsValid())
			envLightComp->SetCubemapIntensity(myEnvLightSettings.myCubeIntensity->Get().Evaluate(myTimeProgress));
	}

	Engine::PostProcessComponent* postProcess = myGameObject->GetSingletonComponent<Engine::PostProcessComponent>();
	if (!postProcess)
	{
		LOG_ERROR(LogType::Game) << "DayNight: No post process in scene.";
		return;
	}

	// Bloom.
	{
		ZoneNamedN(zone2, "DayNightComponent::UpdateEnvironment::Bloom", true);
		if (myBloomSettings.myIntensity->IsValid())
			postProcess->GetBloomSettingsRef().myIntensity = myBloomSettings.myIntensity->Get().Evaluate(myTimeProgress);

		if (myBloomSettings.mySampleScale->IsValid())
			postProcess->GetBloomSettingsRef().mySampleScale = myBloomSettings.mySampleScale->Get().Evaluate(myTimeProgress);

		if (myBloomSettings.myThreshold->IsValid())
			postProcess->GetBloomSettingsRef().myThreshold = myBloomSettings.myThreshold->Get().Evaluate(myTimeProgress);

		if (myBloomSettings.mySoftThreshold->IsValid())
			postProcess->GetBloomSettingsRef().mySoftThreshold = myBloomSettings.mySoftThreshold->Get().Evaluate(myTimeProgress);

		if (myBloomSettings.myIterationsOffset->IsValid())
			postProcess->GetBloomSettingsRef().myIterationsOffset = myBloomSettings.myIterationsOffset->Get().Evaluate(myTimeProgress);
	}

	// Fog.
	{
		if(myFogSettings.myColorController->IsValid())
			postProcess->GetFogSettingsRef().myColor = ControllerLerp(myFogSettings.myColors, myFogSettings.myColorController, myTimeProgress);

		if (myFogSettings.myStartDistance->IsValid())
			postProcess->GetFogSettingsRef().myStart = myFogSettings.myStartDistance->Get().Evaluate(myTimeProgress);

		if (myFogSettings.myEndDistance->IsValid())
			postProcess->GetFogSettingsRef().myEnd = myFogSettings.myEndDistance->Get().Evaluate(myTimeProgress);

		if (myFogSettings.myStrength->IsValid())
			postProcess->GetFogSettingsRef().myFogStrength = myFogSettings.myStrength->Get().Evaluate(myTimeProgress);

		if (myFogSettings.myHeightOffset->IsValid())
			postProcess->GetFogSettingsRef().myHeightFogOffset = myFogSettings.myHeightOffset->Get().Evaluate(myTimeProgress);

		if (myFogSettings.myHeightStrength->IsValid())
			postProcess->GetFogSettingsRef().myHeightFogStrength = myFogSettings.myHeightStrength->Get().Evaluate(myTimeProgress);
	}
}

void DayNightComponent::DoLightRotation()
{
	GameObject* envLight = myGameObject->GetScene()->GetEnvironmentLight();
	if (!envLight)
		return;

	bool isDay = myTimeProgress < myDayDuration;

	float lerpValue = 0;

	if (isDay)
		lerpValue = myTimeProgress / myDayDuration;
	else
		lerpValue = (myTimeProgress - myDayDuration) / myNightDuration;

	if (isDay) 
	{
		// Morgon - dag
		if (lerpValue < .5f)
		{
			float remappedValue = C::Remap(0.f, .5f, 0.f, 1.f, lerpValue);
			envLight->GetTransform().SetRotation(Quatf::Slerp(myEnvLightRotations[0] * C::DegToRad, myEnvLightRotations[1] * C::DegToRad, remappedValue));
		}
		// Dag - kväll
		else if (lerpValue < 1.f)
		{
			float remappedValue = C::Remap(.5f, 1.f, 0.f, 1.f, lerpValue);
			envLight->GetTransform().SetRotation(Quatf::Slerp(myEnvLightRotations[1] * C::DegToRad, myEnvLightRotations[2] * C::DegToRad, remappedValue));
		}
	}
	else
	{
		// kväll - natt
		if (lerpValue < 0.5f)
		{
			float remappedValue = C::Remap(0.f, .5f, 0.f, 1.f, lerpValue);
			envLight->GetTransform().SetRotation(Quatf::Slerp(myEnvLightRotations[2] * C::DegToRad, myEnvLightRotations[3] * C::DegToRad, remappedValue));
		}
		// Natt - morgon.
		else if (lerpValue < 1.f)
		{
			float remappedValue = C::Remap(.5f, 1.f, 0.f, 1.f, lerpValue);
			envLight->GetTransform().SetRotation(Quatf::Slerp(myEnvLightRotations[3] * C::DegToRad, myEnvLightRotations[4] * C::DegToRad, remappedValue));
		}
	}
	/*
	if (lerpValue < .25f)
	{
		float remappedValue = C::Remap(0.f, .25f, 0.f, 1.f, lerpValue);
		envLight->GetTransform().SetRotation(Quatf::Slerp(myEnvLightRotations[0] * C::DegToRad, myEnvLightRotations[1] * C::DegToRad, remappedValue));
	}
	
	else if (lerpValue < 0.5f)
	{
		float remappedValue = C::Remap(.25f, 0.5f, 0.f, 1.f, lerpValue);
		envLight->GetTransform().SetRotation(Quatf::Slerp(myEnvLightRotations[1] * C::DegToRad, myEnvLightRotations[2] * C::DegToRad, remappedValue));
	}

	else if (lerpValue < 0.75f)
	{
		float remappedValue = C::Remap(.5f, 0.75f, 0.f, 1.f, lerpValue);
		envLight->GetTransform().SetRotation(Quatf::Slerp(myEnvLightRotations[2] * C::DegToRad, myEnvLightRotations[3] * C::DegToRad, remappedValue));
	}

	else if (lerpValue < 1.0f)
	{
		float remappedValue = C::Remap(.75f, 1.0f, 0.f, 1.f, lerpValue);
		envLight->GetTransform().SetRotation(Quatf::Slerp(myEnvLightRotations[3] * C::DegToRad, myEnvLightRotations[4] * C::DegToRad, remappedValue));
	}
	*/
}
