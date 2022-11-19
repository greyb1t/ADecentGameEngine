#include "pch.h"
#include "ConsoleCommand_Var.h"
#include "..\CommandHandler.h"

Engine::ConsoleCommand_Var::ConsoleCommand_Var(CommandHandler& aCommandHandler)
	: myCommandHandler(aCommandHandler)
{
}

bool Engine::ConsoleCommand_Var::Execute(const std::vector<std::string>& aTokens, std::string& aOutput)
{
	assert(aTokens[0] == "var");

	if (aTokens.size() < 2)
	{
		aOutput = UsageString();

		return false;
	}

	const auto& variableName = aTokens[1];

	if (auto variable = myCommandHandler.GetVariable(variableName))
	{
		switch (myCommandHandler.VariableToValueType(*variable))
		{
			case ValType::String:
			{
				const std::string value = *std::get<std::string*>(*variable);
				aOutput = "Type: string, Value: " + value;
			} break;
			case ValType::Bool:
			{
				const bool value = *std::get<bool*>(*variable);
				aOutput = "Type: bool, Value: " + value ? "true" : "false";
			} break;

			case ValType::Int:
			{
				const int value = *std::get<int*>(*variable);
				aOutput = "Type: int, Value: " + std::to_string(value);
			} break;

			case ValType::Float:
			{
				const float value = *std::get<float*>(*variable);
				aOutput = "Type: float, Value: " + std::to_string(value);
			} break;

			default:
				assert(false);
				break;
		}
	}
	else
	{
		aOutput = "Variable \"" + variableName + "\" was not found";
		return false;
	}

	return true;
}

std::string Engine::ConsoleCommand_Var::UsageString() const
{
	return "Usage: var <variable name>";
}

std::string Engine::ConsoleCommand_Var::Description() const
{
	return "Gets the variable value and type";
}
