#pragma once

namespace Engine
{
	struct TranslationKey
	{
		Vec3f myPosition;
		float myTimeTicks = 0;
	};

	struct ScaleKey
	{
		Vec3f myScale;
		float myTimeTicks = 0;
	};

	struct RotationKey
	{
		Quatf myRotation;
		float myTimeTicks = 0;
	};

	// Holds all the keyframes
	struct Track
	{
		std::vector<TranslationKey> myTranslationKeys;
		std::vector<ScaleKey> myScaleKeys;
		std::vector<RotationKey> myRotationKeys;
		std::string myName;

		unsigned int FindScaleKeyIndexBefore(float aAnimationTimeTicks) const;
		Vec3f CalcInterpolatedScaling(float aAnimationTimeTicks) const;

		unsigned int FindRotationKeyIndexBefore(float aAnimationTimeTicks) const;
		Quatf CalcInterpolatedRotation(float aAnimationTimeTicks) const;

		unsigned int  FindPositionKeyIndexBefore(float aAnimationTimeTicks) const;

		Vec3f CalcInterpolatedPosition(float aAnimationTimeTicks) const;
	};
}