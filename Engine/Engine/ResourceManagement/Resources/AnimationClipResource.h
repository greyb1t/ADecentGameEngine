#pragma once

#include "../ResourceManager.h"
#include "../ResourceId.h"
#include "../ResourceBase.h"

namespace Engine
{
	class AnimationClip;
}

namespace Engine
{
	class AnimationClipResource : public ResourceBase
	{
	public:
		static inline ResourceType ourResourceType = ResourceType::AnimationClip;

	public:
		AnimationClipResource(
			ResourceManager& aResourceManager,
			const std::string& aPath,
			const ResourceId& aId);

		virtual ~AnimationClipResource();

		AnimationClipResource(const AnimationClipResource& aOther);

		AnimationClip& Get() const;
		AnimationClip& Get() { return *myAnimationClip; }

	protected:
		friend class ResourceManager;

		bool Create(ResourceThreadData& aThreadData) override;
		void Swap(ResourceBase* aResourceBase);
		void PostCreation(ResourceThreadData& aResourceThreadData) override;
		void RequestMe() const;
		void LoadMe() const;
		void UnloadMe();

	private:
		Owned<AnimationClip> myAnimationClip = nullptr;
	};
}