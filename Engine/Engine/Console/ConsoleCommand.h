#pragma once

namespace Engine
{
	class ConsoleCommand
	{
	public:
		virtual bool Execute(const std::vector<std::string>& aTokens, std::string& aOutput) = 0;

		virtual std::string UsageString() const = 0;
		virtual std::string Description() const { return ""; }
	};
}