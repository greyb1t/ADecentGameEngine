#pragma once

namespace Engine
{
	struct IntensityControllerDesc
	{
		float myTargetIntensity = 1.f;

		float myHowFastToIncreaseIntensity = 10.f;

		float mySustainIntensityDurationSeconds = 0.f;

		float myHowFastToDecreaseIntensity = 1.f;

		void InitFromJson(const nlohmann::json& aJson);
	};

	class IntensityController
	{
	public:
		IntensityController(const IntensityControllerDesc& aDesc);

		void Update(const float aDeltaTime);

		float GetIntensity() const;
		bool IsFinished() const;

	private:
		enum State
		{
			Increase,
			Sustain,
			Decrease
		};

		State myState = Increase;
		float myInternalIntensity = 0.f;
		float myIntensity = 0.f;

		bool myFinished = false;

		float mySustainTimerSec = 0.f;

		IntensityControllerDesc myDesc;
	};
}