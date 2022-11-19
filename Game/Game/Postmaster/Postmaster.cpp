#include "pch.h"
#include "PostMaster.h"
#include "Subscriber.h"

PostMaster PostMaster::ourPostMaster;

PostMaster::PostMaster()
{
	for (int i = 0; i < static_cast<int>(MessageType::Count); ++i)
	{
		mySubscribers.push_back({ });
	}
}

void PostMaster::Subscribe(const MessageType aMessageType, Subscriber* aObserver)
{
	mySubscribers[static_cast<int>(aMessageType)].push_back(aObserver);
}

void PostMaster::Unsubscribe(const MessageType aMessageType, Subscriber* aObserver)
{
	std::vector<Subscriber*>& messageObservers = mySubscribers[static_cast<int>(aMessageType)];

	auto result = std::find(messageObservers.begin(), messageObservers.end(), aObserver);

	if (result != messageObservers.end())
	{
		messageObservers.erase(result);
	}
}

void PostMaster::SendLetter(const Message& aMessage)
{
	for (auto& subscriber : mySubscribers[static_cast<int>(aMessage.myMessageType)])
	{
		subscriber->Receive(aMessage);
	}
}

PostMaster& PostMaster::GetInstance()
{
	return ourPostMaster;
}
