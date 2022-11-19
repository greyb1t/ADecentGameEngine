#include "LinearAllocator.h"

LinearAllocator::LinearAllocator(const size_t aCapacity)
{
	// allocates uninitialized memory
	myStart = ::operator new(aCapacity);
	myCapacity = aCapacity;
}

LinearAllocator::~LinearAllocator()
{
	::operator delete(myStart);
	myStart = nullptr;
}

void LinearAllocator::SetOffset(const size_t aOffset)
{
	myOffset = aOffset;
}

size_t LinearAllocator::GetOffset() const
{
	return myOffset;
}

void LinearAllocator::Clear()
{
	myOffset = 0;
}
