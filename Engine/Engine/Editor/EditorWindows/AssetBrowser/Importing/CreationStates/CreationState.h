#pragma once

namespace Engine
{
	class CreationState
	{
	public:
		virtual bool Draw() = 0;

		virtual void ParseFiles(const std::vector<std::filesystem::path>& aPaths) = 0;

	private:

	};
}