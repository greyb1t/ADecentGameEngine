#pragma once

#include "../ResourceManager.h"
#include "../ResourceId.h"
#include "../ResourceBase.h"

namespace Engine
{
	class AnimationStateMachine;

	class AnimationStateMachineResource : public ResourceBase
	{
	public:
		static inline ResourceType ourResourceType = ResourceType::AnimationStateMachine;

	public:
		AnimationStateMachineResource(
			ResourceManager& aResourceManager,
			const std::string& aPath,
			const ResourceId& aId);

		virtual ~AnimationStateMachineResource();

		AnimationStateMachineResource(const AnimationStateMachineResource& aOther);

		AnimationStateMachine& Get() const;

	protected:
		friend class ResourceManager;

		bool Create(ResourceThreadData& aThreadData) override;
		void Swap(ResourceBase* aResourceBase);
		void PostCreation(ResourceThreadData& aResourceThreadData) override;
		void RequestMe() const;
		void LoadMe() const;
		void UnloadMe();

	private:
		Owned<AnimationStateMachine> myAnimation = nullptr;
	};
}