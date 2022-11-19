#pragma once

#include <vector>

#include "Message.h"

class Subscriber;

class PostMaster
{
public:
	PostMaster();

	void Subscribe(const MessageType aMessageType, Subscriber* aSubscriber);
	void Unsubscribe(const MessageType aMessageType, Subscriber* aSubscriber);

	void SendLetter(const Message& aMessage);

	static PostMaster& GetInstance();

private:
	std::vector<std::vector<Subscriber*>> mySubscribers;

	static PostMaster ourPostMaster;
};