#pragma once

namespace Engine
{
	class ResourceId
	{
	public:
		ResourceId() = default;
		ResourceId(const std::string& aId);

		operator std::string()
		{
			return myId;
		}

		operator std::string() const
		{
			return myId;
		}

		bool operator==(const ResourceId& aOther) const
		{
			return myId == aOther.myId;
		}

	private:
		std::string myId;
	};
}