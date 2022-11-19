#include "pch.h"
#include "kDTreeNode.h"
#include "Utils.h"
#include "kDTreeObject.h"
#include <algorithm>
#include "kDTree.h"

void kDTreeNode::Init(
	kDTreeNode* aParent,
	const SplitAxis aSplitAxis,
	const bool aIsTopOrLeftChild,
	const CU::Vector2f& aRectMin,
	const CU::Vector2f& aRectMax)
{
	myParent = aParent;
	mySplitAxis = aSplitAxis;
	myIsTopOrLeftChild = aIsTopOrLeftChild;

	myRectMin = aRectMin;
	myRectMax = aRectMax;
}

void kDTreeNode::RenderNode()
{
	if (HasChildren())
	{
		for (const auto& child : myChildren)
		{
			if (child)
			{
				child->RenderNode();
			}
		}

		return;
	}

	const auto resolution = Utils::GetRenderSize();

	CU::Vector2f topLeft = myRectMin;
	CU::Vector2f bottomRight = myRectMax;

	if (myIsHighlighted)
	{
		Tga2D::CSprite onePixel;
		onePixel.Init("sprites/onepixel.png");
		onePixel.SetSizeRelativeToImage({ bottomRight.x - topLeft.x, bottomRight.y - topLeft.y });
		onePixel.SetPosition({
			(topLeft.x + (bottomRight.x - topLeft.x) * 0.5f) / resolution.x,
			(topLeft.y + (bottomRight.y - topLeft.y) * 0.5f) / resolution.y
			});
		onePixel.SetPivot({ 0.5f, 0.5f });

		onePixel.SetColor({ 76.f / 255.f, 0, 35.f / 255.f, 1.f });

		onePixel.Render();
	}

	topLeft.x /= resolution.x;
	topLeft.y /= resolution.y;

	bottomRight.x /= resolution.x;
	bottomRight.y /= resolution.y;

	const auto color = Tga2D::CColor(0.f, 1.f, 0.f, 1.f);
	Tga2D::CDebugDrawer::DrawLine(Tga2D::Vector2f(topLeft.x, topLeft.y), Tga2D::Vector2f(topLeft.x, bottomRight.y), color);
	Tga2D::CDebugDrawer::DrawLine(Tga2D::Vector2f(topLeft.x, bottomRight.y), Tga2D::Vector2f(bottomRight.x, bottomRight.y), color);
	Tga2D::CDebugDrawer::DrawLine(Tga2D::Vector2f(bottomRight.x, bottomRight.y), Tga2D::Vector2f(bottomRight.x, topLeft.y), color);
	Tga2D::CDebugDrawer::DrawLine(Tga2D::Vector2f(bottomRight.x, topLeft.y), Tga2D::Vector2f(topLeft.x, topLeft.y), color);
}

void kDTreeNode::InsertObject(kDTreeObject* aObject)
{
	assert(aObject != nullptr);

	if (HasChildren())
	{
		myObjects.push_back(aObject);

		InsertIntoCorrectChild(aObject, myObjects.size() - 1);
	}
	else
	{
		myObjects.push_back(aObject);

		if (myObjects.size() > 1)
		{
			DetermineSplitValue();

			auto topOrLeftChildMin = myRectMin;
			auto topOrLeftChildMax = myRectMax;

			if (mySplitAxis == SplitAxis::X)
			{
				// left child
				topOrLeftChildMax.x = mySplitValue;
			}
			else if (mySplitAxis == SplitAxis::Y)
			{
				// top child
				topOrLeftChildMax.y = mySplitValue;
			}

			// top or left
			myChildren[0] = DBG_NEW kDTreeNode;
			myChildren[0]->Init(this, mySplitAxis == SplitAxis::X ? SplitAxis::Y : SplitAxis::X, true, topOrLeftChildMin, topOrLeftChildMax);

			auto bottomOrRightChildMin = myRectMin;
			auto bottomOrRightChildMax = myRectMax;

			if (mySplitAxis == SplitAxis::X)
			{
				// right child
				bottomOrRightChildMin.x = mySplitValue;
			}
			else if (mySplitAxis == SplitAxis::Y)
			{
				// bottom child
				bottomOrRightChildMin.y = mySplitValue;
			}

			// bottom or right
			myChildren[1] = DBG_NEW kDTreeNode;
			myChildren[1]->Init(this, mySplitAxis == SplitAxis::X ? SplitAxis::Y : SplitAxis::X, false, bottomOrRightChildMin, bottomOrRightChildMax);

			for (int i = static_cast<int>(myObjects.size()) - 1; i >= 0; i--)
			{
				InsertIntoCorrectChild(myObjects[i], i);
			}
		}
	}
}

std::vector<kDTreeObject*> kDTreeNode::GetObjectsFromArea(const CU::AABB2D<float>& aAABB)
{
	std::vector<kDTreeObject*> objectList;
	auto nodes = GetNodesFromArea(aAABB);

	for (auto& node : nodes)
	{
		objectList.insert(objectList.end(), node->myObjects.begin(), node->myObjects.end());
	}

	return objectList;
}

std::vector<kDTreeNode*> kDTreeNode::GetNodesFromArea(const CU::AABB2D<float>& aAABB, bool aAppendOnlyLeafNodes)
{
	std::vector<kDTreeNode*> nodeList;

	GetNodesFromAreaInternal(aAABB, nodeList);

	return nodeList;
}

bool kDTreeNode::IsTopOrLeftChild() const
{
	return myIsTopOrLeftChild;
}

void kDTreeNode::Highight()
{
	myIsHighlighted = true;
}

void kDTreeNode::ResetHighlights()
{
	ResetHighlightsInternal();
}

void kDTreeNode::GetNodesFromAreaInternal(const CU::AABB2D<float>& aAABB, std::vector<kDTreeNode*>& aNodeList, bool aAppendOnlyLeafNodes)
{
	// Base case
	if (HasChildren() == false)
	{
		aNodeList.push_back(this);

		return;
	}

	if (aAppendOnlyLeafNodes == false)
	{
		// If there was an object straddling we need to check this node
		if (myObjects.empty() == false)
		{
			aNodeList.push_back(this);
		}
	}

	for (int i = 0; i < static_cast<int>(myChildren.size()); ++i)
	{
		const auto child = myChildren[i];

		if (mySplitAxis == SplitAxis::X)
		{
			if (child->IsTopOrLeftChild())
			{
				if (aAABB.GetMin().x < mySplitValue)
				{
					// recurse down
					child->GetNodesFromAreaInternal(aAABB, aNodeList);
				}
			}
			else
			{
				if (aAABB.GetMax().x > mySplitValue)
				{
					// recurse down
					child->GetNodesFromAreaInternal(aAABB, aNodeList);
				}
			}
		}
		else if (mySplitAxis == SplitAxis::Y)
		{
			if (child->IsTopOrLeftChild())
			{
				if (aAABB.GetMin().y < mySplitValue)
				{
					// recurse down
					child->GetNodesFromAreaInternal(aAABB, aNodeList);
				}
			}
			else
			{
				if (aAABB.GetMax().y > mySplitValue)
				{
					// recurse down
					child->GetNodesFromAreaInternal(aAABB, aNodeList);
				}
			}
		}
		else
		{
			assert(false);
		}
	}
}

void kDTreeNode::ResetHighlightsInternal()
{
	myIsHighlighted = false;

	if (HasChildren())
	{
		for (auto& child : myChildren)
		{
			child->ResetHighlightsInternal();
		}
	}
}

bool kDTreeNode::HasChildren() const
{
	return myChildren[0] != nullptr;
}

void kDTreeNode::DetermineSplitValue()
{
	auto sortObjectBasedOnAxis = [this](kDTreeObject* aLeft, kDTreeObject* aRight)
	{
		if (mySplitAxis == SplitAxis::X)
		{
			return aLeft->GetPosition().x < aRight->GetPosition().x;
		}
		else if (mySplitAxis == SplitAxis::Y)
		{
			return aLeft->GetPosition().y < aRight->GetPosition().y;
		}
		else
		{
			assert(false);
		}

		return false;
	};

	std::sort(myObjects.begin(), myObjects.end(), sortObjectBasedOnAxis);

	const auto middleLeftOrTopObject = myObjects[(myObjects.size() / 2) - 1];
	const auto middleRightOrBottomObject = myObjects[myObjects.size() / 2];

	float middleLeftOrTopMax = -1.f;
	float middleRightOrBottomMin = -1.f;

	if (mySplitAxis == SplitAxis::X)
	{
		middleLeftOrTopMax = middleLeftOrTopObject->GetPosition().x + middleLeftOrTopObject->GetHalfSize();
		middleRightOrBottomMin = middleRightOrBottomObject->GetPosition().x - middleRightOrBottomObject->GetHalfSize();
	}
	else if (mySplitAxis == SplitAxis::Y)
	{
		middleLeftOrTopMax = middleLeftOrTopObject->GetPosition().y + middleLeftOrTopObject->GetHalfSize();
		middleRightOrBottomMin = middleRightOrBottomObject->GetPosition().y - middleRightOrBottomObject->GetHalfSize();
	}
	else
	{
		assert(false);
	}

	const float delta = middleRightOrBottomMin - middleLeftOrTopMax;

	const float middlePoint = (delta / 2) + middleLeftOrTopMax;
	mySplitValue = middlePoint;
}

void kDTreeNode::InsertIntoCorrectChild(kDTreeObject* aObject, const int aObjectIndex)
{
	float objectPositionOnAxis = 0.f;

	if (mySplitAxis == SplitAxis::X)
	{
		objectPositionOnAxis = aObject->GetPosition().x;
	}
	else if (mySplitAxis == SplitAxis::Y)
	{
		objectPositionOnAxis = aObject->GetPosition().y;
	}

	const float objectMin = objectPositionOnAxis - aObject->GetHalfSize();
	const float objectMax = objectPositionOnAxis + aObject->GetHalfSize();

	if (objectMax < mySplitValue)
	{
		myChildren[0]->InsertObject(aObject);

		myObjects[aObjectIndex] = myObjects.back();
		myObjects.pop_back();
	}
	else if (objectMin > mySplitValue)
	{
		myChildren[1]->InsertObject(aObject);

		myObjects[aObjectIndex] = myObjects.back();
		myObjects.pop_back();
	}
	else
	{
		// straddling, do not recurse it downwards
	}
}
