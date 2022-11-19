#pragma once

#include <stdint.h>
#include <memory>

class LinearAllocator
{
public:
	LinearAllocator() = default;
	LinearAllocator(const size_t aCapacityBytes);
	~LinearAllocator();

	// Returns nullptr if out of memory
	template <typename T>
	T* Allocate(const size_t aCount = 1);

	void SetOffset(const size_t aOffset);
	size_t GetOffset() const;

	void Clear();

private:
	void* myStart = nullptr;
	size_t myOffset = 0;
	size_t myCapacity = 0;
};

template <typename T>
T* LinearAllocator::Allocate(const size_t aCount)
{
	const size_t totalBytes = sizeof(T) * aCount;

	void* currentAddr = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(myStart) + myOffset);

	size_t spaceLeft = myCapacity - myOffset;

	if (std::align(alignof(T), totalBytes, currentAddr, spaceLeft))
	{
		myOffset = myCapacity - spaceLeft + totalBytes;

		return reinterpret_cast<T*>(currentAddr);
	}

	return nullptr;
}
