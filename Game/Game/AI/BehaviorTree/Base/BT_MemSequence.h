#pragma once
#include "BT_Composite.h"

class BT_MemSequence : public BT_Composite
	{
	public:
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

			it = children.begin();
			return Status::Success;
		}
	};
