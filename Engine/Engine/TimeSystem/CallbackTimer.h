#pragma once

namespace Engine
{
	
class CallbackTimer
{
public:
	CallbackTimer(const std::function<void(void)>& aFunction, float aTime, bool aDeleteOnSceneTransition = true);
	~CallbackTimer();

	bool DeleteOnSceneTransition() const;
private:
	friend class TimeSystem;
	bool Update(float aDeltaTime);

	std::function<void(void)> myFunction;
	float myTime;
	bool myDeleteOnSceneTransition = true;
};

}
