#include "BinaryReader.h"

#include <fstream>
#include "libdeflate\libdeflate.h"
#include "BinaryWriter.h"

bool BinaryReader::InitWithFile(const Path& aPath)
{
	std::ifstream file(aPath.ToWString(), std::ifstream::binary);

	if (!file.is_open())
	{
		return false;
	}

	file.seekg(0, file.end);
	const int length = file.tellg();
	file.seekg(0, file.beg);

	myBuffer.resize(length);

	file.read(reinterpret_cast<char*>(myBuffer.data()), length);

	if (myBuffer.size() == 0)
	{
		return false;
	}

	return true;
}

void BinaryReader::InitFromBinaryWriter(const BinaryWriter& aWriter)
{
	myBuffer = aWriter.myBuffer;
}

static uint32_t loadDecompressedGzipSize(const uint8_t* p)
{
	return ((uint32_t)p[0] << 0) | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) |
		((uint32_t)p[3] << 24);
}

bool BinaryReader::UncompressGzip()
{
	auto decompressor = libdeflate_alloc_decompressor();

	const size_t compressed_size = myBuffer.size();

	// Retrieve uncompressed size from the trailer of the gziped data
	const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&myBuffer.front());
	auto uncompressed_size = loadDecompressedGzipSize(&ptr[compressed_size - 4]);

	std::vector<uint8_t> bufferOut;
	bufferOut.resize(uncompressed_size);

	libdeflate_result result = libdeflate_gzip_decompress(
		decompressor,
		myBuffer.data(),
		compressed_size,
		&bufferOut[0],
		uncompressed_size,
		NULL);

	if (result != LIBDEFLATE_SUCCESS)
	{
		return false;
	}

	libdeflate_free_decompressor(decompressor);

	std::swap(myBuffer, bufferOut);

	return true;
}

const char* BinaryReader::ReadString(int* aLengthOut)
{
	const int size = Read<uint32_t>();

	if (aLengthOut)
	{
		*aLengthOut = size;
	}

	return ReadRawMemoryAsPtr<char>(size);
}

void* BinaryReader::GetStart()
{
	return myBuffer.data();
}
