#pragma once

#include <string>
#include <vector>

namespace StringUtilities
{
	std::string ToLower(std::string aString);

	bool EndsWith(const std::string& aFullString, const std::string& aEnding);

	bool Replace(std::string& aStr, const std::string& aFrom, const std::string& aTo);

	std::string FloatToString(float aNumber, int aDecimalCount);

	// Splits a string by a delimiter.
	// Example: Split("this is good", " ") becomes { "this", "is", "good" }
	std::vector<std::string> Split(const std::string& aStr, const std::string& aDelimiter);
}
