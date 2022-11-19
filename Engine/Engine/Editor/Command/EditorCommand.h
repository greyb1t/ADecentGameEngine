#pragma once

namespace Engine
{
	class EditorCommand
	{
	public:
		virtual ~EditorCommand() = default;

		virtual bool Do() = 0;
		virtual bool Undo() = 0;
	};
}