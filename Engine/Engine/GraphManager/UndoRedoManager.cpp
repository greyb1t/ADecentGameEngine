#include "pch.h"
#include "UndoRedoManager.h"
#include "Command.h"

UndoRedoManager::UndoRedoManager(GraphManager& aGraphManager)
	: myGraphManager(aGraphManager)
{
}

void UndoRedoManager::Undo()
{
	if (myUndoStack.empty())
	{
		return;
	}

	Command* command = myUndoStack.top();
	command->Undo();
	myUndoStack.pop();

	myRedoStack.push(command);
}

void UndoRedoManager::Redo()
{
	if (myRedoStack.empty())
	{
		return;
	}

	Command* command = myRedoStack.top();
	command->Execute();
	myRedoStack.pop();

	myUndoStack.push(command);
}

void UndoRedoManager::ExecuteCommand(Command* aCommand)
{
	aCommand->Execute();
	myUndoStack.push(aCommand);
	myRedoStack = std::stack<Command*>();
}
