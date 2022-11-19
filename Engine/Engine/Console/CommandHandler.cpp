#include "pch.h"

#include "CommandHandler.h"
#include "Commands\ConsoleCommand_Set.h"
#include "Commands\ConsoleCommand_Var.h"
#include "Commands\ConsoleCommand_Help.h"
#include "Commands\ConsoleCommand_Call.h"

bool Engine::CommandHandler::Init()
{
	myCommandHandlers["set"] = MakeOwned<ConsoleCommand_Set>(*this);
	myCommandHandlers["var"] = MakeOwned<ConsoleCommand_Var>(*this);
	myCommandHandlers["help"] = MakeOwned<ConsoleCommand_Help>(*this);
	myCommandHandlers["call"] = MakeOwned<ConsoleCommand_Call>(*this);

	return true;
}

bool Engine::CommandHandler::ExecuteCommand(std::string aCommand, std::string& aOutput)
{
	std::vector<std::string> tokens;

	int argumentCount = 0;

	const auto argumentArray = CommandLineToArgvW(
		std::wstring(aCommand.begin(), aCommand.end()).c_str(), &argumentCount);

	for (int i = 0; i < argumentCount; ++i)
	{
		const std::wstring s = argumentArray[i];
		tokens.push_back(std::string(s.begin(), s.end()));
	}

	LocalFree(argumentArray);

	if (tokens.empty())
	{
		aOutput = "No input";
		return false;
	}

	auto findResult = myCommandHandlers.find(StringUtilities::ToLower(tokens[0]));

	if (findResult != myCommandHandlers.end())
	{
		return findResult->second->Execute(tokens, aOutput);
	}

	aOutput = "Unknown command \"" + aCommand + "\"";

	return false;
}

void Engine::CommandHandler::BindVariable(const std::string& aName, std::string* aVariable)
{
	assert(myBoundVariables.find(aName) == myBoundVariables.end() && "already exists");
	myBoundVariables[aName] = aVariable;
}

void Engine::CommandHandler::BindVariable(const std::string& aName, bool* aVariable)
{
	assert(myBoundVariables.find(aName) == myBoundVariables.end() && "already exists");
	myBoundVariables[aName] = aVariable;
}

void Engine::CommandHandler::BindVariable(const std::string& aName, float* aVariable)
{
	assert(myBoundVariables.find(aName) == myBoundVariables.end() && "already exists");
	myBoundVariables[aName] = aVariable;
}

void Engine::CommandHandler::BindVariable(const std::string& aName, int* aVariable)
{
	assert(myBoundVariables.find(aName) == myBoundVariables.end() && "already exists");
	myBoundVariables[aName] = aVariable;
}

void Engine::CommandHandler::UnbindVariable(const std::string& aName)
{
	myBoundVariables.erase(aName);
}

void Engine::CommandHandler::BindFunction(const std::string& aName, std::function<void()> aFunc)
{
	assert(myFunctions.find(aName) == myFunctions.end() && "already exists");
	myFunctions[aName] = aFunc;
}

void Engine::CommandHandler::UnbindFunction(const std::string& aName)
{
	myFunctions.erase(aName);
}

Engine::CommandHandler::VariableType* Engine::CommandHandler::GetVariable(const std::string& aVariableName)
{
	auto findResult = myBoundVariables.find(aVariableName);

	if (findResult != myBoundVariables.end())
	{
		return &findResult->second;
	}

	return nullptr;
}

Engine::ValType Engine::CommandHandler::VariableToValueType(VariableType& aVariable) const
{
	switch (aVariable.index())
	{
		case 0: return ValType::String;
		case 1: return ValType::Int;
		case 2: return ValType::Bool;
		case 3: return ValType::Float;
		default:
			assert(false);
			break;
	}

	return ValType::Unknown;
}

std::function<void()> Engine::CommandHandler::GetFunction(const std::string& aName)
{
	auto findResult = myFunctions.find(aName);

	if (findResult != myFunctions.end())
	{
		return findResult->second;
	}

	return nullptr;
}
