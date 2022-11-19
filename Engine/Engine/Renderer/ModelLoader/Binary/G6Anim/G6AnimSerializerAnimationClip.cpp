#include "pch.h"
#include "G6AnimSerializerAnimationClip.h"
#include "Engine/Renderer\Animation\AnimationClip.h"
#include "Engine/Renderer/Model/Model.h"

bool Engine::G6AnimSerializerAnimationClip::Serialize(
	const Engine::AnimationClip& aAnimationClip,
	const Engine::Model& aAttachedFbx)
{
	// Convert AnimationClip to G6Anim

	myG6Anim.myDurationInTicks = aAnimationClip.GetDurationInTicks();
	myG6Anim.myTicksPerSecond = aAnimationClip.GetTicksPerSecond();

	WriteStrings(aAnimationClip);

	WriteBoneNodes(aAnimationClip, aAttachedFbx);

	WriteBoneNodeNameToTrackIndex(aAnimationClip);

	WriteTracks(aAnimationClip);

	return G6AnimSerializer::Serialize(myG6Anim);
}

void Engine::G6AnimSerializerAnimationClip::WriteStrings(const Engine::AnimationClip& aAnimationClip)
{
	const int beginSize = myStringsWriter.GetSize();

	// NOTE: Assumes aAnimationClip bonenodes and toTrackIndex have same strings
	for (int i = 0; i < aAnimationClip.GetBoneNodes().size(); ++i)
	{
		const auto& s = aAnimationClip.GetBoneNodes()[i].myName;
		myStringToStringIndex[s] = i;
		myStringsWriter.WriteString(s);
	}

	myG6Anim.myStrings.asPtr = reinterpret_cast<G6Anim_String*>(myStringsWriter.GetFinalBuffer().data());
	myG6Anim.myStringsTotalBytesCount = myStringsWriter.GetSize() - beginSize;

	myG6Anim.myStringsCount = aAnimationClip.GetBoneNodes().size();
}

void Engine::G6AnimSerializerAnimationClip::WriteTracks(const Engine::AnimationClip& aAnimationClip)
{
	for (const auto& track : aAnimationClip.GetTracks())
	{
		G6Anim_Track g6Track;

		g6Track.myNameIndex = myStringToStringIndex.at(track.myName);

		// Translation keys
		{
			g6Track.myTranslationKeysIndex = allTranslationKeys.size();
			g6Track.myTranslationKeysCount = track.myTranslationKeys.size();

			for (const auto& translationKey : track.myTranslationKeys)
			{
				G6Anim_TranslationKey g6TranslatioKey;
				g6TranslatioKey.myPosition = translationKey.myPosition;
				g6TranslatioKey.myTimeTicks = translationKey.myTimeTicks;
				allTranslationKeys.push_back(g6TranslatioKey);
			}
		}

		// Scale keys
		{
			g6Track.myScaleKeysIndex = allScaleKeys.size();
			g6Track.myScaleKeysCount = track.myScaleKeys.size();

			for (const auto& scaleKey : track.myScaleKeys)
			{
				G6Anim_ScaleKey g6ScaleKey;
				g6ScaleKey.myScale = scaleKey.myScale;
				g6ScaleKey.myTimeTicks = scaleKey.myTimeTicks;
				allScaleKeys.push_back(g6ScaleKey);
			}
		}

		// Rotation keys
		{
			g6Track.myRotationKeysIndex = allRotationKeys.size();
			g6Track.myRotationKeysCount = track.myRotationKeys.size();

			for (const auto& rotationKey : track.myRotationKeys)
			{
				G6Anim_RotationKey g6RotationKey;
				g6RotationKey.myRotation = rotationKey.myRotation;
				g6RotationKey.myTimeTicks = rotationKey.myTimeTicks;
				allRotationKeys.push_back(g6RotationKey);
			}
		}

		tracks.push_back(g6Track);
	}

	myG6Anim.myTracks.asPtr = tracks.data();
	myG6Anim.myTracksCount = tracks.size();

	// its a pointer now, but in Serialize(G6Anim) we turn it into a 
	// file offset after reading the values in the pointer
	myG6Anim.myTranslationKeysCount = allTranslationKeys.size();
	myG6Anim.myTranslationKeys.asPtr = allTranslationKeys.data();

	myG6Anim.myScaleKeysCount = allScaleKeys.size();
	myG6Anim.myScaleKeys.asPtr = allScaleKeys.data();

	myG6Anim.myRotationKeysCount = allRotationKeys.size();
	myG6Anim.myRotationKeys.asPtr = allRotationKeys.data();
}

void Engine::G6AnimSerializerAnimationClip::WriteBoneNodes(
	const Engine::AnimationClip& aAnimationClip,
	const Engine::Model& aAttachedFbx)
{
	const auto& boneNameToIndex = aAttachedFbx.GetBoneNameToIndex();
	const auto& boneNameToTrackIndex = aAnimationClip.GetToBoneTrackIndex();

	for (const auto& boneNode : aAnimationClip.GetBoneNodes())
	{
		G6Anim_BoneNode loaderBoneNode;
		{
			loaderBoneNode.myNameFileOffset = myStringToStringIndex.at(boneNode.myName);
			loaderBoneNode.myParentIndex = boneNode.myParentIndex;
			loaderBoneNode.myRotation = boneNode.myRotation;
			loaderBoneNode.myScale = boneNode.myScale;
			loaderBoneNode.myTranslation = boneNode.myTranslation;

			if (boneNameToIndex.find(boneNode.myName) != boneNameToIndex.end())
			{
				loaderBoneNode.myBoneToNameIndex = boneNameToIndex.at(boneNode.myName);
			}
			else
			{
				loaderBoneNode.myBoneToNameIndex = -1;
			}

			if (boneNameToTrackIndex.find(boneNode.myName) != boneNameToTrackIndex.end())
			{
				loaderBoneNode.myBoneToTrackIndex = boneNameToTrackIndex.at(boneNode.myName);
			}
			else
			{
				loaderBoneNode.myBoneToTrackIndex = -1;
			}
		}

		myBoneNodes.push_back(loaderBoneNode);
	}

	myG6Anim.myBoneNodes.asPtr = myBoneNodes.data();
	myG6Anim.myBoneNodesCount = myBoneNodes.size();
}

void Engine::G6AnimSerializerAnimationClip::WriteBoneNodeNameToTrackIndex(const Engine::AnimationClip& aAnimationClip)
{
	for (const auto& [name, index] : aAnimationClip.GetToBoneTrackIndex())
	{
		G6Anim_BoneNodeNameToTrackIndex loaderNameToTrackIndex;
		{
			loaderNameToTrackIndex.myNameFileOffset = myStringToStringIndex.at(name);
			loaderNameToTrackIndex.myIndex = index;
		}

		myBoneNodeNameToTrackIndices.push_back(loaderNameToTrackIndex);
	}

	myG6Anim.myBoneNodeNameToTrackIndices.asPtr = myBoneNodeNameToTrackIndices.data();
	myG6Anim.myBoneNodeNameToTrackIndicesCount = myBoneNodeNameToTrackIndices.size();
}
