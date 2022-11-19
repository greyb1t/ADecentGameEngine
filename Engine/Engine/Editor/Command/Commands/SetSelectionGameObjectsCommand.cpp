#include "pch.h"
#include "SetSelectionGameObjectsCommand.h"

#include "Engine/Editor/Editor.h"

Engine::SetSelectionGameObjectsCommand::SetSelectionGameObjectsCommand(
	Selection& aSelection,
	const Selection& aNewSelection)
	: mySelection(aSelection),
	myNewSelection(aNewSelection)
{
}

bool Engine::SetSelectionGameObjectsCommand::Do()
{
	myPreviousSelection = mySelection;

	mySelection = myNewSelection;

	return true;
}

bool Engine::SetSelectionGameObjectsCommand::Undo()
{
	mySelection = myPreviousSelection;

	return true;
}
