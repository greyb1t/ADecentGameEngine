#pragma once

#include "BlendTree.h"
#include "BlendNode2D.h"
#include "Common/delaunay.h"

namespace Engine
{
	class BlendTree2D : public BlendTree
	{
	public:
		struct Triangle
		{
			Triangle()
				: myDelaTri({}, {}, {})
			{}

			Triangle(const delaunay::Triangle<float>& aTriangle)
				: myDelaTri(aTriangle)
			{}

			int myNodeIndex1 = -1;
			int myNodeIndex2 = -1;
			int myNodeIndex3 = -1;

			delaunay::Triangle<float> myDelaTri;
		};

		void AddNode(BlendNode2D aNode);
		void RemoveNode(const int aIndex);

		AnimationTransformResult CalculateInterpolatedTransforms() override;

		AnimationTransform GetRootMotionDelta() const override;

		bool HasFinished() const override;
		void Reset() override;
		float GetElapsedTimeNormalized() const override;

		void Update(AnimationState& aState, const float aDeltaTime) override;

		float GetDurationSeconds() const override;

		bool IsRootMotion() const override;

		// when adding a node, and later on modifying it, we must
		// call this to recalculate the tree variables
		void RecalculateTriangles();

		void InitFromJson2(const nlohmann::json& aJson, AnimationMachine& aMachine) override;
		nlohmann::json ToJson() const override;
		void SetElapsedTimeNormalized(const float aTimeNormalized) override;
		void SetIsRootMotion(const bool aIsRootMotion) override;
		void SetTimeScale(const float aTimeScale) override;
		float GetTimeScale() const override;
		const std::string& GetName() const override;
		std::vector<const AnimationEvent*> GetEventsExecutedThisFrame() override;
		bool IsLooping() const override;
		void SetIsLooping(const bool aIsLooping) override;
		bool CanPlay() const override;

		void Accept(PlayableVisitor& aVisitor) override;

		delaunay::Delaunay<float> TriangulateNodes() const;

	private:
		// The right blendnode will be nullptr if no other node was found to blend with
		// this occurs when the blendvalue is the exact same as one of the
		// blend node thesholds
		// aU, aV and aW is the barycentric coords used for the blend weight
		Triangle* DetermineTriangleToBlendWith(float& aU, float& aV, float& aW);

		void RemoveVec2Usage(const std::string& aFloatName) override;

		const Vec2f& GetBlendValue() const;

		// Returns -1 when it was not found
		int GetNodeIndexFromPosition(const Vec2f& aPosition) const;

		void Barycentric(
			const Vec2f& p,
			const Vec2f& a,
			const Vec2f& b,
			const Vec2f& c,
			float& u,
			float& v,
			float& w);

		float DistanceToTriangle(const Vec2f& aPosition, const delaunay::Triangle<float>& aTriangle);
		float DistanceToEdge(const Vec2f& aPosition, const delaunay::Edge<float>& anEdge);
		Triangle& FindClosestTriangle(const Vec2f& aPosition);

	private:
		friend class AnimationNodeEditorWindow;
		friend class FindAnimationClipsVisitor;
		friend class BlendTree2DEditor;

		AnimationMachine* myMachine = nullptr;

		std::string myName = "BlendTreeName";

		std::vector<BlendNode2D> myNodes;
		std::vector<Triangle> myTriangles;

		// float myMinThreshold = 0.f;
		// float myMaxThreshold = 1.f;

		std::string myBlendVec2Name;
		Vec2f* myBlendValue = nullptr;

		float myNormalizedTime = 0.f;

		// The duration is the time of the shortest animation
		float myDurationSeconds = 0.f;

		bool myIsLooping = false;

		AnimationTransform myRootMotion;
		AnimationTransform myRootMotionPrevious;
	};
}