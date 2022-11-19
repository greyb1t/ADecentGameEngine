#pragma once

namespace Engine
{
	class AnimationController;

	class AnimationStateBehaviour
	{
	public:
		virtual ~AnimationStateBehaviour() = default;

		virtual void OnAnimationStateEnter() {}

		virtual void OnAnimationStateExit() {}

		// Called during the animation being active
		virtual void OnAnimationStateUpdate() {}
	};
}