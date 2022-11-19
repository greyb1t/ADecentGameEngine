#pragma once

namespace Engine
{
	class AnimationStateMachineResource;

	class AnimationStateMachine
	{
	public:
		bool Init(const Path& aPath, AnimationStateMachineResource& aAnimationStateMachine);

		const nlohmann::json& GetJson() const { return myJson; }

	private:
		// NOTE: At the moment this is literally holding a json
		// the reason is I added this resource too late and already
		// implemented the animation system, and its too much work
		// to do it properly

		nlohmann::json myJson;

		// Node editor stuff

		// Visa flow s�h�r:
		// GraphManager::ShowFlow(link.LinkID);
		// M�ste d� ha link ID's h�r inne.
	};
}