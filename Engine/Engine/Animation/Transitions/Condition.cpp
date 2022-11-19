#include "pch.h"
#include "Condition.h"

bool Engine::Condition::InitFromJson(const nlohmann::json& aJson)
{
	myValueName = aJson["Name"];
	// myTrueValue = aJson["Value"];

	// FOR BACKWARDS COMPABILITY
	if (aJson.contains("Value"))
	{
		myCompareValue = static_cast<bool>(aJson["Value"]);
	}

	if (aJson.contains("ValueType"))
	{
		myValueType = aJson["ValueType"];
	}
	else
	{
		myValueType = ConditionValueType::Bool;
	}

	if (aJson.contains("Comparison"))
	{
		myComparison = aJson["Comparison"];
	}

	if (aJson.contains("CompareValue"))
	{
		switch (myValueType)
		{
		case ConditionValueType::Bool:
		{
			bool val = aJson["CompareValue"];
			myCompareValue = val;
		} break;
		case ConditionValueType::Int:
		{
			int val = aJson["CompareValue"];
			myCompareValue = val;
		} break;
		case ConditionValueType::Float:
		{
			float val = aJson["CompareValue"];
			myCompareValue = val;
		} break;
		default:
			break;
		}
	}

	return true;
}

nlohmann::json Engine::Condition::ToJson() const
{
	nlohmann::json j;

	j["Name"] = myValueName;
	// j["Value"] = myTrueValue;
	j["Comparison"] = myComparison;
	j["ValueType"] = myValueType;

	switch (myValueType)
	{
	case ConditionValueType::Bool:
		if (const bool* val = std::get_if<bool>(&myCompareValue))
		{
			j["CompareValue"] = *val;
		}
		break;
	case ConditionValueType::Int:
		if (const int* val = std::get_if<int>(&myCompareValue))
		{
			j["CompareValue"] = *val;
		}
		break;
	case ConditionValueType::Float:
		if (const float* val = std::get_if<float>(&myCompareValue))
		{
			j["CompareValue"] = *val;
		}
		break;
	default:
		break;
	}

	return j;
}

bool Engine::Condition::IsTrue() const
{
	switch (myValueType)
	{
	case ConditionValueType::Bool:
	{
		if (const bool* compareVal = std::get_if<bool>(&myCompareValue))
		{
			if (const bool* const* actualValue = std::get_if<bool*>(&myValuePointer))
			{
				switch (myComparison)
				{
				case Engine::ConditionComparison::Equals: return *compareVal == **actualValue;
				case Engine::ConditionComparison::NotEquals: return *compareVal != **actualValue;
				case Engine::ConditionComparison::Greater:
					LOG_WARNING(LogType::Animation) << "A comparison of \"Greater\" is not valid for type bool";
					return false;
					break;
				case Engine::ConditionComparison::GreaterEquals:
					LOG_WARNING(LogType::Animation) << "A comparison of \"GreaterEquals\" is not valid for type bool";
					return false;
					break;
				case Engine::ConditionComparison::Less:
					LOG_WARNING(LogType::Animation) << "A comparison of \"Less\" is not valid for type bool";
					return false;
					break;
				case Engine::ConditionComparison::LessEquals:
					LOG_WARNING(LogType::Animation) << "A comparison of \"LessEquals\" is not valid for type bool";
					return false;
					break;
				default:
					break;
				}
			}
			else
			{
				LOG_ERROR(LogType::Animation) << "Condition value mismatch: " << myValueName;
			}
		}
		else
		{
			LOG_ERROR(LogType::Animation) << "Condition value mismatch: " << myValueName;
		}
	} break;
	case ConditionValueType::Int:
	{
		if (const int* compareVal = std::get_if<int>(&myCompareValue))
		{
			if (const int* const* actualValue = std::get_if<int*>(&myValuePointer))
			{
				switch (myComparison)
				{
				case Engine::ConditionComparison::Equals: return *compareVal == **actualValue;
				case Engine::ConditionComparison::NotEquals: return *compareVal != **actualValue;
				case Engine::ConditionComparison::Greater: return **actualValue > *compareVal;
				case Engine::ConditionComparison::GreaterEquals: return **actualValue >= *compareVal;
				case Engine::ConditionComparison::Less: return **actualValue < *compareVal;
				case Engine::ConditionComparison::LessEquals: return **actualValue <= *compareVal;
				default:
					break;
				}
			}
			else
			{
				LOG_ERROR(LogType::Animation) << "Condition value mismatch: " << myValueName;
			}
		}
		else
		{
			LOG_ERROR(LogType::Animation) << "Condition value mismatch: " << myValueName;
		}
	} break;
	case ConditionValueType::Float:
	{
		if (const float* compareVal = std::get_if<float>(&myCompareValue))
		{
			if (const float* const* actualValue = std::get_if<float*>(&myValuePointer))
			{
				switch (myComparison)
				{
				case Engine::ConditionComparison::Equals: return *compareVal == **actualValue;
				case Engine::ConditionComparison::NotEquals: return *compareVal != **actualValue;
				case Engine::ConditionComparison::Greater: return **actualValue > *compareVal;
				case Engine::ConditionComparison::GreaterEquals: return **actualValue >= *compareVal;
				case Engine::ConditionComparison::Less: return **actualValue < *compareVal;
				case Engine::ConditionComparison::LessEquals: return **actualValue <= *compareVal;
				default:
					break;
				}
			}
			else
			{
				LOG_ERROR(LogType::Animation) << "Condition value mismatch: " << myValueName;
			}
		}
		else
		{
			LOG_ERROR(LogType::Animation) << "Condition value mismatch: " << myValueName;
		}
	} break;
	default:
		break;
	}

	LOG_ERROR(LogType::Animation) << "Condition has unknown type that is not handled in : Condition::IsTrue()" << myValueName;

	return false;
	//return *myConditionBoolPointer == myTrueValue;
}

void Engine::Condition::SetValuePointer(int* aIntPointer)
{
	myValuePointer = aIntPointer;
}

void Engine::Condition::SetValuePointer(float* aFloatPointer)
{
	myValuePointer = aFloatPointer;
}

void Engine::Condition::SetValuePointer(bool* aBoolPointer)
{
	myValuePointer = aBoolPointer;
}

const std::string& Engine::Condition::GetValueName() const
{
	return myValueName;
}

Engine::ConditionValueType Engine::Condition::GetValueType() const
{
	return myValueType;
}
