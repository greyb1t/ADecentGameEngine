#pragma once

class GraphManager;
class Command;

class UndoRedoManager
{
public:
	UndoRedoManager(GraphManager& aGraphManager);

	void Undo();
	void Redo();

	void ExecuteCommand(Command* aCommand);

private:
	GraphManager& myGraphManager;

	std::stack<Command*> myUndoStack;
	std::stack<Command*> myRedoStack;
};