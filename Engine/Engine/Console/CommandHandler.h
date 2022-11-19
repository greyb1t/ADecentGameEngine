#pragma once

#include "ConsoleCommand.h"

namespace Engine
{
	enum class ValType
	{
		String,
		Bool,
		Int,
		Float,

		Unknown
	};

	class CommandHandler
	{
	public:
		void BindVariable(const std::string& aName, std::string* aVariable);
		void BindVariable(const std::string& aName, int* aVariable);
		void BindVariable(const std::string& aName, float* aVariable);
		void BindVariable(const std::string& aName, bool* aVariable);

		void UnbindVariable(const std::string& aName);

		void BindFunction(const std::string& aName, std::function<void()> aFunc);
		void UnbindFunction(const std::string& aName);

		bool Init();

		// Returns true if succeeded
		bool ExecuteCommand(std::string aCommand, std::string& aOutput);

	private:
		friend class ConsoleCommand_Call;
		friend class ConsoleCommand_Help;
		friend class ConsoleCommand_Set;
		friend class ConsoleCommand_Var;

		using VariableType = std::variant<std::string*, int*, bool*, float*>;

		// Returns nullptr if not found
		VariableType* GetVariable(const std::string& aVariableName);
		ValType VariableToValueType(CommandHandler::VariableType& aVariable) const;

		const std::unordered_map<std::string, Owned<ConsoleCommand>>& GetCommands() const { return myCommandHandlers; }
		const std::unordered_map<std::string, VariableType>& GetBoundVariables() const { return myBoundVariables; }

		std::function<void()> GetFunction(const std::string& aName);

	private:
		std::unordered_map<std::string, Owned<ConsoleCommand>> myCommandHandlers;

		std::unordered_map<std::string, VariableType> myBoundVariables;
		std::unordered_map<std::string, std::function<void()>> myFunctions;
	};
}