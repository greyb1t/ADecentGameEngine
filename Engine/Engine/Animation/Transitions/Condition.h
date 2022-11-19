#pragma once

namespace Engine
{
	enum class ConditionValueType
	{
		Bool,
		Int,
		Float,

		Count
	};

	const static inline std::string GlobalConditionValueTypeStrings[] =
	{
		"Bool",
		"Int",
		"Float"
	};

	enum class ConditionComparison
	{
		Equals,
		NotEquals,
		Greater,
		GreaterEquals,
		Less,
		LessEquals,

		Count
	};

	const static inline std::string GlobalConditionComparison[] =
	{
		"Equals",
		"NotEquals",
		"Greater",
		"GreaterEquals",
		"Less",
		"LessEqual"
	};

	class Condition
	{
	public:
		bool InitFromJson(const nlohmann::json& aJson);
		nlohmann::json ToJson() const;

		bool IsTrue() const;

		void SetValuePointer(int* aIntPointer);
		void SetValuePointer(bool* aBoolPointer);
		void SetValuePointer(float* aFloatPointer);

		const std::string& GetValueName() const;

		ConditionValueType GetValueType() const;

	private:
		friend class AnimationNodeEditorWindow;

		// for debugging
		std::string myValueName;

		// bool* myConditionBoolPointer = nullptr;
		// bool myTrueValue = false;

		ConditionValueType myValueType = ConditionValueType::Bool;
		ConditionComparison myComparison = ConditionComparison::Equals;
		std::variant<bool, int, float> myCompareValue;

		std::variant<bool*, int*, float*> myValuePointer;
	};
}