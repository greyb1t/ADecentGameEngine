#pragma once

#include "G6Anim.h"

namespace Engine
{
	class G6AnimDeserializer
	{
	public:
		bool Deserialize(const Path& aPath);

		const G6Anim& GetHeader() const;

		const char* GetString(const int aIndex) const;

	private:
		BinaryReader myBinaryReader;

		G6Anim* myHeader = nullptr;

		std::vector<const char*> myStrings;
	};
}