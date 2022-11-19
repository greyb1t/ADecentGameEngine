#pragma once
#include "BT_Decorator.h"

// The Repeater decorator repeats infinitely or to a limit until the child returns success.
class BT_Repeater : public BT_Decorator
{
public:
	BT_Repeater(int limit = 0) : limit(limit) {}

	void initialize() override
	{
		counter = 0;
	}

	Status update() override
	{
		auto status = child->tick();

		if (status == Status::Success) {
			return Status::Success;
		}

		if (limit > 0 && ++counter == limit) {
			return Status::Success;
		}

		return Status::Running;
	}

protected:
	int limit;
	int counter = 0;
};