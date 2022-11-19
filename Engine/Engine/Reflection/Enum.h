#pragma once

namespace Engine
{
	class Enum
	{
	public:
		Enum() = default;
		Enum(const int aValue) : myValue(aValue) { }

		virtual std::string EnumToString(int aValue) = 0;
		virtual int GetCount() = 0;

		int GetValue() const;
		void SetValue(int aValue);

		bool operator==(const Enum& aOther) const
		{
			return myValue == aOther.myValue;
		}

		bool operator!=(const Enum& aOther) const
		{
			return !(*this == aOther);
		}

	private:
		friend class GameObjectPrefabReflector;

		int myValue = 0;
	};

	template <typename BetterEnum>
	class EnumClass : public Enum
	{
	public:
		EnumClass() = default;
		EnumClass(int aValue) : Enum(aValue)
		{};

		virtual std::string EnumToString(int aValue)
		{
			return BetterEnum::_from_integral(aValue)._to_string();
		};

		virtual int GetCount()
		{
			return BetterEnum::_size();
		};
	};
}