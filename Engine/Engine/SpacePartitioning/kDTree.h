#pragma once

#include "AABB2D.hpp"

class kDTreeNode;
class kDTreeObject;

class kDTree
{
public:
	bool Init();

	void Render();

	void ResetHighlights();

	void InsertObject(kDTreeObject* aObject);

	std::vector<kDTreeObject*> GetObjectsFromArea(const CU::AABB2D<float>& aABBB);
	std::vector<kDTreeNode*> GetNodesFromArea(const CU::AABB2D<float>& aAABB);
	std::vector<kDTreeNode*> GetLeafNodesFromArea(const CU::AABB2D<float>& aAABB);

private:
	kDTreeNode* myRootNode = nullptr;
};