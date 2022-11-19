#pragma once

#include "G6AnimSerializer.h"

namespace Engine
{
	class AnimationClip;
	class Model;
}

namespace Engine
{
	class G6AnimSerializerAnimationClip : public G6AnimSerializer
	{
	public:
		bool Serialize(const AnimationClip& aAnimationClip, const Model& aAttachedFbx);

	private:
		void WriteStrings(const AnimationClip& aAnimationClip);
		void WriteTracks(const AnimationClip& aAnimationClip);
		void WriteBoneNodes(const AnimationClip& aAnimationClip, const Model& aAttachedFbx);
		void WriteBoneNodeNameToTrackIndex(const AnimationClip& aAnimationClip);

	private:
		G6Anim myG6Anim;

		// THESE ARE ACTUALLY NOT SUPPOSED TO BE HERE, THEY ARE ONLY FOR WHEN CONVERTING FROM AnimationClip
		// to the G6Anim, so the proper thing would be to make a separate class for just that
		// Must be here because it holds the buffer for the strings
		BinaryWriter myStringsWriter;
		std::unordered_map<std::string, int> myStringToStringIndex;

		std::vector<G6Anim_Track> tracks;
		std::vector<G6Anim_TranslationKey> allTranslationKeys;
		std::vector<G6Anim_ScaleKey> allScaleKeys;
		std::vector<G6Anim_RotationKey> allRotationKeys;

		std::vector<G6Anim_BoneNode> myBoneNodes;
		std::vector<G6Anim_BoneNodeNameToTrackIndex> myBoneNodeNameToTrackIndices;
	};
}