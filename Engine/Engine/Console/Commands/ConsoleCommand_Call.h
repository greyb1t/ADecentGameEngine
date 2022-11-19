#pragma once

#include "..\ConsoleCommand.h"

namespace Engine
{
	class CommandHandler;

	class ConsoleCommand_Call : public ConsoleCommand
	{
	public:
		ConsoleCommand_Call(CommandHandler& aCommandHandler);

		bool Execute(const std::vector<std::string>& aTokens, std::string& aOutput) override;

		std::string UsageString() const override;
		std::string Description() const override;

	private:
		CommandHandler& myCommandHandler;
	};
}