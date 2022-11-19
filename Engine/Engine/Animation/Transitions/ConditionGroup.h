#pragma once

#include "Condition.h"

namespace Engine
{
	enum class ConditionGroupOp
	{
		And,
		Or
	};

	class ConditionGroup
	{
	public:
		bool InitFromJson(const nlohmann::json& aJson);
		nlohmann::json ToJson() const;

		void AddCondition(const Condition& aCondition);

		bool IsTrue() const;

		std::vector<Condition>& GetConditions() { return myConditions; }

	private:
		friend class AnimationNodeEditorWindow;

		std::vector<Condition> myConditions;
		ConditionGroupOp myOp = ConditionGroupOp::And;
	};
}