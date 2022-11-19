#include "JsonFile.h"

JsonFile::~JsonFile()
{
	if (myBuffer)
	{
		delete[] myBuffer;
		myBuffer = nullptr;
	}
}
