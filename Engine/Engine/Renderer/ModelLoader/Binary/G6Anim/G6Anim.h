#pragma once

namespace Engine
{
	// WHEN WE WANT TO SERIALIZE, WE HAVE TO CONVERT INTO THIS FORMAT AND GIVE TO G6AnimSerializer
	// NOTE: there are pointers in here, they must be valid so allocate yourself and make sure they're valid while
	// serializing

	struct G6Anim_BoneNode
	{
		int myNameFileOffset = 0;

		Vec3f myScale;
		Quatf myRotation;
		Vec3f myTranslation;

		int myParentIndex = 0;

		int myBoneToNameIndex = -1;
		int myBoneToTrackIndex = -1;
	};

	struct G6Anim_BoneNodeNameToTrackIndex
	{
		int myNameFileOffset = 0;
		int myIndex = 0;
	};

	struct G6Anim_String
	{
		uint32_t mySize = 0;
		char myString[1]; // the length is dependant on mySize
	};

	struct G6Anim_TranslationKey
	{
		Vec3f myPosition;
		float myTimeTicks = 0;
	};

	struct G6Anim_ScaleKey
	{
		Vec3f myScale;
		float myTimeTicks = 0;
	};

	struct G6Anim_RotationKey
	{
		Quatf myRotation;
		float myTimeTicks = 0;
	};

	struct G6Anim_Track
	{
		// The index into the string array
		int myNameIndex = 0;

		// Index into the translation keys array
		int myTranslationKeysIndex = 0;
		int myTranslationKeysCount = 0;

		int myScaleKeysIndex = 0;
		int myScaleKeysCount = 0;

		int myRotationKeysIndex = 0;
		int myRotationKeysCount = 0;
	};

	struct G6Anim
	{
		uint8_t myMagic[6] = { 'G', '6', 'A', 'N', 'I', 'M' };

		uint8_t version = 1;

		float myTicksPerSecond = 0.f;
		float myDurationInTicks = 0.f;

		// The strings are a unique case because they can vary in size
		// the asPtr value expects a string to look like this in memory
		// Size+String+Size+String+Size+String
		// In other words, size of string first, then the string, then right 
		// after null terminator, size and another string
		union
		{
			// File offset is the value storted in the binary file
			uintptr_t asFileOffset = 0;

			// Used when going to serialize or in runtime
			G6Anim_String* asPtr;
		} myStrings;

		int myStringsTotalBytesCount = 0;
		int myStringsCount = 0;

		union
		{
			uintptr_t asFileOffset = 0;

			G6Anim_BoneNode* asPtr;
		} myBoneNodes;

		int myBoneNodesCount = 0;

		union
		{
			uintptr_t asFileOffset = 0;

			G6Anim_BoneNodeNameToTrackIndex* asPtr;
		} myBoneNodeNameToTrackIndices;

		int myBoneNodeNameToTrackIndicesCount = 0;

		union
		{
			// File offset is the value storted in the binary file
			uintptr_t asFileOffset = 0;

			// Used when going to serialize or in runtime
			G6Anim_TranslationKey* asPtr;
		} myTranslationKeys;

		int myTranslationKeysCount = 0;

		union
		{
			uintptr_t asFileOffset = 0;

			G6Anim_ScaleKey* asPtr;
		} myScaleKeys;

		int myScaleKeysCount = 0;

		union
		{
			uintptr_t asFileOffset = 0;

			G6Anim_RotationKey* asPtr;
		} myRotationKeys;

		int myRotationKeysCount = 0;

		union
		{
			uintptr_t asFileOffset = 0;

			G6Anim_Track* asPtr;
		} myTracks;

		int myTracksCount = 0;
	};
}