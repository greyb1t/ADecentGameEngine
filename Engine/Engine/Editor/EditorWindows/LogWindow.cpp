#include "pch.h"
#include "LogWindow.h"

Engine::LogWindow::LogWindow(Editor& aEditor)
	: EditorWindow(aEditor),
	myLinesRing(150)
{
	myCallback = Log::AddHook([this](const LogType aLogType, const LogCategory aCategory, const std::string& aMessage)
		{
			LogCallback(aLogType, aCategory, aMessage);
		});
}

Engine::LogWindow::~LogWindow()
{
	Log::RemoveHook(myCallback);
}

void Engine::LogWindow::Draw(const float aDeltaTime)
{
	ZoneNamedN(zone1, "LogWindow::Update", true);

	const float footerHeight = 0.f;

	{
		if (ImGui::BeginChild("ScrollRegion",
			ImVec2(0.f, -footerHeight),
			false,
			ImGuiWindowFlags_HorizontalScrollbar))
		{
			RingBuffer<Line> lines;

			// Must lock globa log mutex, otherwise another thread could log and write to myLines
			// which is race condition
			// this solution is bad because it copies the whole vector of strings
			// however, better than a crash ;)
			{
				std::scoped_lock lock(globalLogMutex);

				lines = myLinesRing;
			}

			for (int i = static_cast<int>(myLinesRing.GetCount() - 1); i >= 0; --i)
			{
				const auto& line = myLinesRing[i];

				ImGui::PushStyleColor(ImGuiCol_Text, line.myColor);

				ImGui::TextUnformatted(line.myLine.c_str());

				ImGui::PopStyleColor();
			}

			if (myAutoScroll)
			{
				ImGui::SetScrollHereY(1.0f);
			}
		}
		ImGui::EndChild();
	}
}

void Engine::LogWindow::LogCallback(const LogType aLogType, const LogCategory aCategory, const std::string& aMessage)
{
	switch (aCategory)
	{
		case LogCategory::Info:
			Info() << aMessage;
			break;
		case LogCategory::Warning:
			Warning() << aMessage;
			break;
		case LogCategory::Error:
			Error() << aMessage;
			break;
		default:
			assert(false);
			break;
	}
}
