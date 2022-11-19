#include "pch.h"
#include "EditorCommandManager.h"
#include "EditorCommand.h"

void Engine::EditorCommandManager::Undo()
{
	if (myUndoStack.empty())
	{
		return;
	}

	Owned<EditorCommand> command = std::move(myUndoStack.top());
	command->Undo();
	myUndoStack.pop();

	myRedoStack.push(std::move(command));
}

void Engine::EditorCommandManager::Redo()
{
	if (myRedoStack.empty())
	{
		return;
	}

	Owned<EditorCommand> command = std::move(myRedoStack.top());
	command->Do();
	myRedoStack.pop();

	myUndoStack.push(std::move(command));
}

void Engine::EditorCommandManager::DoCommand(Owned<EditorCommand> aCommand)
{
	aCommand->Do();
	myUndoStack.push(std::move(aCommand));
	myRedoStack = std::stack<Owned<EditorCommand>>();
}
