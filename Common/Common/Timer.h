#pragma once

#include <chrono>

namespace Common
{
	class Timer
	{
	public:
		Timer();
		Timer(const Timer& aTimer) = delete;
		Timer& operator=(const Timer& aTimer) = delete;

		void Update();
		float GetDeltaTime() const;
		void SetDeltaTime(const float aDeltaTime);
		double GetTotalTime() const;

	private:
		std::chrono::high_resolution_clock::time_point Now() const;

		std::chrono::high_resolution_clock::time_point myPrevFrameTime;

		float myDeltaTime = 0.f;
		double myTotalTime = 0.0;

		std::chrono::high_resolution_clock::time_point myStartTime;
	};
}

namespace CU = Common;