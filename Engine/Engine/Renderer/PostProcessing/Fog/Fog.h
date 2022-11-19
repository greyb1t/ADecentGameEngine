#pragma once

namespace Engine
{
	struct FogData
	{
		Vec3f myColor = Vec3f(15.f / 255.f, 15.f / 255.f, 15.f / 255.f);

		float myStart = 0.f;
		float myEnd = 26000.f;

		float myFogStrength = 1.f;

		float myHeightFogOffset = 0.f;
		float myHeightFogStrength = 10.f;

		static FogData Lerp(const FogData& aFirst, const FogData& aSecond, const float aT);
	};

	class Fog
	{
	public:
		Fog() = default;
		Fog(const std::string& aName);

		void SetData(const FogData& aData);

		const std::string& GetName() const;

		const FogData& GetData() const;

	private:
		std::string myName;

		FogData myData;
	};
}