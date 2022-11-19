#pragma once

#include "../ResourceManager.h"
#include "../ResourceId.h"
#include "../ResourceBase.h"

namespace Engine
{
	class Model;
}

namespace Engine
{
	class ModelResource : public ResourceBase
	{
	public:
		static inline ResourceType ourResourceType = ResourceType::Model;

	public:
		ModelResource(
			ResourceManager& aResourceManager,
			const std::string& aPath,
			const ResourceId& aId);

		virtual ~ModelResource();

		ModelResource(const ModelResource& aOther);

		Model& Get() const;
		Model& Get() { return *myModel; }

	protected:
		friend class ResourceManager;

		bool Create(ResourceThreadData& aThreadData) override;
		void Swap(ResourceBase* aResourceBase);
		void PostCreation(ResourceThreadData& aResourceThreadData) override;
		void RequestMe() const;
		void LoadMe() const;
		void UnloadMe();

	private:
		Owned<Model> myModel = nullptr;
	};
}