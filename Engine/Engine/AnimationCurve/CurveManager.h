#pragma once

namespace Engine
{
	class CurveList;
	class Curve;

	class CurveManager
	{
	public:
		bool Init(JsonManager& aJsonManager);

		const Curve& GetCurve(const std::string& aName) const;

	private:
		const CurveList* myCurveList = nullptr;
	};
}