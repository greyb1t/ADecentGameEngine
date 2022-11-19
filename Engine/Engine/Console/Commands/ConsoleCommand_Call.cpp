#include "pch.h"
#include "ConsoleCommand_Call.h"
#include "..\CommandHandler.h"

Engine::ConsoleCommand_Call::ConsoleCommand_Call(CommandHandler& aCommandHandler)
	: myCommandHandler(aCommandHandler)
{
}

bool Engine::ConsoleCommand_Call::Execute(const std::vector<std::string>& aTokens, std::string& aOutput)
{
	assert(aTokens[0] == "call");

	if (aTokens.size() < 2)
	{
		aOutput = UsageString();

		return false;
	}

	const auto& name = aTokens[1];

	auto func = myCommandHandler.GetFunction(name);

	if (func)
	{
		func();
	}
	else
	{
		aOutput = "Function \"" + name + "\" was not found";
		return false;
	}

	aOutput = "Successfully called function";

	return true;
}

std::string Engine::ConsoleCommand_Call::UsageString() const
{
	return "call <function name>";
}

std::string Engine::ConsoleCommand_Call::Description() const
{
	return "Calls a function";
}
