#pragma once

#include "../ResourceManager.h"
#include "../ResourceId.h"
#include "../ResourceBase.h"

namespace Engine
{
	class Curve;

	class AnimationCurveResource : public ResourceBase
	{
	public:
		static inline ResourceType ourResourceType = ResourceType::AnimationCurve;

	public:
		AnimationCurveResource(
			ResourceManager& aResourceManager,
			const std::string& aPath,
			const ResourceId& aId);

		virtual ~AnimationCurveResource();

		AnimationCurveResource(const AnimationCurveResource& aOther);

		Curve& Get() const;

	protected:
		friend class ResourceManager;

		bool Create(ResourceThreadData& aThreadData) override;
		void Swap(ResourceBase* aResourceBase);
		void PostCreation(ResourceThreadData& aResourceThreadData) override;
		void RequestMe() const;
		void LoadMe() const;
		void UnloadMe();

	private:
		Owned<Curve> myCurve = nullptr;
	};
}