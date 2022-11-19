#include "pch.h"
#include "ConditionGroup.h"

bool Engine::ConditionGroup::InitFromJson(const nlohmann::json& aJson)
{
	for (const nlohmann::json& conditionJson : aJson["Conditions"])
	{
		Condition condition;
		condition.InitFromJson(conditionJson);

		AddCondition(condition);
	}

	myOp = aJson["Op"];

	return true;
}

nlohmann::json Engine::ConditionGroup::ToJson() const
{
	nlohmann::json j;

	nlohmann::json conditionsArrayJson;

	for (const auto& condition : myConditions)
	{
		conditionsArrayJson.push_back(condition.ToJson());
	}

	j["Conditions"] = conditionsArrayJson;

	j["Op"] = myOp;

	return j;
}

void Engine::ConditionGroup::AddCondition(const Condition& aCondition)
{
	myConditions.push_back(aCondition);
}

bool Engine::ConditionGroup::IsTrue() const
{
	for (const auto& cond : myConditions)
	{
		if (!cond.IsTrue())
		{
			return false;
		}
	}

	return true;
}
