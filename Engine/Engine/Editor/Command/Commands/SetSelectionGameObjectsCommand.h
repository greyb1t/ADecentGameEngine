#pragma once

#include "Engine/Editor/Selection/Selection.h"
#include "Engine/Editor/Command/EditorCommand.h"

class GameObject;

namespace Engine
{
class Editor;

class SetSelectionGameObjectsCommand : public EditorCommand
{
public:
	SetSelectionGameObjectsCommand(Selection& aSelection, const Selection& aNewSelection);

	bool Do() override;
	bool Undo() override;

private:
	friend class GameObjectManager;

	Selection& mySelection;

	Selection myNewSelection;
	Selection myPreviousSelection;
};
}
