#include "pch.h"
#include "EditorCommandComposite.h"

void Engine::EditorCommandComposite::AddCommand(Owned<EditorCommand> aCommand)
{
	myEditorCommands.push_back(std::move(aCommand));
}

bool Engine::EditorCommandComposite::Do()
{
	for (auto& command : myEditorCommands)
	{
		command->Do();
	}

	return true;
}

bool Engine::EditorCommandComposite::Undo()
{
	for (auto& command : myEditorCommands)
	{
		command->Undo();
	}

	return true;
}
