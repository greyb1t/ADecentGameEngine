#include "pch.h"
#include "Curve.h"

using namespace rapidjson;

const int globalMaxPoints = 32;

void Engine::Curve::InitLinear()
{
	myKeys.push_back(Key({ 0.f, 0.f }));
	myKeys.push_back(Key({ 1.f, 1.f }));
}

bool Engine::Curve::InitFromJson(const nlohmann::json& aKeysArrayJson)
{
	for (const auto& keyValue : aKeysArrayJson)
	{
		Key key;

		if (!key.InitFromJson(keyValue))
		{
			return false;
		}

		AddKey(key);
	}

	return true;
}

nlohmann::json Engine::Curve::ToJson() const
{
	nlohmann::json j = nlohmann::json::array();

	for (const auto& key : myKeys)
	{
		j.push_back(key.ToJson());
	}

	return j;
}

void Engine::Curve::AddKey(const Key& aKey)
{
	const auto it = std::upper_bound(myKeys.begin(), myKeys.end(), aKey,
		[](const Key& aLeft, const Key& aRight)
		{
			return aLeft.myPosition.x < aRight.myPosition.x;
		});

	myKeys.insert(it, aKey);
}

void Engine::Curve::RemoveKeyAtIndex(const int aIndex)
{
	myKeys.erase(myKeys.begin() + aIndex);
}

void Engine::Curve::Clear()
{
	myKeys.clear();
}

void Engine::Curve::SwapKeys(const int aIndex1, const int aIndex2)
{
	std::swap(myKeys[aIndex1], myKeys[aIndex2]);
}

CU::Vector2f VectorLerp(const CU::Vector2f& aA, const CU::Vector2f& aB, const float aTime)
{
	return CU::Vector2f(
		CU::Lerp(aA.x, aB.x, aTime),
		CU::Lerp(aA.y, aB.y, aTime)
	);
}

template <int N>
CU::Vector2f EvaluateAsBezierHighPerformance(CU::Vector2f(&aPoints)[N], const float aTime)
{
	static_assert(N != 0);

	int index = N;

	while (index > 1)
	{
		for (int i = 0; i < index - 1; ++i)
		{
			const auto& p1 = aPoints[i];
			const auto& p2 = aPoints[i + 1];

			const auto value = VectorLerp(p1, p2, aTime);

			aPoints[i] = value;
		}

		--index;
	}

	return aPoints[0];
}

CU::Vector2f EvaluateAsBezier(C::VectorOnStack<Vec2f, globalMaxPoints, int>&& aPoints, const float aTime)
{
	if (aPoints.Size() == 0)
	{
		return { };
	}

	while (aPoints.Size() > 1)
	{
		for (int i = 0; i < aPoints.Size() - 1; ++i)
		{
			const auto& p1 = aPoints[i];
			const auto& p2 = aPoints[i + 1];

			const auto value = VectorLerp(p1, p2, aTime);

			aPoints[i] = value;
		}

		aPoints.RemoveCyclicAtIndex(aPoints.Size() - 1);
	}

	return aPoints[0];
}

CU::Vector2f Engine::Curve::EvaluateVec2(const float aTime) const
{
	if (myKeys.empty())
	{
		return { };
	}

	// Search for the 2 points to lerp
	for (int i = 0; i < static_cast<int>(myKeys.size()) - 1; ++i)
	{
		auto& key1 = myKeys[i];
		auto& key2 = myKeys[i + 1];

		if (aTime >= key1.myPosition.x && aTime <= key2.myPosition.x)
		{
			// CU::Vector2f pos1 = key1.myPosition;
			// CU::Vector2f pos2 = key2.myPosition;

			const float someTime = Math::InverseLerp(key1.myPosition.x, key2.myPosition.x, aTime);

			if (key1.myHasRightTangent && key2.myHasLeftTangent)
			{
				auto& nextKey = key2;

				//std::vector<CU::Vector2f> poses;
				//
				//poses.push_back(key1.myPosition);
				//poses.push_back(key1.myTangentRight);
				//poses.push_back(key2.myTangentLeft);
				//poses.push_back(nextKey.myPosition);

				//CU::VectorOnStack<CU::Vector2f, globalMaxPoints, int> poses;
				//
				//poses.Add(key1.myPosition);
				//poses.Add(key1.myTangentRight);
				//poses.Add(key2.myTangentLeft);
				//poses.Add(nextKey.myPosition);

				//auto yeet = EvaluateAsBezier(std::move(poses), someTime);

				CU::Vector2f arr[] = {
					key1.myPosition,
					key1.myTangentRight,
					key2.myTangentLeft,
					nextKey.myPosition
				};

				auto yeet = EvaluateAsBezierHighPerformance(arr, someTime);

				return yeet;
			}
			else
			{
				if (key1.myHasRightTangent)
				{
					auto& nextKey = key2;

					//std::vector<CU::Vector2f> poses;

					//CU::VectorOnStack<CU::Vector2f, globalMaxPoints, int> poses;
					//
					//poses.Add(key1.myPosition);
					//poses.Add(key1.myTangentRight);
					//poses.Add(nextKey.myPosition);

					//auto yeet = EvaluateAsBezier(std::move(poses), someTime);


					CU::Vector2f arr[] = {
						key1.myPosition,
						key1.myTangentRight,
						nextKey.myPosition
					};

					auto yeet = EvaluateAsBezierHighPerformance(arr, someTime);

					return yeet;
				}

				if (key2.myHasLeftTangent)
				{
					auto& prevKey = key1;

					// std::vector<CU::Vector2f> poses;

					/*
					CU::VectorOnStack<CU::Vector2f, globalMaxPoints, int> poses;

					poses.Add(prevKey.myPosition);
					poses.Add(key2.myTangentLeft);
					poses.Add(key2.myPosition);

					auto yeet = EvaluateAsBezier(std::move(poses), someTime);
					*/

					CU::Vector2f arr[] = {
						prevKey.myPosition,
						key2.myTangentLeft,
						key2.myPosition
					};

					auto yeet = EvaluateAsBezierHighPerformance(arr, someTime);

					return yeet;
				}
			}

			// Found them
			const auto delta = key2.myPosition - key1.myPosition;

			const auto result = key1.myPosition + someTime * delta;
			return result;
		}
	}

	return CU::Vector2f(aTime, myKeys.back().myPosition.y);
}

float Engine::Curve::Evaluate(const float aTime) const
{
	return EvaluateVec2(aTime).y;
}

std::vector<Engine::Key>& Engine::Curve::GetKeys()
{
	return myKeys;
}

float Engine::Curve::GetMinimumTime() const
{
	if (!myKeys.empty())
	{
		return myKeys.front().myPosition.x;
	}

	return 0.f;
}

float Engine::Curve::GetMaximumTime() const
{
	if (!myKeys.empty())
	{
		return myKeys.back().myPosition.x;
	}

	return 0.f;
}

void Engine::Curve::SortKeys()
{
	std::sort(myKeys.begin(), myKeys.end(),
		[](const Key& aLeft, const Key& aRight)
		{
			return aLeft.myPosition.x < aRight.myPosition.x;
		});
}

Engine::Key::Key(const CU::Vector2f& aPosition)
	: myPosition(aPosition)
{
}

bool Engine::Key::InitFromJson(const nlohmann::json& aJson)
{
	myPosition = CU::Vector2f(
		aJson["PositionX"],
		aJson["PositionY"]);

	myTangentLeft = CU::Vector2f(
		aJson["TangentLeftX"],
		aJson["TangentLeftY"]);

	myTangentRight = CU::Vector2f(
		aJson["TangentRightX"],
		aJson["TangentRightY"]);

	myHasLeftTangent = aJson["HasTangentLeft"];
	myHasRightTangent = aJson["HasTangentRight"];

	return true;
}

nlohmann::json Engine::Key::ToJson() const
{
	nlohmann::json j;

	j["PositionX"] = myPosition.x;
	j["PositionY"] = myPosition.y;

	j["TangentLeftX"] = myTangentLeft.x;
	j["TangentLeftY"] = myTangentLeft.y;

	j["TangentRightX"] = myTangentRight.x;
	j["TangentRightY"] = myTangentRight.y;

	j["HasTangentLeft"] = myHasLeftTangent;
	j["HasTangentRight"] = myHasRightTangent;

	return j;
}

void Engine::Key::SetPosition(const CU::Vector2f& aPosition)
{
	if (myHasLeftTangent)
	{
		const auto delta = myTangentLeft - myPosition;
		myTangentLeft = aPosition + delta;
	}

	if (myHasRightTangent)
	{
		const auto delta = myTangentRight - myPosition;
		myTangentRight = aPosition + delta;
	}

	myPosition = aPosition;
}
