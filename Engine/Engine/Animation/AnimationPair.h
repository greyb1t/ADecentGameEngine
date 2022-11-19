#pragma once

#include "Engine/ResourceManagement/ResourceRef.h"

namespace Engine
{
	class AnimationMachine;

	// Purpose is to send this to the reflection system and it chooses
	// what to edit based on if the instance is valid
	struct AnimationPair
	{
		// MUST HAVE A REFERENCE LIKE THIS, OTHERWISE THE REFLECT function cannot edit the ResourceRef
		AnimationPair(AnimationMachineRef& aAnimResource)
			: myAnimationResource(aAnimResource)
		{
		}

		bool operator==(const AnimationPair& aOther) const
		{
			return myAnimationResource == aOther.myAnimationResource;
		}

		bool operator!=(const AnimationPair& aOther) const
		{
			return !(*this == aOther);
		}

		AnimationPair& operator=(const AnimationPair& aOther)
		{
			myAnimationResource = aOther.myAnimationResource;
			return *this;
		}

		AnimationMachineRef& myAnimationResource;
		AnimationMachine* myMachineInstance = nullptr;
	};
}