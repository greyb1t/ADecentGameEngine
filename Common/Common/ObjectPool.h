#pragma once

#include <queue>

template <typename T>
class ObjectPool
{
public:
	ObjectPool(const int aObjectCount);

	template <typename... Args>
	T* Allocate(Args&&... aConstructorArgs);

	T* Allocate();

	void Free(T*& aObject);

private:
	// Why am I using a vector of uint8_t instead of a vector of Enemy?
	// Because in this way I do not need to default construct the 
	// Enemy when calling .resize() on the vector
	// It removes the requirement for an object to be default constructible
	// also it avoids the issue that the first time, we call Allocate() on an 
	// object, the destructor of the initial default constructed object won't execute
	std::vector<uint8_t> myBuffer;

	std::queue<T*> myFreeObjects;
};

template <typename T>
ObjectPool<T>::ObjectPool(const int aObjectCount)
{
	myBuffer.resize(aObjectCount * sizeof(T));

	for (int i = 0; i < aObjectCount; ++i)
	{
		myFreeObjects.push(reinterpret_cast<T*>(&myBuffer[i * sizeof(T)]));
	}
}

template <typename T>
template <typename... Args>
T* ObjectPool<T>::Allocate(Args&&... aConstructorArgs)
{
	assert(!myFreeObjects.empty() && "no more objects in the pool");

	T* freeObject = myFreeObjects.front();
	myFreeObjects.pop();

	freeObject = new(freeObject) T(aConstructorArgs...);

	return freeObject;
}

template <typename T>
T* ObjectPool<T>::Allocate()
{
	assert(!myFreeObjects.empty() && "no more objects in the pool");

	T* freeObject = myFreeObjects.front();
	myFreeObjects.pop();

	return freeObject;
}

template <typename T>
void ObjectPool<T>::Free(T*& aObject)
{
	assert(aObject != nullptr);

	// absolutely disgusting
	aObject->~T();

	myFreeObjects.push(aObject);

	aObject = nullptr;
}