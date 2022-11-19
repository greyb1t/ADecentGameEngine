#pragma once

#include "..\Playable.h"

namespace Engine
{
	class BlendNode1D
	{
	public:
		BlendNode1D() = default;

		// aTimePosition can be -1 to 1
		BlendNode1D(const float aTimePosition, Owned<Playable> aPlayable);

		nlohmann::json ToJson() const;
		void InitFromJson(const nlohmann::json& aJson, AnimationMachine& aMachine);

		float GetThreshold() const;

		Playable* GetPlayable();
		const Playable* GetPlayable() const;

	private:
		friend class AnimationNodeEditorWindow;

		// When the BlendTree is set to this time, means 
		// this animation is the primary animation being played
		float myThreshold = 0.f;

		Owned<Playable> myPlayable;
	};
}