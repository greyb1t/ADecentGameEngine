#pragma once
#include "BT_Decorator.h"

// The Inverter decorator inverts the child node's status, i.e. failure becomes success and success becomes failure.
	// If the child runs, the Inverter returns the status that it is running too.
class BT_Inverter : public BT_Decorator
{
public:
	Status update() override
	{
		auto s = child->tick();

		if (s == Status::Success) {
			return Status::Failure;
		}
		else if (s == Status::Failure) {
			return Status::Success;
		}

		return s;
	}
};