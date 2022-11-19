#include "ObjectPoolUntyped.h"

ObjectPoolUntyped::ObjectPoolUntyped(const int aObjectCount, const int aSizeOfObject)
{
	myCapacity = aObjectCount;
	mySizeOfObject = aSizeOfObject;

	myBuffer.resize(aObjectCount * aSizeOfObject);

	for (int i = 0; i < aObjectCount; ++i)
	{
		myFreeObjects.push(reinterpret_cast<uint8_t*>(&myBuffer[i * aSizeOfObject]));
	}
}

ObjectPoolUntyped::~ObjectPoolUntyped()
{
	assert(myFreeObjects.size() == myCapacity && "must free all objects before destroying");
}

int ObjectPoolUntyped::GetCapacity() const
{
	return myCapacity;
}

bool ObjectPoolUntyped::IsFull() const
{
	return myFreeObjects.empty();
}

