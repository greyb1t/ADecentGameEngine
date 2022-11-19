#include "pch.h"
#include "AnimationClip.h"
#include "AssimpUtils.h"
#include "..\ModelLoader\Binary\G6Anim\G6AnimDeserializer.h"
#include "Engine\Editor\FileTypes.h"

namespace Engine
{

	AnimationClip::AnimationClip()
	{

	}

	AnimationClip::~AnimationClip()
	{

	}

	bool AnimationClip::Init(const Path& aPath)
	{
		// auto pathCopy = aPath;
		// pathCopy.ChangeExtension(".anim");
		// 
		// return InitCustomAnim(pathCopy);

		if (aPath.IsExtension(FileType::Extension_AnimationClip))
		{
			return InitCustomAnim(aPath);
		}

		LOG_ERROR(LogType::Resource) << "ANIM FBX NOT SUPPORTED ANYMORE: " << aPath;

		// return false;

		START_TIMER(timer);

		myPath = aPath.ToString();

		Assimp::Importer imp;

		unsigned int flags =
			aiProcessPreset_TargetRealtime_MaxQuality |
			aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_ConvertToLeftHanded |
			aiProcess_SortByPType;

		flags &= ~aiProcess_JoinIdenticalVertices;

		const aiScene* scene = imp.ReadFile(aPath.ToString(), flags);

		if (scene == nullptr)
		{
			LOG_ERROR(LogType::Resource) << "Failed to parse animation clip " << aPath;
			return false;
		}

		if (scene->mNumAnimations > 1)
		{
			LOG_WARNING(LogType::Animation) <<
				"Warning: Animation has multiple animations, we do not support this: " << aPath;
		}

		int animIndex = 0;

		if (scene->mNumAnimations <= 0)
		{
			LOG_ERROR(LogType::Animation) << "The FBX had no animations: " << aPath;
			return false;
		}

		const auto& anim = scene->mAnimations[animIndex];

		myTicksPerSecond = static_cast<float>(scene->mAnimations[animIndex]->mTicksPerSecond);

		if (myTicksPerSecond == 0.f)
		{
			LOG_ERROR(LogType::Animation) << "Animation has 0 ticks per second, whot?";
			return false;
		}

		myDurationInTicks = static_cast<float>(scene->mAnimations[animIndex]->mDuration);

		const float durationTicks = static_cast<float>(scene->mAnimations[animIndex]->mDuration);
		myDurationSeconds = durationTicks / myTicksPerSecond;

		for (unsigned int channelIndex = 0; channelIndex < anim->mNumChannels; ++channelIndex)
		{
			Track track;
			track.myName = anim->mChannels[channelIndex]->mNodeName.C_Str();

			// Remove the namespace part of the meshname for meshes with namespaces
			const auto colonIndex = track.myName.find(':');

			if (colonIndex != std::string::npos)
			{
				track.myName = track.myName.substr(colonIndex + 1);
			}

			for (unsigned int posKeyIndex = 0; posKeyIndex < anim->mChannels[channelIndex]->mNumPositionKeys; ++posKeyIndex)
			{
				const auto& aiPosKey = anim->mChannels[channelIndex]->mPositionKeys[posKeyIndex];

				TranslationKey transKey;
				transKey.myTimeTicks = static_cast<float>(aiPosKey.mTime);
				transKey.myPosition.x = aiPosKey.mValue.x;
				transKey.myPosition.y = aiPosKey.mValue.y;
				transKey.myPosition.z = aiPosKey.mValue.z;

				track.myTranslationKeys.push_back(transKey);
			}

			for (unsigned int scaleKeyIndex = 0; scaleKeyIndex < anim->mChannels[channelIndex]->mNumScalingKeys; ++scaleKeyIndex)
			{
				const auto& aiScaleKey = anim->mChannels[channelIndex]->mScalingKeys[scaleKeyIndex];

				ScaleKey scaleKey;
				scaleKey.myTimeTicks = static_cast<float>(aiScaleKey.mTime);
				scaleKey.myScale.x = aiScaleKey.mValue.x;
				scaleKey.myScale.y = aiScaleKey.mValue.y;
				scaleKey.myScale.z = aiScaleKey.mValue.z;

				track.myScaleKeys.push_back(scaleKey);
			}

			for (unsigned int rotKeyIndex = 0; rotKeyIndex < anim->mChannels[channelIndex]->mNumRotationKeys; ++rotKeyIndex)
			{
				const auto& aiRotKey = anim->mChannels[channelIndex]->mRotationKeys[rotKeyIndex];

				RotationKey rotKey;
				rotKey.myTimeTicks = static_cast<float>(aiRotKey.mTime);
				rotKey.myRotation = AiQuaternionToCUQuaternion(aiRotKey.mValue);

				track.myRotationKeys.push_back(rotKey);
			}

			// If first key is not at tick [0] duplicate second
			{
				if (track.myScaleKeys.size() > 1)
				{
					if (track.myScaleKeys[0].myTimeTicks > 0.f)
					{
						auto key = track.myScaleKeys[0];
						key.myTimeTicks = 0.f;
						track.myScaleKeys.insert(track.myScaleKeys.begin(), key);
					}
				}
				if (track.myRotationKeys.size() > 1)
				{
					if (track.myRotationKeys[0].myTimeTicks > 0.f)
					{
						auto key = track.myRotationKeys[0];
						key.myTimeTicks = 0.f;
						track.myRotationKeys.insert(track.myRotationKeys.begin(), key);
					}
				}
				if (track.myTranslationKeys.size() > 1)
				{
					if (track.myTranslationKeys[0].myTimeTicks > 0.f)
					{
						auto key = track.myTranslationKeys[0];
						key.myTimeTicks = 0.f;
						track.myTranslationKeys.insert(track.myTranslationKeys.begin(), key);
					}
				}
			}

			myTracks.push_back(track);
		}

		CreateBoneToAnimMap(myToBoneTrackIndex, scene->mRootNode, anim);

		CreateBoneNodes(scene->mRootNode, -1);

		float ms = END_TIMER_GET_RESULT_MS(timer);

		LOG_INFO(LogType::Resource) << "Anim Load Ms: " << ms << " ms";

		return true;
	}

	bool AnimationClip::InitCustomAnim(const Path& aPath)
	{
		//START_TIMER(timer);

		myPath = aPath.ToString();

		Engine::G6AnimDeserializer deserializer;

		{
			//START_TIMER(readfiletimer);

			if (!deserializer.Deserialize(aPath))
			{
				LOG_ERROR(LogType::Resource) << "Failed to deserialize: " << aPath;
				return false;
			}

			//float readfilems = END_TIMER_GET_RESULT_MS(readfiletimer);
			//LOG_INFO(LogType::Resource) << "Anim read file ms: " << readfilems;
		}

		const auto& header = deserializer.GetHeader();

		myTicksPerSecond = header.myTicksPerSecond;

		if (myTicksPerSecond == 0.f)
		{
			LOG_ERROR(LogType::Animation) << "Animation has 0 ticks per second, whot?";
			return false;
		}

		myDurationInTicks = header.myDurationInTicks;
		myDurationSeconds = myDurationInTicks / myTicksPerSecond;

		{
			START_TIMER(loop1timer);

			myTracks.reserve(header.myTracksCount);

			for (int i = 0; i < header.myTracksCount; ++i)
			{
				const auto& g6Track = header.myTracks.asPtr[i];

				Track track;
				track.myName = deserializer.GetString(g6Track.myNameIndex);

				// Remove the namespace part of the meshname for meshes with namespaces
				const auto colonIndex = track.myName.find(':');

				if (colonIndex != std::string::npos)
				{
					assert(false && "this should already be removed since its my own format");
					track.myName = track.myName.substr(colonIndex + 1);
				}

				track.myTranslationKeys.reserve(g6Track.myTranslationKeysCount);

				// Translation keys
				for (unsigned int transKeyIndex = 0;
					transKeyIndex < g6Track.myTranslationKeysCount;
					++transKeyIndex)
				{
					const auto& g6TransKey =
						header.myTranslationKeys.asPtr[g6Track.myTranslationKeysIndex + transKeyIndex];

					TranslationKey transKey;
					transKey.myTimeTicks = g6TransKey.myTimeTicks;
					transKey.myPosition = g6TransKey.myPosition;

					track.myTranslationKeys.push_back(transKey);
				}

				track.myScaleKeys.reserve(g6Track.myScaleKeysCount);

				// Scale keys
				for (unsigned int scaleKeyIndex = 0;
					scaleKeyIndex < g6Track.myScaleKeysCount;
					++scaleKeyIndex)
				{
					const auto& g6ScaleKey =
						header.myScaleKeys.asPtr[g6Track.myScaleKeysIndex + scaleKeyIndex];

					ScaleKey scaleKey;
					scaleKey.myTimeTicks = g6ScaleKey.myTimeTicks;
					scaleKey.myScale = g6ScaleKey.myScale;

					track.myScaleKeys.push_back(scaleKey);
				}

				track.myRotationKeys.reserve(g6Track.myRotationKeysCount);

				// Rotation keys
				for (unsigned int rotationKeyIndex = 0;
					rotationKeyIndex < g6Track.myRotationKeysCount;
					++rotationKeyIndex)
				{
					const auto& g6RotationKey =
						header.myRotationKeys.asPtr[g6Track.myRotationKeysIndex + rotationKeyIndex];

					RotationKey rotationKey;
					rotationKey.myTimeTicks = g6RotationKey.myTimeTicks;
					rotationKey.myRotation = g6RotationKey.myRotation;

					track.myRotationKeys.push_back(rotationKey);
				}

				myTracks.push_back(track);
			}

			float loop1ems = END_TIMER_GET_RESULT_MS(loop1timer);
			{
				LOG_INFO(LogType::Resource) << "Anim loop1timer ms: " << loop1ems << " tid: " << std::this_thread::get_id() << " path: " << aPath;
			}
		}

		{
			// START_TIMER(lasttimer);

			for (int i = 0; i < header.myBoneNodeNameToTrackIndicesCount; ++i)
			{
				const auto& boneNodeNameToTrackIndex = header.myBoneNodeNameToTrackIndices.asPtr[i];
				myToBoneTrackIndex[deserializer.GetString(boneNodeNameToTrackIndex.myNameFileOffset)] =
					boneNodeNameToTrackIndex.myIndex;
			}

			// CreateBoneToAnimMap(myToBoneTrackIndex, scene->mRootNode, anim);

			myBoneNodes.reserve(header.myBoneNodesCount);

			for (int i = 0; i < header.myBoneNodesCount; ++i)
			{
				const auto& boneNode = header.myBoneNodes.asPtr[i];

				OurNode ourNode;
				{
					ourNode.myName = deserializer.GetString(boneNode.myNameFileOffset);
					ourNode.myParentIndex = boneNode.myParentIndex;
					ourNode.myRotation = boneNode.myRotation;
					ourNode.myScale = boneNode.myScale;
					ourNode.myTranslation = boneNode.myTranslation;
					ourNode.myBoneNameToIndex = boneNode.myBoneToNameIndex;
					ourNode.myBoneNameToTrackIndex = boneNode.myBoneToTrackIndex;
				}

				myBoneNodes.push_back(ourNode);
			}

			//float last = END_TIMER_GET_RESULT_MS(lasttimer);
			//LOG_INFO(LogType::Resource) << "Anim LAST Ms: " << last << " ms";
		}

		// CreateBoneNodes(scene->mRootNode, -1);

		//float ms = END_TIMER_GET_RESULT_MS(timer);
		//
		//LOG_INFO(LogType::Resource) << "Anim Load Ms: " << ms << " ms";

		return true;
	}

	int AnimationClip::FindBoneAnim(const std::string& aNodeName)
	{
		for (unsigned int i = 0; i < myTracks.size(); ++i)
		{
			if (myTracks[i].myName == aNodeName)
			{
				return i;
			}
		}

		return -1;
	}

	void AnimationClip::CreateBoneToAnimMap(
		std::unordered_map<std::string, int>& aMap,
		const aiNode* aNode,
		const aiAnimation* aAnimation)
	{
		std::string nodeName = aNode->mName.data;

		// Remove the namespace part of the meshname for meshes with namespaces
		const auto colonIndex = nodeName.find(':');

		if (colonIndex != std::string::npos)
		{
			nodeName = nodeName.substr(colonIndex + 1);
		}

		const auto animNode = FindBoneAnim(nodeName);

		assert(aMap.find(nodeName) == aMap.end() && "duplciates");

		aMap[nodeName] = animNode;

		for (unsigned int i = 0; i < aNode->mNumChildren; ++i)
		{
			const auto& child = aNode->mChildren[i];

			CreateBoneToAnimMap(aMap, child, aAnimation);
		}
	}

	bool Approximately(Quatf quatA, Quatf value, float acceptableRange)
	{
		return 1.f - fabs(quatA.Dot(value)) < acceptableRange;
	}

	void AnimationClip::CreateBoneNodes(const aiNode* aNode, const int aParentIndex)
	{
		OurNode ourNode;

		aiVector3D scaling;
		aiQuaterniont<float> rotation;
		aiVector3D translation;
		aNode->mTransformation.Decompose(scaling, rotation, translation);

		ourNode.myScale = AiVector3DToCUVector3f(scaling);
		ourNode.myRotation = AiQuaternionToCUQuaternion(rotation);
		ourNode.myTranslation = AiVector3DToCUVector3f(translation);

		//// TEMP TEST CODE
		//{
		//	CU::Matrix4f ma = CU::Matrix4f::CreateScale(ourNode.myScale)
		//		* ourNode.myRotation.ToMatrix()
		//		* CU::Matrix4f::CreateTranslation(ourNode.myTranslation);

		//	CU::Vector3f t;
		//	CU::Vector3f s;
		//	CU::Quaternion r;
		//	ma.Decompose(t, r, s);

		//	assert(CU::AreEqual(t.x, ourNode.myTranslation.x) &&
		//		CU::AreEqual(t.y, ourNode.myTranslation.y) &&
		//		CU::AreEqual(t.z, ourNode.myTranslation.z));

		//	assert(CU::AreEqual(s.x, ourNode.myScale.x) &&
		//		CU::AreEqual(s.y, ourNode.myScale.y) &&
		//		CU::AreEqual(s.z, ourNode.myScale.z));

		//	// assert(Approximately(r, ourNode.myRotation, 0.001f));

		//	// assert(CU::AreEqual(r.EulerAngles().x, ourNode.myRotation.EulerAngles().x) &&
		//	// 	CU::AreEqual(r.EulerAngles().y, ourNode.myRotation.EulerAngles().y) &&
		//	// 	CU::AreEqual(r.EulerAngles().z, ourNode.myRotation.EulerAngles().z));

		//	if (!Approximately(r, ourNode.myRotation, 0.001f))
		//	{
		//		ma.Decompose(t, r, s);
		//	}

		//	//assert(CU::AreEqual(r.myW, ourNode.myRotation.myW) &&
		//	//	CU::AreEqual(r.myVector.x, ourNode.myRotation.myVector.x) &&
		//	//	CU::AreEqual(r.myVector.y, ourNode.myRotation.myVector.y) &&
		//	//	CU::AreEqual(r.myVector.z, ourNode.myRotation.myVector.z));

		//	ourNode.myScale = s;
		//	ourNode.myTranslation = t;
		//	ourNode.myRotation = r;

		//	int test = 0;
		//}

		ourNode.myName = aNode->mName.data;

		// Remove the namespace part of the meshname for meshes with namespaces
		const auto colonIndex = ourNode.myName.find(':');

		if (colonIndex != std::string::npos)
		{
			ourNode.myName = ourNode.myName.substr(colonIndex + 1);
		}

		// ourNode.myTransformation = CU::Matrix4f::Transpose(AiMatrixToCUMatrix(aNode->mTransformation));
		ourNode.myParentIndex = aParentIndex;

		const int currentIndex = myBoneNodes.size();
		myBoneNodes.push_back(ourNode);

		for (unsigned int i = 0; i < aNode->mNumChildren; ++i)
		{
			const auto& child = aNode->mChildren[i];

			CreateBoneNodes(child, currentIndex);
		}
	}

	float AnimationClip::GetTicksPerSecond() const
	{
		return myTicksPerSecond;
	}

	float AnimationClip::GetDurationInTicks() const
	{
		return myDurationInTicks;
	}

	float AnimationClip::GetDurationSeconds() const
	{
		return myDurationSeconds;
	}

	const std::vector<OurNode>& AnimationClip::GetBoneNodes() const
	{
		return myBoneNodes;
	}

	const std::unordered_map<std::string, int>& AnimationClip::GetToBoneTrackIndex() const
	{
		return myToBoneTrackIndex;
	}

	const std::vector<Track>& AnimationClip::GetTracks() const
	{
		return myTracks;
	}

	const std::string& AnimationClip::GetPath() const
	{
		return myPath;
	}

}