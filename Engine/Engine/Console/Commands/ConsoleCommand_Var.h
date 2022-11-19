#pragma once
#include "..\ConsoleCommand.h"

namespace Engine
{
	class CommandHandler;

	class ConsoleCommand_Var : public ConsoleCommand
	{
	public:
		ConsoleCommand_Var(CommandHandler& aCommandHandler);

		bool Execute(const std::vector<std::string>& aTokens, std::string& aOutput) override;

		std::string UsageString() const override;
		std::string Description() const override;

	private:
		CommandHandler& myCommandHandler;
	};
}