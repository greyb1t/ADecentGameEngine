#pragma once
#include "BT_Composite.h"

class BT_ParallelSequence : public BT_Composite
{
public:
	BT_ParallelSequence(bool successOnAll = true, bool failOnAll = true) : useSuccessFailPolicy(true), successOnAll(successOnAll), failOnAll(failOnAll) {}
	BT_ParallelSequence(int minSuccess, int minFail) : minSuccess(minSuccess), minFail(minFail) {}

	Status update() override
	{
		assert(hasChildren() && "Composite has no children");

		int minimumSuccess = minSuccess;
		int minimumFail = minFail;

		if (useSuccessFailPolicy) {
			if (successOnAll) {
				minimumSuccess = children.size();
			}
			else {
				minimumSuccess = 1;
			}

			if (failOnAll) {
				minimumFail = children.size();
			}
			else {
				minimumFail = 1;
			}
		}

		int total_success = 0;
		int total_fail = 0;

		for (auto& child : children) {
			auto status = child->tick();
			if (status == Status::Success) {
				total_success++;
			}
			if (status == Status::Failure) {
				total_fail++;
			}
		}

		if (total_success >= minimumSuccess) {
			return Status::Success;
		}
		if (total_fail >= minimumFail) {
			return Status::Failure;
		}

		return Status::Running;
	}

private:
	bool useSuccessFailPolicy = false;
	bool successOnAll = true;
	bool failOnAll = true;
	int minSuccess = 0;
	int minFail = 0;
};
