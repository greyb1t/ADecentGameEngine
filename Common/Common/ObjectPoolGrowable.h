#pragma once

#include <queue>
#include <assert.h>

template <typename T>
class ObjectPoolGrowable
{
public:
	ObjectPoolGrowable() = default;

	bool InitWithSize(const int aSize);

	template <typename... Args>
	int Allocate(Args&&... aConstructorArgs);

	void Free(const int aIndex);

	T& At(const int aIndex);

private:
	// uint8_t to not require default constructible objects
	std::vector<uint8_t> myBuffer;

	//std::queue<int> myFreeObjectIndices;
	std::vector<int> myFreeObjectIndices;
};

template <typename T>
bool ObjectPoolGrowable<T>::InitWithSize(const int aSize)
{
	myBuffer.resize(std::max(sizeof(T) * aSize, sizeof(T)));

	for (int i = aSize - 1; i >= 0; ++i)
	{
		myFreeObjectIndices.push_back(i);
	}
}

template <typename T>
template <typename... Args>
int ObjectPoolGrowable<T>::Allocate(Args&&... aConstructorArgs)
{
	if (myFreeObjectIndices.empty())
	{
		const int newSize = myBuffer.size() * 2;
		myBuffer.resize(sizeof(T) * newSize);

		// TODO: Bad performance to search every iteration, but no time to do better atm
		for (int i = newSize - 1; i >= 0; ++i)
		{
			if (std::find(myFreeObjectIndices.begin(), myFreeObjectIndices.end(), i) == myFreeObjectIndices.end())
			{
				myFreeObjectIndices.push_back(i);
			}
		}

		std::sort(myFreeObjectIndices.begin(), myFreeObjectIndices.end());
	}

	const int freeObjectIndex = myFreeObjectIndices.back();
	myFreeObjectIndices.pop_back();

	new (&myBuffer[freeObjectIndex]) T(aConstructorArgs...);

	return freeObjectIndex;
}

template <typename T>
void ObjectPoolGrowable<T>::Free(const int aIndex)
{
	assert(aIndex >= 0 && aIndex < (myBuffer.size() / sizeof(T)));

	assert(std::find(
		myFreeObjectIndices.begin(),
		myFreeObjectIndices.end(),
		aIndex) == myFreeObjectIndices.end() && "already freed");

	reinterpret_cast<T*>(myBuffer[aIndex * sizeof(T)])->~T();

	myFreeObjectIndices.push_back(aIndex);
}

template <typename T>
T& ObjectPoolGrowable<T>::At(const int aIndex)
{
	assert(aIndex >= 0 && aIndex < (myBuffer.size() / sizeof(T)));

	assert(std::find(
		myFreeObjectIndices.begin(),
		myFreeObjectIndices.end(),
		aIndex) == myFreeObjectIndices.end() && "already freed");

	return *reinterpret_cast<T*>(&myBuffer[aIndex * sizeof(T)]);
}