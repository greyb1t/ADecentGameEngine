#include "BinaryWriter.h"

#include <libdeflate/libdeflate.h>
#include <assert.h>

bool BinaryWriter::InitWithSize(const int aBufferSize)
{
	myBuffer.resize(aBufferSize);
	return true;
}

void BinaryWriter::Compress(const int aLevel)
{
	auto compressor = libdeflate_alloc_compressor(aLevel);

	const void* uncompressed_data = myBuffer.data();
	size_t uncompressed_size = mySize;

	void* compressed_data;
	size_t actual_compressed_size;
	size_t max_compressed_size;

	max_compressed_size = libdeflate_gzip_compress_bound(compressor, uncompressed_size);
	compressed_data = malloc(max_compressed_size);

	actual_compressed_size = libdeflate_gzip_compress(
		compressor, uncompressed_data, uncompressed_size, compressed_data, max_compressed_size);

	libdeflate_free_compressor(compressor);

	// TODO: UNOPTIMIZED, WE COPY THE DATA FOR NO REASON, WHY NOT KEEP IT IN THE ALLOCATED BUFFER ALREADY?
	myBuffer.clear();
	myBuffer.resize(actual_compressed_size);
	mySize = actual_compressed_size;
	memcpy(&myBuffer[0], compressed_data, actual_compressed_size);

	free(compressed_data);
}

int BinaryWriter::WriteString(const std::string& aValue)
{
	const int size = aValue.size() + 1;
	const int pos = Write<uint32_t>(size);

	// We include the null terminator
	WriteRawMemory(reinterpret_cast<const uint8_t*>(&aValue[0]), size);

	return pos;
}

int BinaryWriter::WriteRawMemory(const void* aBuffer, const int aSize)
{
	while ((mySize + aSize) >= myBuffer.size())
	{
		myBuffer.resize(myBuffer.size() * 2 + 1);
	}

	assert(myBuffer.size() > (mySize + aSize) && "the resize was not enough");

	memcpy(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(myBuffer.data()) + mySize), aBuffer, aSize);

	const int writtenAtPosition = mySize;

	mySize += aSize;

	return writtenAtPosition;
}

std::vector<uint8_t>& BinaryWriter::GetFinalBuffer()
{
	// Shrink it to fit exactly same size
	myBuffer.resize(mySize);

	return myBuffer;
}

int BinaryWriter::GetSize() const
{
	return mySize;
}
