#pragma once

#include "EditorWindow.h"
#include "Common\RingBuffer.h"

namespace Engine
{
	class LogWindow : public EditorWindow
	{
	public:
		struct Line
		{
			std::string myLine;
			ImVec4 myColor;
		};

		LogWindow(Editor& aEditor);
		~LogWindow() override;

		void Draw(const float aDeltaTime) override;

		LogWindow& Info()
		{
			myColor = ImVec4(128.f / 255.f, 1, 0, 1);
			return *this;
		}

		LogWindow& Warning()
		{
			myColor = ImVec4(1, 216.f / 255.f, 0, 1);
			return *this;
		}

		LogWindow& Error()
		{
			myColor = ImVec4(1, 0, 89.f / 255.f, 1);
			return *this;
		}

		template <typename T>
		LogWindow& operator<<(const T& aInput)
		{
			std::time_t t = std::time(0);
			std::tm* now = std::localtime(&t);

			mySS << "[" << now->tm_hour << ":" << now->tm_min << ":" << now->tm_sec << "] ";

			mySS << aInput;

			Line line;
			line.myLine = mySS.str();
			line.myColor = myColor;
			myLinesRing.Add(line);

			mySS.str(std::string());
			mySS.clear();

			return *this;
		}

		std::string GetName() const override { return "Log"; }

		const RingBuffer<Line>& GetLines() const { return myLinesRing; }

	private:
		void LogCallback(const LogType aLogType, const LogCategory aCategory, const std::string& aMessage);

	private:
		ImVec4 myColor;

		std::stringstream mySS;

		// std::vector<Line> myLines;
		RingBuffer<Line> myLinesRing;

		bool myAutoScroll = true;

		Log::HookCallback myCallback;
	};
}
