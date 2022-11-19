#pragma once

#include "Message.h"

class Subscriber
{
public:
	virtual void Receive(const Message& aMessage) = 0;
};