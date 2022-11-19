#pragma once

#include <xhash>

// Thank you for TheCherno

namespace Engine
{
	class UUID
	{
	public:
		UUID();
		UUID(const int aUUID);
		UUID(const UUID&) = default;

		operator int() const { return myUUID; }

		int& GetRef() { return myUUID; }

	private:
		int myUUID;
	};
}

namespace std
{
	template <>
	struct hash<Engine::UUID>
	{
		std::size_t operator()(const Engine::UUID& aUUID) const
		{
			return hash<int>()(static_cast<int>(aUUID));
		}
	};
}