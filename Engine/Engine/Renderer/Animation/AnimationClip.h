#pragma once

#include "Track.h"
#include "OurNode.h"

struct aiNode;
struct aiAnimation;

namespace Engine
{
	class SkinnedModel;
}

namespace Engine
{
	class Animation;

	// Holds multiple different tracks
	class AnimationClip
	{
	public:
		AnimationClip();
		~AnimationClip();

		bool Init(const Path& aPath);
		bool InitCustomAnim(const Path& aPath);

		// Returns -1 if not found
		int FindBoneAnim(const std::string& aNodeName);

		void CreateBoneToAnimMap(
			std::unordered_map<std::string, int>& aMap,
			const aiNode* aNode,
			const aiAnimation* aAnimation);

		void CreateBoneNodes(const aiNode* aNode, const int aParentIndex);

		float GetTicksPerSecond() const;
		float GetDurationInTicks() const;
		float GetDurationSeconds() const;

		const std::vector<OurNode>& GetBoneNodes() const;
		const std::unordered_map<std::string, int>& GetToBoneTrackIndex() const;
		const std::vector<Track>& GetTracks() const;

		const std::string& GetPath() const;

	private:
		friend class AnimationClipLoader;

		std::string myPath;

		std::vector<Track> myTracks;
		float myTicksPerSecond = 0.f;
		float myDurationInTicks = 0.f;
		float myDurationSeconds = 0.f;

		std::unordered_map<std::string, int> myToBoneTrackIndex;

		std::vector<OurNode> myBoneNodes;
	};
}