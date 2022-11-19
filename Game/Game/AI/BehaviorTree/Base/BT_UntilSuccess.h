#pragma once
#include "BT_Decorator.h"

// The UntilSuccess decorator repeats until the child returns success and then returns success.
class BT_UntilSuccess : public BT_Decorator
{
public:
	Status update() override
	{
		while (1) {
			auto status = child->tick();

			if (status == Status::Success) {
				return Status::Success;
			}
		}
	}
};
