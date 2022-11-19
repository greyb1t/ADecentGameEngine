#pragma once

namespace Engine
{
	class CommandHandler;

	class IngameConsole
	{
	public:
		IngameConsole(CommandHandler& aCommandHandler);

		bool Init();

		void DrawImgui();

		void SetEnabled(const bool aEnabled);
		bool IsEnabled() const { return myIsEnabled; }

	private:
		int OnTextEdit(ImGuiInputTextCallbackData& aData);

		void PushCommand(const std::string& aCommand);

		struct CommandLine
		{
			std::string myLine;
			ImVec4 myColor;
		};

	private:
		CommandHandler& myCommandHandler;
		bool myIsEnabled = false;

		std::vector<CommandLine> myLines;

		std::vector<std::string> myCommandHistory;
		int myHistoryIndex = -1;

		char myCommandTextBuffer[1024] = {};
		bool reclaimFocus = false;

		bool myScrollToBottom = false;
	};
}