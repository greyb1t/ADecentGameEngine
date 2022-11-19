#pragma once

#include "Engine\Reflection\Enum.h"
#include "Engine\Reflection\Reflectable.h"

namespace Engine
{
	enum class TonemapType
	{
		Timothy = 0,
		DX11DSK = 1,
		Reinhard = 2,
		Uncharted2 = 3,
		Aces = 4,

		Count
	};

	class TonemapTypeClass : public Enum
	{
	public:
		TonemapTypeClass(const int aValue) : Enum(aValue) { }

		std::string EnumToString(int aValue)
		{
			switch (static_cast<TonemapType>(aValue))
			{
			case TonemapType::Timothy: return "Timothy";
			case TonemapType::DX11DSK: return "DX11DSK";
			case TonemapType::Reinhard: return "Reinhard";
			case TonemapType::Uncharted2: return "Uncharted2";
			case TonemapType::Aces: return "Aces Filmic (like Unreal)";
			default:
				return "Missing case";
				break;
			}
		}

		int GetCount() { return static_cast<int>(TonemapType::Count); }
	};

	struct TonemappingSettings : public Reflectable
	{
		bool myEnabled = true;

		TonemapTypeClass myTonemapType = static_cast<int>(TonemapType::Aces);
		float myExposure = 1.f;

		void Reflect(Reflector& aReflector) override;
	};
}