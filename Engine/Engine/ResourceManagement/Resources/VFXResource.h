#pragma once

#include "../ResourceManager.h"
#include "../ResourceId.h"
#include "../ResourceBase.h"

namespace VFX
{
	struct VFXDescription;
}

namespace Engine
{
	class VFXResource : public ResourceBase
	{
	public:
		static inline ResourceType ourResourceType = ResourceType::VFX;

	public:
		VFXResource(
			ResourceManager& aResourceManager,
			const std::string& aPath,
			const ResourceId& aId);

		virtual ~VFXResource();

		VFXResource(const VFXResource& aOther);

		VFX::VFXDescription& Get() const;

	protected:
		friend class ResourceManager;

		bool Create(ResourceThreadData& aThreadData) override;
		void Swap(ResourceBase* aResourceBase);
		void PostCreation(ResourceThreadData& aResourceThreadData) override;
		void RequestMe() const;
		void LoadMe() const;
		void UnloadMe();

	private:
		Owned<VFX::VFXDescription> myVFXDesc = {};
	};
}