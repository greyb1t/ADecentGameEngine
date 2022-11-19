#pragma once

#include <any>

enum class MessageType
{
	Test,

	//UI
	UIButtonClicked,

	//Leave this last (Warning)
	Count,
};

struct Message
{
	Message() = default;
	Message(const MessageType aMessageType);

	MessageType myMessageType;

	std::any myValue;
};
