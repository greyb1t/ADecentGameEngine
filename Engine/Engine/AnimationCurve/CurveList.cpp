#include "pch.h"
#include "CurveList.h"

Engine::CurveList::CurveList()
{

}

Engine::CurveList::~CurveList()
{

}

bool Engine::CurveList::InitFromJson(const nlohmann::json& aCurveListJsonValue)
{
	assert(aCurveListJsonValue.is_array() && "The curve list must be an array");

	for (const auto& curveEntryValue : aCurveListJsonValue)
	{
		if (!curveEntryValue.contains("Name") || !curveEntryValue.contains("Curve"))
		{
			return false;
		}

		const std::string name = curveEntryValue["Name"];

		const auto& curveJsonValue = curveEntryValue["Curve"];

		Curve curve;

		if (!curve.InitFromJson(curveJsonValue))
		{
			return false;
		}

		myCurves[name] = curve;
	}

	return true;
}

const Engine::Curve& Engine::CurveList::GetCurve(const std::string& aName) const
{
	return myCurves.at(aName);
}

void Engine::CurveList::Deserialize(const nlohmann::json& aJson)
{
	InitFromJson(aJson["CurveList"]);
}
