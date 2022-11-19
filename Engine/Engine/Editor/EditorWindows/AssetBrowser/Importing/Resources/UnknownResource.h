#pragma once

#include "Resource.h"

namespace Engine
{
	class UnknownResource : public Resource
	{
	public:
		UnknownResource(const std::filesystem::path& aPath);

		bool Draw(ModelImportState& aModelCreationState) override;

	private:

	};
}