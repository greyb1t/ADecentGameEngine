#pragma once

#include <vector>

//#include "rapidjson/document.h"

namespace Engine
{
	class Key
	{
	public:
		Key() = default;

		Key(const CU::Vector2f& aPosition);

		bool InitFromJson(const nlohmann::json& aJson);

		nlohmann::json ToJson() const;

		void SetPosition(const CU::Vector2f& aPosition);

	private:
		friend class Curve;
		friend class CurveEditor;

		CU::Vector2f myPosition;

		CU::Vector2f myTangentRight;
		CU::Vector2f myTangentLeft;

		bool myHasLeftTangent = false;
		bool myHasRightTangent = false;
	};

	class Curve
	{
	public:
		Curve() = default;

		void InitLinear();

		bool InitFromJson(const nlohmann::json& aKeysArrayJson);
		nlohmann::json ToJson() const;

		void AddKey(const Key& aKey);
		void RemoveKeyAtIndex(const int aIndex);
		void Clear();
		void SwapKeys(const int aIndex1, const int aIndex2);

		CU::Vector2f EvaluateVec2(const float aTime) const;
		float Evaluate(const float aTime) const;

		std::vector<Key>& GetKeys();

		float GetMinimumTime() const;
		float GetMaximumTime() const;

		void SortKeys();

	private:
		std::vector<Key> myKeys;
	};
}