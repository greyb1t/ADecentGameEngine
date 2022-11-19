#pragma once
#include "BT_Decorator.h"

// The Succeeder decorator returns success, regardless of what happens to the child.
class BT_Succeeder : public BT_Decorator
{
public:
	Status update() override
	{
		child->tick();
		return Status::Success;
	}
};