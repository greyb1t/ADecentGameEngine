#pragma once

#include "CameraShakeBase.h"
#include "IntensityController.h"

namespace Engine
{
	struct NoisePair
	{
		float myFrequency = 0.f;
		float myAmplitude = 0.f;
	};

	struct PerlinShakeDesc
	{
		Displacement myAxesStrength = Displacement({ 0, 0, 0 }, { 1, 1, 1 });

		Vec2f myNoiseSeed;

		float myNoiseFrequency = 12.f;
		float myNoiseAmplitude = 1.f;

		IntensityControllerDesc myIntensityDesc;

		void InitFromJson(const nlohmann::json& aJson);
	};

	class PerlinShake : public CameraShakeBase
	{
	public:
		PerlinShake(const PerlinShakeDesc& aDesc);

		void Update(const float aDeltaTime) override;

		static void* operator new(size_t aSize)
		{
			return (void*)ourPerlinShakePool.Allocate();
		}

		static void operator delete(void* aPtr)
		{
			auto p = reinterpret_cast<PerlinShake*>(aPtr);
			ourPerlinShakePool.Free(p);
		}

	private:

	private:
		PerlinShakeDesc myDesc;

		IntensityController myIntensityController;

		float myTime = 0.f;

		// CU::Vector2f myNoiseSeed;

		static ObjectPool<PerlinShake> ourPerlinShakePool;
	};
}