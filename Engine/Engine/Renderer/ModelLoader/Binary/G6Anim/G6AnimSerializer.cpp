#include "pch.h"
#include "G6AnimSerializer.h"
#include "Engine/Renderer\Animation\AnimationClip.h"

Engine::G6AnimSerializer::G6AnimSerializer()
{
	myMainWriter.InitWithSize(1000000);
}

bool Engine::G6AnimSerializer::Serialize(G6Anim aG6Anim)
{
	// Write empty placeholder
	const auto headerPos = myMainWriter.Write<G6Anim>(G6Anim());

	// Strings
	const int stringsOffset = myMainWriter.WriteRawMemory(
		aG6Anim.myStrings.asPtr,
		aG6Anim.myStringsTotalBytesCount);

	aG6Anim.myStrings.asFileOffset = stringsOffset;

	// Bone nodes
	const int boneNodesOffset = myMainWriter.WriteRawMemory(
		aG6Anim.myBoneNodes.asPtr,
		aG6Anim.myBoneNodesCount * sizeof(G6Anim_BoneNode));

	aG6Anim.myBoneNodes.asFileOffset = boneNodesOffset;

	// BoneName to name
	const int boneNodeNameToTrackIndicesOffset = myMainWriter.WriteRawMemory(
		aG6Anim.myBoneNodeNameToTrackIndices.asPtr,
		aG6Anim.myBoneNodeNameToTrackIndicesCount * sizeof(G6Anim_BoneNodeNameToTrackIndex));

	aG6Anim.myBoneNodeNameToTrackIndices.asFileOffset = boneNodeNameToTrackIndicesOffset;


	// Translation keys
	// When all buffers are finished, we insert them into the main buffer
	const int translationKeysOffset = myMainWriter.WriteRawMemory(
		aG6Anim.myTranslationKeys.asPtr,
		aG6Anim.myTranslationKeysCount * sizeof(G6Anim_TranslationKey));

	// Update their file offset
	aG6Anim.myTranslationKeys.asFileOffset = translationKeysOffset;

	// Scale keys
	const int scaleKeysOffset = myMainWriter.WriteRawMemory(
		aG6Anim.myScaleKeys.asPtr,
		aG6Anim.myScaleKeysCount * sizeof(G6Anim_ScaleKey));

	aG6Anim.myScaleKeys.asFileOffset = scaleKeysOffset;

	// Rotation keys
	const int rotationKeysOffset = myMainWriter.WriteRawMemory(
		aG6Anim.myRotationKeys.asPtr,
		aG6Anim.myRotationKeysCount * sizeof(G6Anim_RotationKey));

	aG6Anim.myRotationKeys.asFileOffset = rotationKeysOffset;

	// Tracks keys
	const int tracksOffset = myMainWriter.WriteRawMemory(
		aG6Anim.myTracks.asPtr,
		aG6Anim.myTracksCount * sizeof(G6Anim_Track));

	aG6Anim.myTracks.asFileOffset = tracksOffset;

	myMainWriter.ReplaceAt<G6Anim>(headerPos, aG6Anim);

	return true;
}

const std::vector<uint8_t>& Engine::G6AnimSerializer::GetBuffer()
{
	return myMainWriter.GetFinalBuffer();
}


