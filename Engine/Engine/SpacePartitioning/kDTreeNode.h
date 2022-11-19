#pragma once

#include <array>

#include "SplitAxis.h"
#include "AABB2D.hpp"

class kDTreeObject;
class kDTree;

class kDTreeNode
{
public:
	void Init(
		kDTreeNode* aParent, 
		const SplitAxis aSplitAxis,
		const bool aIsTopOrLeftChild, 
		const CU::Vector2f& aRectMin,
		const CU::Vector2f& aRectMax);

	void RenderNode();

	void InsertObject(kDTreeObject* aObject);

	std::vector<kDTreeObject*> GetObjectsFromArea(const CU::AABB2D<float>& aAABB);
	std::vector<kDTreeNode*> GetNodesFromArea(const CU::AABB2D<float>& aAABB, bool aAppendOnlyLeafNodes = false);

	bool IsTopOrLeftChild() const;

	void Highight();

	void ResetHighlights();

private:
	void GetNodesFromAreaInternal(const CU::AABB2D<float>& aAABB, std::vector<kDTreeNode*>& aNodeList, bool aAppendOnlyLeafNodes = false);
	void ResetHighlightsInternal();

	bool HasChildren() const;

	void DetermineSplitValue();

	void InsertIntoCorrectChild(kDTreeObject* aObject, const int aObjectIndex);

private:
	
	// For visualizing purposes
	kDTreeNode* myParent = nullptr;
	bool myIsTopOrLeftChild = false;
	CU::Vector2f myRectMin;
	CU::Vector2f myRectMax;
	bool myIsHighlighted = false;

	// 0 = left/top child
	// 1 = right/bottom child
	std::array<kDTreeNode*, 2> myChildren = {};

	SplitAxis mySplitAxis = SplitAxis::Undefined;

	float mySplitValue = 0.f;

	std::vector<kDTreeObject*> myObjects;
};