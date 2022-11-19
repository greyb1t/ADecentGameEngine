#pragma once

namespace Engine
{
	class ModelImportState;

	class Resource
	{
	public:
		Resource(const std::filesystem::path& aPath);
		virtual ~Resource() = default;

		virtual bool Draw(ModelImportState& aModelCreationState) = 0;

		const std::filesystem::path& GetPath() const;

	private:
		std::filesystem::path myPath;
	};
}