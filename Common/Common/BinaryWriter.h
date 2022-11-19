#pragma once

#include <vector>
#include <string>

class BinaryWriter
{
public:
	bool InitWithSize(const int aBufferSize);

	void Compress(const int aLevel);

	// Returns position it was written at
	template <typename T>
	int Write(const T& aValue);

	// Returns position it was written at
	int WriteString(const std::string& aValue);

	int WriteRawMemory(const void* aBuffer, const int aSize);

	template <typename T>
	void ReplaceAt(const int aPosition, const T& aValue);

	std::vector<uint8_t>& GetFinalBuffer();

	int GetSize() const;

private:
	friend class BinaryReader;

	std::vector<uint8_t> myBuffer;
	int mySize = 0;
};

template <typename T>
int BinaryWriter::Write(const T& aValue)
{
	return WriteRawMemory(reinterpret_cast<const uint8_t*>(&aValue), sizeof(aValue));
}

template <typename T>
void BinaryWriter::ReplaceAt(const int aPosition, const T& aValue)
{
	*reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(myBuffer.data()) + aPosition) = aValue;
}
