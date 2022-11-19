#pragma once
#include "EditorCommand.h"

namespace Engine
{
	class EditorCommand;

	// A class that combines several commands that acts as one
	// E.g. when we want to create new gameobject, we both add it to scene and select it
	// we want that in ONE undo, therefore we use this to make them into one
	class EditorCommandComposite : public EditorCommand
	{
	public:
		void AddCommand(Owned<EditorCommand> aCommand);

		bool Do() override;
		bool Undo() override;

	private:
		std::vector<Owned<EditorCommand>> myEditorCommands;
	};
}
