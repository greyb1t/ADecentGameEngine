#pragma once

namespace Engine
{
	class SceneHandle
	{
	public:
		SceneHandle() = default;

		SceneHandle(const uint32_t aValue);

		bool IsValid() const;

		operator uint32_t() const { return myValue; }

	private:
		uint32_t myValue = static_cast<uint32_t>(-1);
	};
}

namespace std
{
	template <>
	struct hash<Engine::SceneHandle>
	{
		std::size_t operator()(const Engine::SceneHandle& aHandle) const
		{
			return hash<uint32_t>()(static_cast<uint32_t>(aHandle));
		}
	};
}