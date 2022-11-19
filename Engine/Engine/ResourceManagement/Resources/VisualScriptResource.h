#pragma once

#include "../ResourceManager.h"
#include "../ResourceId.h"
#include "../ResourceBase.h"

namespace Engine
{
	class VisualScript;

	class VisualScriptResource : public ResourceBase
	{
	public:
		static inline ResourceType ourResourceType = ResourceType::VisualScript;

	public:
		VisualScriptResource(
			ResourceManager& aResourceManager,
			const std::string& aPath,
			const ResourceId& aId);

		virtual ~VisualScriptResource();

		VisualScriptResource(const VisualScriptResource& aOther);

		VisualScript& Get() const;

	protected:
		friend class ResourceManager;

		bool Create(ResourceThreadData& aThreadData) override;
		void Swap(ResourceBase* aResourceBase);
		void PostCreation(ResourceThreadData& aResourceThreadData) override;
		void RequestMe() const;
		void LoadMe() const;
		void UnloadMe();

	private:
		Owned<VisualScript> myVisualGraph = nullptr;
	};
}