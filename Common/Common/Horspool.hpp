#pragma once

#include <string>
#include <array>

namespace Common
{
	int Horspool(const std::string& aNeedle, const std::string& aHaystack);

	std::array<int, 256> BuildTable(const std::string& aNeedle);
}

int Common::Horspool(const std::string& aNeedle, const std::string& aHaystack)
{
	const auto table = BuildTable(aNeedle);

	int index = aNeedle.size() - 1;

	while (index < aHaystack.size())
	{
		const int asciiValue = aHaystack[index];

		bool exists = table[asciiValue] != aNeedle.size();

		if (exists == false)
		{
			index += aNeedle.size();
		}
		else
		{
			// Character exists in haystack

			const int offset = table[asciiValue];
			index += offset;


			bool wordFound = true;
			for (int i = 0; i < aNeedle.size(); i++)
			{
				const int haystackIndex = index + i - (aNeedle.size() - 1);
				if (aNeedle[i] != aHaystack[haystackIndex])
				{
					wordFound = false;
					break;
				}
			}

			if (wordFound)
			{
				return index - (aNeedle.size() - 1);
			}
			else
			{
				index += (aNeedle.size() - 1) - offset;
			}
		}
	}

	return -1;
}

std::array<int, 256> Common::BuildTable(const std::string& aNeedle)
{
	std::array<int, 256> result;
	result.fill(aNeedle.size());

	for (int i = 0; i < static_cast<int>(aNeedle.size()); i++)
	{
		const int asciiValue = aNeedle[i];
		result[asciiValue] = aNeedle.size() - 1 - i;
	}

	return result;
}

namespace CU = Common;