#include "pch.h"
#include "IngameConsole.h"
#include "CommandHandler.h"

Engine::IngameConsole::IngameConsole(CommandHandler& aCommandHandler)
	: myCommandHandler(aCommandHandler)
{
}

bool Engine::IngameConsole::Init()
{
	return true;
}

void Engine::IngameConsole::DrawImgui()
{
	const float footerHeight = 20.f;

	ImGui::SetNextWindowSize(ImVec2(500, 300), ImGuiCond_Once);
	if (ImGui::Begin("console baby"))
	{
		// Draw console lines
		if (ImGui::BeginChild("ScrollRegion", ImVec2(0.f, -footerHeight), true, ImGuiWindowFlags_HorizontalScrollbar))
		{
			for (const auto& line : myLines)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, line.myColor);

				ImGui::TextUnformatted(line.myLine.c_str());

				ImGui::PopStyleColor();
			}

			if (myScrollToBottom)
			{
				ImGui::SetScrollHereY(1.0f);
			}
		}
		ImGui::EndChild();

		myScrollToBottom = false;

		// Draw text input
		const ImGuiInputTextFlags textInputFlags =
			ImGuiInputTextFlags_EnterReturnsTrue |
			ImGuiInputTextFlags_CallbackCompletion |
			ImGuiInputTextFlags_CallbackHistory |
			ImGuiInputTextFlags_CallbackEdit;

		const auto callbackStub = [](ImGuiInputTextCallbackData* aData)
		{
			auto console = reinterpret_cast<IngameConsole*>(aData->UserData);
			return console->OnTextEdit(*aData);
		};

		if (ImGui::InputText(
			"Command",
			myCommandTextBuffer,
			std::size(myCommandTextBuffer),
			textInputFlags,
			callbackStub,
			this))
		{
			if (strlen(myCommandTextBuffer) > 0)
			{
				PushCommand(myCommandTextBuffer);
				memset(myCommandTextBuffer, 0, std::size(myCommandTextBuffer));

				myScrollToBottom = true;
			}

			reclaimFocus = true;
		}

		ImGui::SetItemDefaultFocus();

		if (reclaimFocus)
		{
			ImGui::SetKeyboardFocusHere(-1);
		}

		reclaimFocus = false;
	}
	ImGui::End();
}

void Engine::IngameConsole::SetEnabled(const bool aEnabled)
{
	reclaimFocus = true;
	myIsEnabled = aEnabled;
}

int Engine::IngameConsole::OnTextEdit(ImGuiInputTextCallbackData& aData)
{
	switch (aData.EventFlag)
	{
		case ImGuiInputTextFlags_CallbackEdit:
		{
			// Close the console using the same key we opened with
			// Because we give focus to the input text immediately
			// If the last key input into the InputText was the key '§'
			if (aData.Buf[aData.BufTextLen - 2] == static_cast<char>(0xc2))
			{
				memset(myCommandTextBuffer, 0, std::size(myCommandTextBuffer));
				aData.DeleteChars(0, aData.BufTextLen);

				SetEnabled(false);
			}
		} break;

		// When pressing TAB to autocomplete
		case ImGuiInputTextFlags_CallbackCompletion:
		{
			// TODO: Handle this
		} break;

		case ImGuiInputTextFlags_CallbackHistory:
		{
			// Thank you imgui, making my life easy
			const int prevHistory = myHistoryIndex;

			if (aData.EventKey == ImGuiKey_UpArrow)
			{
				if (myHistoryIndex == -1)
				{
					myHistoryIndex = myCommandHistory.size() - 1;
				}
				else if (myHistoryIndex > 0)
				{
					myHistoryIndex--;
				}
			}
			else if (aData.EventKey == ImGuiKey_DownArrow)
			{
				if (myHistoryIndex != -1)
				{
					if (++myHistoryIndex >= myCommandHistory.size())
					{
						myHistoryIndex = -1;
					}
				}
			}

			if (prevHistory != myHistoryIndex)
			{
				const std::string history_str = (myHistoryIndex >= 0) ? myCommandHistory[myHistoryIndex] : "";
				aData.DeleteChars(0, aData.BufTextLen);
				aData.InsertChars(0, history_str.c_str());
			}
		} break;

		default:
			break;
	}

	return 0;
}

void Engine::IngameConsole::PushCommand(const std::string& aCommand)
{
	// Came here if user pressed enter

	// If the command already exists in the history, take it and move it to "top"
	bool foundInHistory = false;

	myHistoryIndex = -1;

	for (int i = 0; i < myCommandHistory.size(); ++i)
	{
		if (myCommandHistory[i] == aCommand)
		{
			// Put it in the back which is also the "top"
			std::swap(myCommandHistory[i], myCommandHistory.back());
			foundInHistory = true;
			break;
		}
	}

	if (!foundInHistory)
	{
		myCommandHistory.push_back(aCommand);
	}

	std::string output;

	if (myCommandHandler.ExecuteCommand(myCommandTextBuffer, output))
	{
		CommandLine cmdLine;
		cmdLine.myLine = output;
		cmdLine.myColor = ImVec4(1.f, 1.f, 1.f, 1.f);

		// White color line
		myLines.push_back(cmdLine);
	}
	else
	{
		CommandLine cmdLine;
		cmdLine.myLine = output;
		cmdLine.myColor = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);

		// Red color line
		myLines.push_back(cmdLine);
	}
}
