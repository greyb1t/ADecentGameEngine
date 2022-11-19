#pragma once

namespace Engine
{
	
class Coroutine
{
public:
	Coroutine(const std::function<float(float)>& aFunction, float aWaitTime, bool aDeleteOnSceneTransition = true);
	~Coroutine();
	void Update(float aDeltaTime);

	bool DeleteOnSceneTransition() const;
private:
	std::function<float(float)> myFunction;
	float myTime;
	float myStepTime = 0;
	bool myDeleteOnSceneTransition = true;
};


}