#include "pch.h"
#include "ConsoleCommand_Set.h"
#include "..\CommandHandler.h"

Engine::ConsoleCommand_Set::ConsoleCommand_Set(CommandHandler& aCommandHandler)
	: myCommandHandler(aCommandHandler)
{
}

bool Engine::ConsoleCommand_Set::Execute(const std::vector<std::string>& aTokens, std::string& aOutput)
{
	assert(aTokens[0] == "set");

	if (aTokens.size() < 3)
	{
		aOutput = UsageString();

		return false;
	}

	const auto& variableName = aTokens[1];
	const auto& value = aTokens[2];

	if (auto variable = myCommandHandler.GetVariable(variableName))
	{
		switch (myCommandHandler.VariableToValueType(*variable))
		{
			case ValType::String:
				*std::get<std::string*>(*variable) = value;
				break;
			case ValType::Bool:
			{
				if (value == "0")
				{
					*std::get<bool*>(*variable) = false;
				}
				else if (value == "1")
				{
					*std::get<bool*>(*variable) = true;
				}
				else if (value == "false")
				{
					*std::get<bool*>(*variable) = false;
				}
				else if (value == "true")
				{
					*std::get<bool*>(*variable) = true;
				}
				else
				{
					aOutput = "value \"" + value + "\" was incorrect type, expected bool";
					return false;
				}
			} break;

			case ValType::Int:
			{
				try
				{
					*std::get<int*>(*variable) = std::stoi(value);
				}
				catch (std::invalid_argument&)
				{
					aOutput = "value \"" + value + "\" not be parsed as an int";
					return false;
				}
				catch (std::out_of_range&)
				{
					aOutput = "value \"" + value + "\" too big";
					return false;
				}
			} break;

			case ValType::Float:
			{
				try
				{
					*std::get<float*>(*variable) = std::stof(value);
				}
				catch (std::invalid_argument&)
				{
					aOutput = "value \"" + value + "\" could not be parsed as an float";
					return false;
				}
				catch (std::out_of_range&)
				{
					aOutput = "value \"" + value + "\" too big";
					return false;
				}
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

	aOutput = "Updated variable \"" + variableName + "\" to " + value;

	return true;
}

std::string Engine::ConsoleCommand_Set::UsageString() const
{
	return "Usage: set <variable name> <value>";
}

std::string Engine::ConsoleCommand_Set::Description() const
{
	return "Sets a variable value";
}
