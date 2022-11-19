#include "StringUtilities.h"

#include <sstream>
#include <iomanip>

std::string StringUtilities::ToLower(std::string aString)
{
	for (auto& c : aString)
	{
		c = tolower(c);
	}

	return aString;
}

bool StringUtilities::EndsWith(const std::string& aFullString, const std::string& aEnding)
{
	if (aFullString.length() >= aEnding.length())
	{
		return (0 == aFullString.compare(aFullString.length() - aEnding.length(), aEnding.length(), aEnding));
	}
	else
	{
		return false;
	}
}

bool StringUtilities::Replace(std::string& aStr, const std::string& aFrom, const std::string& aTo)
{
	size_t startPos = aStr.find(aFrom);

	if (startPos == std::string::npos)
	{
		return false;
	}

	aStr.replace(startPos, aFrom.length(), aTo);

	return true;
}

std::string StringUtilities::FloatToString(float aNumber, int aDecimalCount)
{
	std::stringstream ss;
	ss << std::fixed << std::setprecision(aDecimalCount) << aNumber;
	return ss.str();
}

std::vector<std::string> StringUtilities::Split(const std::string& aStr, const std::string& aDelimiter)
{
	std::vector<std::string> result;

	size_t start = 0;
	size_t end = 0;
	const auto delimiterLength = aDelimiter.size();
	std::string token;

	while ((end = aStr.find(aDelimiter, start)) != std::string::npos)
	{
		token = aStr.substr(start, end - start);
		start = end + delimiterLength;
		result.push_back(token);
	}

	result.push_back(aStr.substr(start));

	return result;
}
