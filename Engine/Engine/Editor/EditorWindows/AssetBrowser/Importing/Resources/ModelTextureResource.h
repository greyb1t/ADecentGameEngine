#pragma once

#include "Resource.h"

namespace Engine
{
	enum class ModelTextureType
	{
		Albedo,
		Normal,
		Material,

		Count,
		Unknown
	};

	std::string ModelTextureTypeToString(const ModelTextureType aModelTextureType);

	class ModelTextureResource : public Resource
	{
	public:
		ModelTextureResource(const std::filesystem::path& aPath, const ModelTextureType aType);

		bool Draw(ModelImportState& aModelCreationState) override;

	private:
		bool DrawOneModel(ModelImportState& aModelCreationState);
		bool DrawMultipleMeshes(ModelImportState& aModelCreationState);

	private:
		ModelTextureType myType = ModelTextureType::Unknown;
	};
}