#pragma once
#include "BT_Decorator.h"

// The Failer decorator returns failure, regardless of what happens to the child.
class BT_Failer : public BT_Decorator
{
public:
	Status update() override
	{
		child->tick();
		return Status::Failure;
	}
};

