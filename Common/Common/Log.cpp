#include "Log.h"

#include <Windows.h>
#include <assert.h>

std::mutex globalLogMutex;

namespace ConsoleForeground
{
	// Thank you: https://stackoverflow.com/questions/17125440/c-win32-console-color/17125539
	enum {
		BLACK = 0,
		DARKBLUE = FOREGROUND_BLUE,
		DARKGREEN = FOREGROUND_GREEN,
		DARKCYAN = FOREGROUND_GREEN | FOREGROUND_BLUE,
		DARKRED = FOREGROUND_RED,
		DARKMAGENTA = FOREGROUND_RED | FOREGROUND_BLUE,
		DARKYELLOW = FOREGROUND_RED | FOREGROUND_GREEN,
		DARKGRAY = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
		GRAY = FOREGROUND_INTENSITY,
		BLUE = FOREGROUND_INTENSITY | FOREGROUND_BLUE,
		GREEN = FOREGROUND_INTENSITY | FOREGROUND_GREEN,
		CYAN = FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE,
		RED = FOREGROUND_INTENSITY | FOREGROUND_RED,
		MAGENTA = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE,
		YELLOW = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN,
		WHITE = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
	};
}

void SetConsoleColor(int aColor)
{
	static HANDLE hConsole = hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	assert(hConsole != NULL);

	SetConsoleTextAttribute(hConsole, static_cast<WORD>(aColor));
}

void Log::Create()
{
	for (int i = 0; i < myChannels.size(); ++i)
	{
		myChannels[i] = new LogChannel(static_cast<LogType>(i));
	}
}

void Log::Destroy()
{
	for (int i = 0; i < myChannels.size(); ++i)
	{
		delete myChannels[i];
	}
}

void Log::SetIsLoggingEnabled(const bool aEnabled)
{
	myIsLoggingEnabled = aEnabled;
}

bool Log::GetIsLoggingEnabled()
{
	return myIsLoggingEnabled;
}

ScopedLog Log::Info(const LogType aChannel)
{
	return ScopedLog(*myChannels[static_cast<int>(aChannel)], LogCategory::Info);
}

ScopedLog Log::Warning(const LogType aChannel)
{
	SetConsoleColor(ConsoleForeground::YELLOW);

	return ScopedLog(*myChannels[static_cast<int>(aChannel)], LogCategory::Warning);
}

ScopedLog Log::Error(const LogType aChannel, const int aLine, const std::string& aFile)
{
	return ScopedLog(
		*myChannels[static_cast<int>(aChannel)],
		LogCategory::Error,
		"(" + aFile + "+L" + std::to_string(aLine) + ") ");
}

ScopedLogEmpty Log::Empty(const LogType aChannel)
{
	return ScopedLogEmpty();
}

void Log::SetChannelState(const LogType aChannel, const bool aEnabled)
{
	myChannels[static_cast<int>(aChannel)]->SetIsEnabled(aEnabled);
}

bool Log::GetChannelState(const LogType aChannel)
{
	return myChannels[static_cast<int>(aChannel)]->GetIsEnabled();
}

Log::HookCallback Log::AddHook(const HookCallback aCallback)
{
	std::scoped_lock lock(globalLogMutex);
	myHooks.push_back(aCallback);
	return myHooks.back();
}

void Log::RemoveHook(const HookCallback aCallback)
{
	std::scoped_lock lock(globalLogMutex);
	auto result = std::find_if(myHooks.begin(), myHooks.end(),
		[aCallback](const HookCallback callback)
		{
			return callback.target<HookCallback>() == aCallback.target<HookCallback>();
		});
	if (result != myHooks.end())
	{
		myHooks.erase(result);
	}
}

LogChannel::LogChannel(LogType aChannelEnum)
	: myChannelEnum(aChannelEnum),
	myChannelEnumString(LogChannelEnumToString(aChannelEnum))
{
}

void LogChannel::Reset()
{
	myStringStream.str(std::string());
	myStringStream.clear();
}

void LogChannel::SetIsEnabled(const bool aIsEnabled)
{
	myIsEnabled = aIsEnabled;
}

bool LogChannel::GetIsEnabled() const
{
	return myIsEnabled;
}

LogType LogChannel::GetChannelEnum() const
{
	return myChannelEnum;
}

const std::string& LogChannel::GetChannelEnumString() const
{
	return myChannelEnumString;
}

std::string LogChannelEnumToString(const LogType aLogChannelEnum)
{
	switch (aLogChannelEnum)
	{
	case LogType::Animation: return "Animation";
	case LogType::Game: return "Game";
	case LogType::Renderer: return "Renderer";
	case LogType::Engine: return "Engine";
	case LogType::Resource: return "Resource";
	case LogType::Audio: return "Audio";
	case LogType::Physics: return "Physics";
	case LogType::Editor: return "Editor";
	case LogType::Video: return "Video";
	case LogType::AIAnimation: return "AIAnimation";
	case LogType::AIPlacement: return "AIPlacement";
	case LogType::Filip: return "Filip";
	case LogType::Items: return "Items";
	case LogType::Jonathan: return "Jonathan";
	case LogType::Viktor: return "Viktor";
	case LogType::Jesper: return "Jesper";
	case LogType::Markus: return "Markus";
	case LogType::Simon: return "Simon";
	case LogType::Pontus: return "Pontus";
	case LogType::Bjorn: return "Bjorn";
	case LogType::Components: return "Components";
	case LogType::William: return "William";
	default:
		return "unhandled case";
		break;
	}
}

ScopedLog::ScopedLog(LogChannel& aLogChannel, const LogCategory aCategory, const std::string& aExtra)
	: myLogChannel(aLogChannel),
	myCategory(aCategory)
{
	// SetConsoleColor(aColor);
	// myLogChannel << "[" + myLogChannel.GetChannelEnumString() + "] " + aExtra;

	GetThreadLocalStringStream() << "[" + myLogChannel.GetChannelEnumString() + "] " + aExtra;
}

ScopedLog::~ScopedLog()
{
	// We must lock like this, if we locked in constructor and then unlocked in destructor
	// WHOLE FUCKING SCOPES WOULD BE LOCKED WHERE WE WOULD CALL LOG
	// ABSOLUTELY HORRIBLE DUDE
	std::scoped_lock lock(globalLogMutex);

	switch (myCategory) {
	case LogCategory::Info:
		SetConsoleColor(ConsoleForeground::GREEN);
		break;
	case LogCategory::Warning:
		SetConsoleColor(ConsoleForeground::YELLOW);
		break;
	case LogCategory::Error:
		SetConsoleColor(ConsoleForeground::RED);
		break;
	default:
		assert(false);
		break;
	}

	GetThreadLocalStringStream() << "\n";

	const std::string s = GetThreadLocalStringStream().str();

	if (myLogChannel.GetIsEnabled())
	{
		myLogChannel << s;
		
		for (const auto& hook : Log::myHooks)
		{
			hook(myLogChannel.GetChannelEnum(), myCategory, s);
		}
	}

	GetThreadLocalStringStream().str(std::string());
	GetThreadLocalStringStream().clear();
}

std::stringstream& ScopedLog::GetThreadLocalStringStream()
{
	static thread_local std::stringstream ss;
	return ss;
}
