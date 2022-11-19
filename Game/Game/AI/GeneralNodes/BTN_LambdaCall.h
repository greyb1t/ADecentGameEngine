#pragma once
#include "AI/BehaviorTree/Base/BT_Leaf.h"

class BTN_LambdaCall : public BT_Leaf
{
public:
	BTN_LambdaCall(std::function<void()> aFunctional, bool myShouldSuccess = true) :
		myFunctional(aFunctional),
		myShouldSuccess(myShouldSuccess)
	{};

	Status update() override
	{
		myFunctional();
		if (myShouldSuccess)
		{
			return Status::Success;
		}
		return Status::Failure;
	}

private:
	std::function<void()> myFunctional;
	bool myShouldSuccess = true;
};

