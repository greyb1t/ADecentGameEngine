#pragma once

#include "Curve.h"

namespace Engine
{
	class CurveList
	{
	public:
		CurveList();
		~CurveList();

		const Curve& GetCurve(const std::string& aName) const;

		void Deserialize(const nlohmann::json& aJson);

	private:
		bool InitFromJson(const nlohmann::json& aCurveListJsonValue);

	private:
		std::map<std::string, Curve> myCurves;
	};
}