#include "pch.h"
#include "GraphVariable.h"

void GraphVariable::Set(DataType someType, DataPtr aDataPtr)
{
	assert(myType == someType);
	assert(myType == aDataPtr.GetType());	
	assert(myValue.Ptr);
	myValue.Size = aDataPtr.Size;
	memcpy(myValue.Ptr, aDataPtr.Ptr, aDataPtr.GetSize());
}

void GraphVariable::SetRaw(const char* someData, const size_t someSize)
{
	assert(myValue.GetSize() == someSize);
	assert(myValue.Ptr);
	memcpy(myValue.Ptr, someData, someSize);
}

void GraphVariable::GetRaw(DataType& outDataType, DataPtr& outDataPtr) const
{
	outDataType = myType;
	outDataPtr = DataPtr::Create(myType, myValue.GetSize());
	assert(outDataPtr.Ptr);
	memcpy(outDataPtr.Ptr, myValue.Ptr, myValue.GetSize());
}
