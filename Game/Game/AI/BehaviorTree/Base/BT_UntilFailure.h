#pragma once
#include "BT_Decorator.h"

// The UntilFailure decorator repeats until the child returns fail and then returns success.
class BT_UntilFailure : public BT_Decorator
{
public:
	Status update() override
	{
		while (1) {
			auto status = child->tick();

			if (status == Status::Failure) {
				return Status::Success;
			}
		}
	}
};