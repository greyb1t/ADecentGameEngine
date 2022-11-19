#pragma once
#include "nlohmann/json.hpp"
#include "CommonUtilities/Vector3.hpp"

namespace JsonUtils
{
	static void from_json(nlohmann::json& j, CU::Vector3f& v)
	{
		v.x = j[0];
		v.y = j[1];
		v.z = j[2];
	}
}
