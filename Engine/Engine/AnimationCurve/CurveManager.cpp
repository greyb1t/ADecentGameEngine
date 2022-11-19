#include "pch.h"
#include "CurveManager.h"
#include "CurveList.h"

bool Engine::CurveManager::Init(JsonManager& aJsonManager)
{
	const std::string path = "Assets\\Json\\Curves.json";
	myCurveList = aJsonManager.DeserializeAs<CurveList>(path);

	if (myCurveList == nullptr)
	{
		LOG_ERROR(LogType::Resource) << "Failed to load: " << path;
		return false;
	}

	return true;
}

const Engine::Curve& Engine::CurveManager::GetCurve(const std::string& aName) const
{
	return myCurveList->GetCurve(aName);
}
