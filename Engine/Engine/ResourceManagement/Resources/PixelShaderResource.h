#pragma once

#include "../ResourceManager.h"
#include "../ResourceId.h"
#include "../ResourceBase.h"

namespace Engine
{
	class PixelShader;
}

namespace Engine
{
	class PixelShaderResource : public ResourceBase
	{
	public:
		static inline ResourceType ourResourceType = ResourceType::PixelShader;

	public:
		PixelShaderResource(
			ResourceManager& aResourceManager,
			const std::string& aPath,
			const ResourceId& aId);

		~PixelShaderResource();

		PixelShaderResource(const PixelShaderResource& aOther);

		PixelShader& Get() const;

	protected:
		friend class ResourceManager;

		bool Create(ResourceThreadData& aThreadData) override;
		void Swap(ResourceBase* aResourceBase);
		void PostCreation(ResourceThreadData& aResourceThreadData) override;
		void RequestMe() const;
		void LoadMe() const;
		void UnloadMe();

	protected:
		Owned<PixelShader> myPixelShader = nullptr;

		std::vector<std::string> myIncludedFilePaths;
	};
}