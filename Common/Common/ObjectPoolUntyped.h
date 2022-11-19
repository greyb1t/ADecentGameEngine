#pragma once

#include <queue>
#include <stdint.h>
#include <utility>
#include <assert.h>

class ObjectPoolUntyped
{
public:
	ObjectPoolUntyped(const int aObjectCount, const int aSizeOfObject);
	~ObjectPoolUntyped();

	ObjectPoolUntyped(const ObjectPoolUntyped&) = delete;
	ObjectPoolUntyped(ObjectPoolUntyped&&) = delete;

	template <typename T, typename... Args>
	T* Allocate(Args&&... aConstructorArgs);

	template <typename T>
	void Free(T*& aObject, const bool aUnsafe = false);

	int GetCapacity() const;

	bool IsFull() const;

private:
	// Why am I using a vector of uint8_t instead of a vector of Enemy?
	// Because in this way I do not need to default construct the 
	// Enemy when calling .resize() on the vector
	// It removes the requirement for an object to be default constructible
	// also it avoids the issue that the first time, we call Allocate() on an 
	// object, the destructor of the initial default constructed object won't execute
	std::vector<uint8_t> myBuffer;

	std::queue<uint8_t*> myFreeObjects;

	int mySizeOfObject = 0;
	int myCapacity = 0;
};

template <typename T, typename... Args>
T* ObjectPoolUntyped::Allocate(Args&&... aConstructorArgs)
{
	assert(!myFreeObjects.empty() && "no more objects in the pool");
	assert(mySizeOfObject == sizeof(T) && "object type size is not the same type as the pool");

	T* freeObject = reinterpret_cast<T*>(myFreeObjects.front());
	myFreeObjects.pop();

	freeObject = new (freeObject) T(std::forward<Args>(aConstructorArgs)...);

	return freeObject;
}

template <typename T>
void ObjectPoolUntyped::Free(T*& aObject, const bool aUnsafe)
{
	assert(aObject != nullptr);

	if (!aUnsafe)
	{
		assert(mySizeOfObject == sizeof(T) && "object type size is not the same type as the pool");
	}

	// absolutely disgusting
	// NOTE: This seems to work even with the case that T* is the base class which is great :)
	aObject->~T();

	myFreeObjects.push(reinterpret_cast<uint8_t*>(aObject));

	aObject = nullptr;
}
