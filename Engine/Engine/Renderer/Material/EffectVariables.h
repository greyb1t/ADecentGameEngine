#pragma once

namespace Engine
{
	class EffectVariable
	{
	public:
		EffectVariable() = default;
		EffectVariable(const int aIndexToDataBuffer);

		int GetIndexToDataBuffer() const;

		nlohmann::json ToJson() const;

	private:
		int myIndexToDataBuffer = -1;

		// ONLY FOR DEBUG
		std::type_info* myType = nullptr;
	};
}