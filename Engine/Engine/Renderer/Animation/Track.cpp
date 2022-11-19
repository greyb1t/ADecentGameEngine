#include "pch.h"
#include "Track.h"
#include "AssimpUtils.h"

namespace Engine
{
	unsigned int Track::FindScaleKeyIndexBefore(float aAnimationTimeTicks) const
	{
		assert(myScaleKeys.size() > 0);

		for (unsigned int i = 0; i < myScaleKeys.size() - 1; i++)
		{
			if (aAnimationTimeTicks <= myScaleKeys[i + 1].myTimeTicks)
			{
				return i;
			}
		}

		assert(false);

		return static_cast<unsigned int>(-1);
	}

	Vec3f Track::CalcInterpolatedScaling(float aAnimationTimeTicks) const
	{
		if (myScaleKeys.size() == 1)
		{
			return myScaleKeys[0].myScale;
		}

		unsigned int scalingIndex = FindScaleKeyIndexBefore(aAnimationTimeTicks);
		unsigned int nextScalingIndex = (scalingIndex + 1);

		assert(nextScalingIndex < myScaleKeys.size());

		const float deltaTimeTicks = myScaleKeys[nextScalingIndex].myTimeTicks - myScaleKeys[scalingIndex].myTimeTicks;

		const float percentBetweenKeys = (aAnimationTimeTicks - myScaleKeys[scalingIndex].myTimeTicks) / deltaTimeTicks;

		assert(percentBetweenKeys >= 0.0f && percentBetweenKeys <= 1.0f);

		const Vec3f leftKeyScale = myScaleKeys[scalingIndex].myScale;

		const Vec3f rightKeyScale = myScaleKeys[nextScalingIndex].myScale;

		// DO THIS LATER, INVERTED?
		// return leftKeyScale * percentBetweenKeys + rightKeyScale * (1.f - percentBetweenKeys);

		return leftKeyScale * (1.f - percentBetweenKeys) + rightKeyScale * percentBetweenKeys;
	}

	unsigned int Track::FindRotationKeyIndexBefore(float aAnimationTimeTicks) const
	{
		assert(myRotationKeys.size() > 0);

		for (unsigned int i = 0; i < myRotationKeys.size() - 1; i++)
		{
			if (aAnimationTimeTicks <= myRotationKeys[i + 1].myTimeTicks)
			{
				return i;
			}
		}

		assert(false);

		return static_cast<unsigned int>(-1);
	}

	Quatf Track::CalcInterpolatedRotation(float aAnimationTimeTicks) const
	{
		if (myRotationKeys.size() == 1)
		{
			return myRotationKeys[0].myRotation;
		}

		unsigned int rotationIndex = FindRotationKeyIndexBefore(aAnimationTimeTicks);

		unsigned int nextRotationIndex = (rotationIndex + 1);

		assert(nextRotationIndex < myRotationKeys.size());

		float deltaTimeTicks = static_cast<float>(myRotationKeys[nextRotationIndex].myTimeTicks - myRotationKeys[rotationIndex].myTimeTicks);

		float percentBetweenKeys = (aAnimationTimeTicks - (float)myRotationKeys[rotationIndex].myTimeTicks) / deltaTimeTicks;

		assert(percentBetweenKeys >= 0.0f && percentBetweenKeys <= 1.0f);

		const Quatf& leftKeyRotationQ = myRotationKeys[rotationIndex].myRotation;

		const Quatf& rightKeyRotationQ = myRotationKeys[nextRotationIndex].myRotation;

		Quatf out = Quatf::Slerp(leftKeyRotationQ, rightKeyRotationQ, percentBetweenKeys);

		out.Normalize();

		return out;
	}

	unsigned int Track::FindPositionKeyIndexBefore(float aAnimationTimeTicks) const
	{
		assert(myTranslationKeys.size() > 0);

		for (unsigned int i = 0; i < myTranslationKeys.size() - 1; i++)
		{
			if (aAnimationTimeTicks <= myTranslationKeys[i + 1].myTimeTicks)
			{
				return i;
			}
		}

		assert(false);

		return static_cast<unsigned int>(-1);
	}

	Vec3f Track::CalcInterpolatedPosition(float aAnimationTimeTicks) const
	{
		if (myTranslationKeys.size() == 1)
		{
			return myTranslationKeys[0].myPosition;
		}

		const unsigned int positionIndex = FindPositionKeyIndexBefore(aAnimationTimeTicks);

		const unsigned int nextPositionIndex = (positionIndex + 1);

		assert(nextPositionIndex < myTranslationKeys.size());

		const float deltaTimeTicks = myTranslationKeys[nextPositionIndex].myTimeTicks - myTranslationKeys[positionIndex].myTimeTicks;

		const float percentBetweenKeys = (aAnimationTimeTicks - myTranslationKeys[positionIndex].myTimeTicks) / deltaTimeTicks;

		assert(percentBetweenKeys >= 0.0f && percentBetweenKeys <= 1.0f);

		const Vec3f& leftKeyPosition = myTranslationKeys[positionIndex].myPosition;

		const Vec3f& rightKeyPosition = myTranslationKeys[nextPositionIndex].myPosition;

		return leftKeyPosition * (1.f - percentBetweenKeys) + rightKeyPosition * percentBetweenKeys;
	}
}
