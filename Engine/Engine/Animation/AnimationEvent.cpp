#include "pch.h"
#include "AnimationEvent.h"
#include "..\Engine.h"
#include "NormalAnimation.h"

bool Engine::AnimationEvent::InitFromJson(const nlohmann::json& aJson)
{
	if (aJson.contains("Name"))
	{
		myEventName = aJson["Name"];
	}
	else
	{
		LOG_ERROR(LogType::Animation) << "Missing \"Name\" in Event";
		return false;
	}

	if (aJson.contains("Frame"))
	{
		myFrame = aJson["Frame"];
		myType = Type::ByFrame;
	}
	else if (aJson.contains("NormalizedTime"))
	{
		myNormalizedTime = aJson["NormalizedTime"];
		myType = Type::ByNormalizedTime;
	}
	else
	{
		LOG_ERROR(LogType::Animation) << "Missing \"Frame\" or \"NormalizedTime\" in Event";
		return false;
	}

	return true;
}

nlohmann::json Engine::AnimationEvent::ToJson() const
{
	nlohmann::json j;

	j["Name"] = myEventName;

	switch (myType)
	{
		case AnimationEvent::Type::ByFrame:
			j["Frame"] = myFrame;
			j["Type"] = Type::ByFrame;
			break;
		case AnimationEvent::Type::ByNormalizedTime:
			j["NormalizedTime"] = myNormalizedTime;
			j["Type"] = Type::ByNormalizedTime;
			break;
		default:
			assert(false);
			break;
	}

	return j;
}

const std::string Engine::AnimationEvent::GetName() const
{
	return myEventName;
}

bool Engine::AnimationEvent::WasExecuted(const NormalAnimation& aNormalAnimation) const
{
	switch (myType)
	{
		case AnimationEvent::Type::ByFrame:
		{
			if (aNormalAnimation.GetCurrentFrame() >= static_cast<float>(myFrame))
			{
				return true;
			}
		} break;

		case AnimationEvent::Type::ByNormalizedTime:
		{
			if (aNormalAnimation.GetElapsedTimeNormalized() >= myNormalizedTime)
			{
				return true;
			}
		} break;
		default:
			break;
	}

	return false;
}
