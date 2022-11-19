#pragma once

#include <vector>

template <typename T>
class RingBuffer
{
public:
	RingBuffer() = default;
	RingBuffer(const size_t aMaxCount);

	void Add(const T& aItem);
	void Clear();

	T& operator[](const size_t aIndex);
	const T& operator[](const size_t aIndex) const;

	size_t GetCount() const;
	size_t GetMaxCount() const;

private:
	void NextFrontIndex();

private:
	std::vector<T> myBuffer;
	size_t myMaxCount = 0;
	size_t myFrontIndex = 0;
};

template <typename T>
const T& RingBuffer<T>::operator[](const size_t aIndex) const
{
	const size_t correctedIndex =
		(aIndex >= myFrontIndex) ?
		myBuffer.size() - 1 - (aIndex - myFrontIndex) :
		myFrontIndex - 1 - aIndex;

	return myBuffer[correctedIndex];
}

template <typename T>
T& RingBuffer<T>::operator[](const size_t aIndex)
{
	const size_t correctedIndex =
		(aIndex >= myFrontIndex) ?
		myBuffer.size() - 1 - (aIndex - myFrontIndex) :
		myFrontIndex - 1 - aIndex;

	return myBuffer[correctedIndex];
}

template <typename T>
void RingBuffer<T>::Clear()
{
	myBuffer.clear();
	myFrontIndex = 0;
}

template <typename T>
void RingBuffer<T>::NextFrontIndex()
{
	myFrontIndex = (myFrontIndex + 1) % myMaxCount;
}

template <typename T>
size_t RingBuffer<T>::GetMaxCount() const
{
	return myMaxCount;
}

template <typename T>
RingBuffer<T>::RingBuffer(const size_t aMaxCount)
	: myMaxCount(aMaxCount)
{
}

template <typename T>
void RingBuffer<T>::Add(const T& aItem)
{
	if (GetCount() < myMaxCount)
	{
		myBuffer.push_back(aItem);
		NextFrontIndex();
	}
	else
	{
		myBuffer[myFrontIndex] = aItem;
		NextFrontIndex();
	}
}

template <typename T>
size_t RingBuffer<T>::GetCount() const
{
	return myBuffer.size();
}
