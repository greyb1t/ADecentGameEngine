#pragma once

#include "Component.h"

namespace Engine
{
	class AnimatorComponent;

	class FollowAnimatedBoneComponent : public Component
	{
	public:
		FollowAnimatedBoneComponent() = default;
		FollowAnimatedBoneComponent(GameObject* aGameObject);

		void Start() override;

	private:
		AnimatorComponent* myAnimatorComponent = nullptr;
	};
}
