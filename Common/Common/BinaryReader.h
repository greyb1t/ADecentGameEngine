#pragma once

#include "Path.h"

class BinaryWriter;

class BinaryReader
{
public:
	bool InitWithFile(const Path& aPath);
	void InitFromBinaryWriter(const BinaryWriter& aWriter);

	bool UncompressGzip();

	template <typename T>
	T Read();

	const char* ReadString(int* aLengthOut = nullptr);

	template <typename T>
	T* ReadAsPtr();

	template <typename T>
	T* ReadRawMemoryAsPtr(const int aSize);

	void* GetStart();

private:
	friend class BinaryWriter;

	std::vector<uint8_t> myBuffer;
	int mySize = 0;
};

template <typename T>
T BinaryReader::Read()
{
	T* value = reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(myBuffer.data()) + mySize);
	mySize += sizeof(T);
	return *value;
}

template <typename T>
T* BinaryReader::ReadAsPtr()
{
	return ReadRawMemoryAsPtr<T>(sizeof(T));
}

template <typename T>
T* BinaryReader::ReadRawMemoryAsPtr(const int aSize)
{
	T* result = reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(myBuffer.data()) + mySize);
	mySize += aSize;
	return result;
}
