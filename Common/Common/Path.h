#pragma once

#include <filesystem>

class Path
{
public:
	Path() = default;
	explicit Path(std::filesystem::path aPath);
	Path(const char* aStr);
	Path(const std::string& aStr);

	void MakeRelativeTo(const Path& aPath);
	void ChangeFilename(const Path& aFilename);
	void ChangeExtension(const Path& aExtension);

	Path GetParentPath() const;
	Path GetStem() const;
	Path GetFilename() const;
	Path GetExtension() const;

	// e.g. .dds
	bool IsExtension(const Path& aExtension) const;

	std::wstring ToWString() const;
	std::string ToString() const;

	Path AppendPath(Path aPath) const;

	const std::filesystem::path& GetRawPath() const;

private:
	friend std::ostream& operator<< (std::ostream& aOut, const Path& aPath);
	friend bool operator==(const Path& aLeft, const Path& aRight);

	std::filesystem::path myPath;
};

std::ostream& operator<< (std::ostream& aOut, const Path& aPath);

bool operator==(const Path& aLeft, const Path& aRight);