#include "pch.h"
#include "G6AnimDeserializer.h"

bool Engine::G6AnimDeserializer::Deserialize(const Path& aPath)
{
	if (!myBinaryReader.InitWithFile(aPath))
	{
		return false;
	}

	myHeader = myBinaryReader.ReadAsPtr<G6Anim>();

	if (myHeader->myMagic[0] != 'G' ||
		myHeader->myMagic[1] != '6' ||
		myHeader->myMagic[2] != 'A' ||
		myHeader->myMagic[3] != 'N' ||
		myHeader->myMagic[4] != 'I' ||
		myHeader->myMagic[5] != 'M')
	{
		LOG_ERROR(LogType::Resource) << aPath << " is not a g6 anim file";

		return false;
	}

	myStrings.reserve(myHeader->myStringsCount);

	for (int i = 0; i < myHeader->myStringsCount; ++i)
	{
		const auto stringSize = myBinaryReader.Read<uint32_t>();
		const char* s = myBinaryReader.ReadRawMemoryAsPtr<const char>(stringSize);

		myStrings.push_back(s);
	}

	// Basically convert all file offset to actual pointers, basically a pointer fixup
	myHeader->myBoneNodes.asPtr =
		reinterpret_cast<G6Anim_BoneNode*>(
			reinterpret_cast<uintptr_t>(myBinaryReader.GetStart()) + myHeader->myBoneNodes.asFileOffset);

	myHeader->myBoneNodeNameToTrackIndices.asPtr =
		reinterpret_cast<G6Anim_BoneNodeNameToTrackIndex*>(
			reinterpret_cast<uintptr_t>(myBinaryReader.GetStart()) + myHeader->myBoneNodeNameToTrackIndices.asFileOffset);

	myHeader->myTranslationKeys.asPtr =
		reinterpret_cast<G6Anim_TranslationKey*>(
			reinterpret_cast<uintptr_t>(myBinaryReader.GetStart()) + myHeader->myTranslationKeys.asFileOffset);

	myHeader->myScaleKeys.asPtr =
		reinterpret_cast<G6Anim_ScaleKey*>(
			reinterpret_cast<uintptr_t>(myBinaryReader.GetStart()) + myHeader->myScaleKeys.asFileOffset);

	myHeader->myRotationKeys.asPtr =
		reinterpret_cast<G6Anim_RotationKey*>(
			reinterpret_cast<uintptr_t>(myBinaryReader.GetStart()) + myHeader->myRotationKeys.asFileOffset);

	myHeader->myTracks.asPtr =
		reinterpret_cast<G6Anim_Track*>(
			reinterpret_cast<uintptr_t>(myBinaryReader.GetStart()) + myHeader->myTracks.asFileOffset);

	return true;
}

const Engine::G6Anim& Engine::G6AnimDeserializer::GetHeader() const
{
	return *myHeader;
}

const char* Engine::G6AnimDeserializer::GetString(const int aIndex) const
{
	return myStrings[aIndex];
}
