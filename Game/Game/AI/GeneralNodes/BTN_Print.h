#pragma once
#include "AI/BehaviorTree/Base/BT_Leaf.h"
#include <string>

class BTN_Print : public BT_Leaf
{
public:
	BTN_Print(std::string aText, bool aShouldSucces) :
		myText(aText),
		myShouldSuccess(aShouldSucces)
	{};

	Status update() override
	{
		if (myShouldSuccess)
		{
			LOG_INFO(LogType::Game) << myText;
			return Status::Success;
		}
		return Status::Failure;
	}

private:
	std::string myText = "default";
	bool myShouldSuccess = true;
};

