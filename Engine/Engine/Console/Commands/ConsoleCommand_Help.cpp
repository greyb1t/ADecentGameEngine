#include "pch.h"
#include "ConsoleCommand_Help.h"
#include "..\CommandHandler.h"

Engine::ConsoleCommand_Help::ConsoleCommand_Help(CommandHandler& aCommandHandler)
	: myCommandHandler(aCommandHandler)
{
}

bool Engine::ConsoleCommand_Help::Execute(const std::vector<std::string>& aTokens, std::string& aOutput)
{
	std::string output;

	for (const auto& [commandString, command] : myCommandHandler.GetCommands())
	{
		output += commandString + " - " + command->UsageString() + " - " + command->Description() + "\n";
	}

	aOutput = output;

	return true;
}

std::string Engine::ConsoleCommand_Help::UsageString() const
{
	return "help";
}

std::string Engine::ConsoleCommand_Help::Description() const
{
	return "Lists all commands";
}
