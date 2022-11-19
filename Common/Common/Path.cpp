#include "Path.h"

#include <cwctype>

Path::Path(std::filesystem::path aPath)
	: myPath(aPath.make_preferred())
{
}

Path::Path(const char* aStr)
	: myPath(std::filesystem::path(aStr).make_preferred())
{
}

Path::Path(const std::string& aStr)
	: myPath(std::filesystem::path(aStr).make_preferred())
{
}

void Path::MakeRelativeTo(const Path& aPath)
{
	const auto relativePath = std::filesystem::relative(
		myPath,
		aPath.myPath);

	myPath = relativePath;
}

void Path::ChangeFilename(const Path& aFilename)
{
	myPath.replace_filename(aFilename.myPath);
}

void Path::ChangeExtension(const Path& aExtension)
{
	myPath.replace_extension(aExtension.myPath);
}

Path Path::GetParentPath() const
{
	return Path(myPath.parent_path());
}

Path Path::GetStem() const
{
	return Path(myPath.stem());
}

Path Path::GetFilename() const
{
	return Path(myPath.filename());
}

Path Path::GetExtension() const
{
	return Path(myPath.extension());
}

bool Path::IsExtension(const Path& aExtension) const
{
	std::wstring thisExtension = GetExtension().ToWString();
	std::wstring otherExtension = aExtension.ToWString();

	// Early return
	if (thisExtension.size() != otherExtension.size())
	{
		return false;
	}

	for (auto& c : thisExtension)
	{
		c = std::towlower(c);
	}

	for (auto& c : otherExtension)
	{
		c = std::towlower(c);
	}

	return thisExtension == otherExtension;
}

std::wstring Path::ToWString() const
{
	return myPath.wstring();
}

std::string Path::ToString() const
{
	return myPath.string();
}

Path Path::AppendPath(Path aPath) const
{
	return Path(myPath / aPath.myPath.make_preferred());
}

const std::filesystem::path& Path::GetRawPath() const
{
	return myPath;
}

std::ostream& operator<<(std::ostream& aOut, const Path& aPath)
{
	aOut << aPath.myPath;
	return aOut;
}

bool operator==(const Path& aLeft, const Path& aRight)
{
	return aLeft.myPath == aRight.myPath;
}
