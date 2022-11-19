#pragma once

#include <stack>
#include "EditorCommand.h"

namespace Engine
{
	class EditorCommandManager
	{
	public:
		EditorCommandManager() = default;

		void Undo();
		void Redo();

		void DoCommand(Owned<EditorCommand> aCommand);

	private:
		std::stack<Owned<EditorCommand>> myUndoStack;
		std::stack<Owned<EditorCommand>> myRedoStack;
	};
}
