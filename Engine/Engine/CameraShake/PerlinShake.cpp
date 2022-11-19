#include "pch.h"
#include "PerlinShake.h"

Engine::PerlinShake::PerlinShake(const PerlinShakeDesc& aDesc)
	: myDesc(aDesc),
	myIntensityController(aDesc.myIntensityDesc)
{
	//myNoiseSeed = CU::Vector2f(
	//	Random::RandomFloat(-200.f, 200.f),
	//	Random::RandomFloat(-200.f, 200.f));
	//myNoiseSeed = { };
}

void Engine::PerlinShake::Update(const float aDeltaTime)
{
	if (myIntensityController.IsFinished())
	{
		myIsFinished = true;
		return;
	}

	myIntensityController.Update(aDeltaTime);

	myTime += aDeltaTime;

	Displacement noiseDisplacement;

	const auto& seed = myDesc.myNoiseSeed;

	auto pos = Vec3f(
		Math::PerlinNoise(myTime * myDesc.myNoiseFrequency + seed.x, seed.y),
		Math::PerlinNoise(seed.x, myTime * myDesc.myNoiseFrequency + seed.y),
		Math::PerlinNoise(myTime * myDesc.myNoiseFrequency + seed.x, myTime * myDesc.myNoiseFrequency + seed.y));

	// Since perlin noise returns between 0 and 1
	// we offset the result to "center" the value to avoid the 
	// camera from going in one direction only
	pos -= Vec3f(1.f, 1.f, 1.f) * 0.5f;

	auto rot = Vec3f(
		Math::PerlinNoise(myTime * myDesc.myNoiseFrequency - seed.x, -seed.y),
		Math::PerlinNoise(-seed.x, myTime * myDesc.myNoiseFrequency - seed.y),
		Math::PerlinNoise(myTime * myDesc.myNoiseFrequency - seed.x, myTime * myDesc.myNoiseFrequency - seed.y));

	rot -= Vec3f(1.f, 1.f, 1.f) * 0.5f;

	noiseDisplacement = noiseDisplacement +
		Displacement(pos, rot) * myDesc.myNoiseAmplitude;

	myDisplacement = myIntensityController.GetIntensity() *
		Displacement::Scale(noiseDisplacement, myDesc.myAxesStrength);
}

ObjectPool<Engine::PerlinShake> Engine::PerlinShake::ourPerlinShakePool = ObjectPool<Engine::PerlinShake>(1000);

void Engine::PerlinShakeDesc::InitFromJson(const nlohmann::json& aJson)
{
	myAxesStrength.InitFromJson(aJson["AxesStrength"]);

	myNoiseFrequency = aJson["NoiseFrequency"];
	myNoiseAmplitude = aJson["NoiseAmplitude"];

	myNoiseSeed.x = aJson["NoiseSeed"][0];
	myNoiseSeed.y = aJson["NoiseSeed"][1];

	myIntensityDesc.InitFromJson(aJson["IntensityController"]);
}
