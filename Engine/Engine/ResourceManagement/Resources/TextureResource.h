#pragma once

#include "../ResourceManager.h"
#include "../ResourceId.h"
#include "../ResourceBase.h"

namespace Engine
{
	class Texture2D;
}

namespace Engine
{
	class TextureResource : public ResourceBase
	{
	public:
		static inline ResourceType ourResourceType = ResourceType::Texture;

	public:
		TextureResource(
			ResourceManager& aResourceManager,
			const std::string& aPath,
			const ResourceId& aId);

		TextureResource(const TextureResource& aOther);

		TextureResource(Shared<Texture2D> aTexture);

		virtual ~TextureResource();

		Texture2D& Get() const;
		Texture2D& Get() { return *myTexture; }

	protected:
		friend class ResourceManager;

		bool Create(ResourceThreadData& aThreadData) override;
		void Swap(ResourceBase* aResourceBase);
		void PostCreation(ResourceThreadData& aResourceThreadData) override;
		void RequestMe() const override;
		void LoadMe() const;
		void UnloadMe();

	private:
		Shared<Texture2D> myTexture = nullptr;
	};
}