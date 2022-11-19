#include "pch.h"
#include "EffectVariables.h"

Engine::EffectVariable::EffectVariable(const int aIndexToDataBuffer)
	: myIndexToDataBuffer(aIndexToDataBuffer)
{
}

int Engine::EffectVariable::GetIndexToDataBuffer() const
{
	return myIndexToDataBuffer;
}

nlohmann::json Engine::EffectVariable::ToJson() const
{
	nlohmann::json j;

	//j["Name"] = myName;
	//j["Type"] = myType;

	//if (myType == "Float4")
	//{
	//	if (auto valueFloat4 = std::get_if<C::Vector4f>(&myDefaultValue))
	//	{
	//		auto& defaultValueJson = j["DefaultValue"];

	//		defaultValueJson["x"] = valueFloat4->x;
	//		defaultValueJson["y"] = valueFloat4->y;
	//		defaultValueJson["z"] = valueFloat4->z;
	//		defaultValueJson["w"] = valueFloat4->w;
	//	}
	//}

	return j;
}
