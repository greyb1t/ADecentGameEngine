#pragma once

#include <iosfwd>
#include <array>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <bitset>
#include <functional>
#include <mutex>

#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

enum class LogCategory
{
	Info,
	Warning,
	Error
};

enum class LogType
{
	Animation,
	Components,
	Game,
	Renderer,
	Engine,
	Resource,
	Audio,
	Physics,
	Particles,
	Editor,
	Video,
	AIAnimation,
	AIPlacement,
	Items,

	Filip,
	Jonathan,
	Viktor,
	Jesper,
	Markus,
	Simon,
	Pontus,
	Bjorn,
	William,

	Count
};

#if defined(_RETAIL) || defined(TRACY_ENABLE)
#define LOG_INFO(logType) Log::Empty(logType)
#define LOG_WARNING(logType) Log::Empty(logType)
#define LOG_ERROR(logType) Log::Empty(logType)
#else
#define LOG_INFO(logType) Log::Info(logType)
#define LOG_WARNING(logType) Log::Warning(logType)
#define LOG_ERROR(logType) Log::Error(logType, __LINE__, __FILENAME__)
#endif

extern std::mutex globalLogMutex;

std::string LogChannelEnumToString(const LogType aLogChannelEnum);

class LogChannel;

class ScopedLog
{
public:
	ScopedLog(LogChannel& aLogChannel, const LogCategory aCategory, const std::string& aExtra = "");

	~ScopedLog();

	template <typename T>
	ScopedLog& operator<<(const T& aInput)
	{
		// myLogChannel << aInput;
		GetThreadLocalStringStream() << aInput;
		return *this;
	}

	std::stringstream& GetThreadLocalStringStream();

private:
	LogChannel& myLogChannel;
	LogCategory myCategory;
};

// Only used to placeholder in retail
class ScopedLogEmpty
{
public:
	ScopedLogEmpty() = default;

	~ScopedLogEmpty() = default;

	template <typename T>
	ScopedLogEmpty& operator<<(const T& aInput)
	{
		return *this;
	}
};

class Log
{
public:
	using HookCallback = std::function<void(
		const LogType aLogType, 
		const LogCategory aCategory, 
		const std::string& aMessage)>;

	static void Create();

	static void Destroy();

	static void SetIsLoggingEnabled(const bool aEnabled);
	static bool GetIsLoggingEnabled();

	static ScopedLog Info(const LogType aChannel);
	static ScopedLog Warning(const LogType aChannel);
	static ScopedLog Error(const LogType aChannel, const int aLine, const std::string& aFile);
	static ScopedLogEmpty Empty(const LogType aChannel);

	static void SetChannelState(const LogType aChannel, const bool aEnabled);
	static bool GetChannelState(const LogType aChannel);

	static HookCallback AddHook(const HookCallback aCallback);
	static void RemoveHook(const HookCallback aCallback);

private:
	friend class LogChannel;
	friend class ScopedLog;

	inline static std::ostream& myOutputStream = std::cout;

	inline static std::array<LogChannel*, static_cast<int>(LogType::Count)> myChannels;

	inline static std::vector<HookCallback> myHooks;

	inline static bool myIsLoggingEnabled = true;
};

class LogChannel
{
public:
	LogChannel(LogType aChannelEnum);

	template <typename T>
	LogChannel& operator<<(const T& aInput);

	void Reset();

	void SetIsEnabled(const bool aIsEnabled);

	bool GetIsEnabled() const;

	LogType GetChannelEnum() const;
	const std::string& GetChannelEnumString() const;

private:
	LogType myChannelEnum;
	std::string myChannelEnumString;

	bool myIsEnabled = false;

	// http://cplusplus.bordoon.com/speeding_up_string_conversions.html
	// std::stringstream is slow because it needs to be locale data in constructor
	// make it static to fix that, then manually reset it when you want to be clean
	inline static std::stringstream myStringStream;
};

template <typename T>
LogChannel& LogChannel::operator<<(const T& aInput)
{
	if (!Log::myIsLoggingEnabled)
	{
		return *this;
	}

	{
		// std::scoped_lock lock(globalLogMutex);

		myStringStream << aInput;

		if (myIsEnabled)
		{
			// TODO: Hook the loggin for the editor
			// TODO: Call hooks here
			/*
			 * 	Problem:
				Nvm, not a problem since we use a fucking mutex in the logging lol.
				Only one thread can write a log at a time. EZ.
			 */

			Log::myOutputStream << myStringStream.str();
		}

		Reset();
	}

	return *this;
}
