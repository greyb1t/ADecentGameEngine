#pragma once
#include "BT_Composite.h"

// The Sequence composite ticks each child node in order.
	// If a child fails or runs, the sequence returns the same status.
	// In the next tick, it will try to run each child in order again.
	// If all children succeeds, only then does the sequence succeed.
class BT_Sequence : public BT_Composite
{
public:
	void initialize() override
	{
		it = children.begin();
	}

	Status update() override
	{
		assert(hasChildren() && "Composite has no children");

		while (it != children.end()) {
			auto status = (*it)->tick();

			if (status != Status::Success) {
				return status;
			}

			it++;
		}

		return Status::Success;
	}
};