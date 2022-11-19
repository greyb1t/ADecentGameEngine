#include "stdafx.h"
#include "kDTree.h"
#include "kDTreeNode.h"
#include "GameManager.h"

bool kDTree::Init()
{
	const auto min = CU::Vector2f(0, 0);
	const auto max = CU::Vector2f(800, 600);

	myRootNode = DBG_NEW kDTreeNode();
	myRootNode->Init(nullptr, SplitAxis::X, false, min, max);

	return true;
}

void kDTree::Render()
{
	myRootNode->RenderNode();
}

void kDTree::ResetHighlights()
{
	myRootNode->ResetHighlights();
}

void kDTree::InsertObject(kDTreeObject* aObject)
{
	myRootNode->InsertObject(aObject);
}

std::vector<kDTreeObject*> kDTree::GetObjectsFromArea(const CU::AABB2D<float>& aABBB)
{
	return myRootNode->GetObjectsFromArea(aABBB);
}

std::vector<kDTreeNode*> kDTree::GetNodesFromArea(const CU::AABB2D<float>& aAABB)
{
	return myRootNode->GetNodesFromArea(aAABB);
}

std::vector<kDTreeNode*> kDTree::GetLeafNodesFromArea(const CU::AABB2D<float>& aAABB)
{
	return myRootNode->GetNodesFromArea(aAABB);
}
