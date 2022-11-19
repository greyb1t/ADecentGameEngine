#pragma once

namespace Engine
{
	struct OurNode
	{
		std::string myName;

		// Cached value to improve performance to avoid lookup in map
		int myBoneNameToIndex = -1;
		int myBoneNameToTrackIndex = -1;

		// OLD, DO NOT USE, INSTEAD USE THE RAW SCALE ROTATION AND TRANSLATION
		// CU::Matrix4f myTransformation;

		// This replaces the transformation
		Vec3f myScale;
		Quatf myRotation;
		Vec3f myTranslation;

		// std::vector<OurNode> myChildren;

		int myParentIndex = 0;
	};
}