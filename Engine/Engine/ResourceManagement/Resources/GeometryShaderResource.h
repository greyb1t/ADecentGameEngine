#pragma once

#include "../ResourceManager.h"
#include "../ResourceId.h"
#include "../ResourceBase.h"

namespace Engine
{
	class GeometryShader;
}

namespace Engine
{
	class GeometryShaderResource : public ResourceBase
	{
	public:
		static inline ResourceType ourResourceType = ResourceType::GeometryShader;

	public:
		GeometryShaderResource(
			ResourceManager& aResourceManager,
			const std::string& aPath,
			const ResourceId& aId);

		~GeometryShaderResource();

		GeometryShaderResource(const GeometryShaderResource& aOther);

		GeometryShader& Get() const;

	protected:
		friend class ResourceManager;

		bool Create(ResourceThreadData& aThreadData) override;
		void Swap(ResourceBase* aResourceBase);
		void PostCreation(ResourceThreadData& aResourceThreadData) override;
		void RequestMe() const;
		void LoadMe() const;
		void UnloadMe();

	private:
		Owned<GeometryShader> myGeometryShader = nullptr;

		std::vector<std::string> myIncludedFilePaths;
	};
}