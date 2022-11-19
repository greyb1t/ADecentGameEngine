#pragma once

#include <cmath>
#include <cassert>

namespace Common
{
	template <class T>
	class BSTSet
	{
	public:
		BSTSet() = default;
		~BSTSet();

		//Returnerar true om elementet finns i mängden, och false annars.
		bool HasElement(const T& aValue);

		//Stoppar in elementet i mängden, om det inte redan finns där. Gör
		//ingenting annars.
		void Insert(const T& aValue);

		//Plockar bort elementet ur mängden, om det finns. Gör ingenting annars.
		void Remove(const T& aValue);

		void DSWBalance();

	private:
		struct TreeNode
		{
			TreeNode(const T& aData)
				: myData(aData)
			{
			}

			void ReplaceChildWith(TreeNode*& aChild, TreeNode*& aTreeNode);
			void SetLeftSubTree(TreeNode*& aSubtree);
			void SetRightSubTree(TreeNode*& aSubtree);

			T myData;

			TreeNode* myLeftChild = nullptr;
			TreeNode* myRightChild = nullptr;
		};

		void EmptyTree(TreeNode*& aNode);
		void Insert(const T& aValue, TreeNode*& aNode);
		bool HasElement(const T& aValue, TreeNode*& aNode);
		void Remove(const T& aValue, TreeNode*& aNode);
		TreeNode* FindLargest(TreeNode*& aNode);

		TreeNode* myRoot = nullptr;

		void ConvertToBackbone(TreeNode*& aParent, TreeNode*& aGrandParent);
		void RotateRight(TreeNode*& aGrandParent, TreeNode*& aParent, TreeNode*& aChild);
		void RotateLeft(TreeNode*& aGrandParent, TreeNode*& aParent, TreeNode*& aChild);

		int CountNodes();
		int CountNodes(TreeNode* aTreeNode);

		void BalanceBackbone(const int aNodeAmount);
		void Compact(const int aNumberOfRotations);
	};


	template<class T>
	inline typename BSTSet<T>::TreeNode* BSTSet<T>::FindLargest(BSTSet<T>::TreeNode*& aNode)
	{
		if (aNode == nullptr)
		{
			return nullptr;
		}
		else if (aNode->myRightChild == nullptr)
		{
			return aNode;
		}
		else
		{
			return FindLargest(aNode->myRightChild);
		}
	}

	template<class T>
	inline void BSTSet<T>::ConvertToBackbone(BSTSet<T>::TreeNode*& aParent, BSTSet<T>::TreeNode*& aGrandParent)
	{
		if (aParent == nullptr)
		{
			return;
		}

		if (aParent->myLeftChild != nullptr)
		{
			while (aParent->myLeftChild != nullptr)
			{
				RotateRight(aGrandParent, aParent, aParent->myLeftChild);
			}
		}

		if (aParent->myLeftChild == nullptr && aParent->myRightChild == nullptr)
		{
			return;
		}

		if (aParent->myLeftChild != nullptr)
		{
			ConvertToBackbone(aParent->myLeftChild, aParent);
		}

		if (aParent->myRightChild != nullptr)
		{
			ConvertToBackbone(aParent->myRightChild, aParent);
		}
	}

	template<class T>
	inline void BSTSet<T>::RotateRight(
		BSTSet<T>::TreeNode*& aGrandParent,
		BSTSet<T>::TreeNode*& aParent,
		BSTSet<T>::TreeNode*& aChild)
	{
		BSTSet<T>::TreeNode* savedParentPointer = aParent;

		if (aGrandParent != nullptr)
		{
			aGrandParent->ReplaceChildWith(aParent, aChild);
		}

		BSTSet<T>::TreeNode* savedChild = aChild;

		savedParentPointer->SetLeftSubTree(aChild->myRightChild);
		savedChild->SetRightSubTree(savedParentPointer);

		if (aGrandParent == nullptr)
		{
			myRoot = savedChild;
		}
	}

	template<class T>
	inline void BSTSet<T>::RotateLeft(TreeNode*& aGrandParent, TreeNode*& aParent, TreeNode*& aChild)
	{
		BSTSet<T>::TreeNode* savedParentPointer = aParent;

		if (aGrandParent != nullptr)
		{
			aGrandParent->ReplaceChildWith(aParent, aChild);
		}

		BSTSet<T>::TreeNode* savedChild = aChild;

		savedParentPointer->SetRightSubTree(aChild->myLeftChild);
		savedChild->SetLeftSubTree(savedParentPointer);

		if (aGrandParent == nullptr)
		{
			myRoot = savedChild;
		}
	}

	template<class T>
	inline int BSTSet<T>::CountNodes()
	{
		if (myRoot == nullptr)
		{
			return 0;
		}
		else
		{
			return CountNodes(myRoot);
		}
	}

	template<class T>
	inline int BSTSet<T>::CountNodes(TreeNode* aTreeNode)
	{
		int count = 1;

		if (aTreeNode->myLeftChild != nullptr)
		{
			count += CountNodes(aTreeNode->myLeftChild);
		}

		if (aTreeNode->myRightChild != nullptr)
		{
			count += CountNodes(aTreeNode->myRightChild);
		}

		return count;
	}

	template<class T>
	inline void BSTSet<T>::BalanceBackbone(const int aNodeAmount)
	{
		int m = static_cast<int>(pow(2, floor(log2(aNodeAmount + 1))) - 1);

		Compact(aNodeAmount - m);

		while (m > 1)
		{
			m /= 2;

			Compact(m);
		}
	}

	template<class T>
	inline void BSTSet<T>::Compact(const int aNumberOfRotations)
	{
		TreeNode* grandChild = nullptr;
		TreeNode* currentNode = myRoot;

		for (size_t index = 0; index < aNumberOfRotations; index++)
		{
			TreeNode* savedCurrentNode = currentNode->myRightChild;

			RotateLeft(grandChild, currentNode, currentNode->myRightChild);

			grandChild = savedCurrentNode;
			currentNode = savedCurrentNode->myRightChild;
		}
	}

	template<class T>
	inline void BSTSet<T>::Remove(const T& aValue, BSTSet<T>::TreeNode*& aNode)
	{
		if (aNode == nullptr)
		{
			return;
		}
		else if (aValue < aNode->myData)
		{
			Remove(aValue, aNode->myLeftChild);
		}
		else if (aNode->myData < aValue)
		{
			Remove(aValue, aNode->myRightChild);
		}
		else
		{
			if (aNode->myLeftChild == nullptr && aNode->myRightChild == nullptr)
			{
				// No children

				delete aNode;
				aNode = nullptr;
			}
			else if (aNode->myLeftChild != nullptr && aNode->myRightChild != nullptr)
			{
				// Two children

				// Copy the data
				aNode->myData = FindLargest(aNode->myLeftChild)->myData;

				Remove(aNode->myData, aNode->myLeftChild);
			}
			else
			{
				//One child
				TreeNode* tempNode = aNode;

				if (aNode->myLeftChild != nullptr)
				{
					aNode = aNode->myLeftChild;
				}
				else
				{
					aNode = aNode->myRightChild;
				}

				delete tempNode;
				tempNode = nullptr;
			}
		}
	}

	template<class T>
	inline void BSTSet<T>::Remove(const T& aValue)
	{
		Remove(aValue, myRoot);
	}

	template<class T>
	inline void BSTSet<T>::DSWBalance()
	{
		TreeNode* grandParent = nullptr;
		ConvertToBackbone(myRoot, grandParent);
		BalanceBackbone(CountNodes());
	}

	template <class T>
	void Common::BSTSet<T>::Insert(const T& aValue, BSTSet<T>::TreeNode*& aNode)
	{
		if (aNode == nullptr)
		{
			aNode = DBG_NEW TreeNode(aValue);
		}
		else if (aValue < aNode->myData)
		{
			Insert(aValue, aNode->myLeftChild);
		}
		else if (aNode->myData < aValue)
		{
			Insert(aValue, aNode->myRightChild);
		}
	}

	template <class T>
	void Common::BSTSet<T>::Insert(const T& aValue)
	{
		Insert(aValue, myRoot);
	}

	template <class T>
	Common::BSTSet<T>::~BSTSet()
	{
		EmptyTree(myRoot);
	}

	template<class T>
	inline bool BSTSet<T>::HasElement(const T& aValue, BSTSet<T>::TreeNode*& aNode)
	{
		if (aNode == nullptr)
		{
			return false;
		}
		else if (aValue < aNode->myData)
		{
			return HasElement(aValue, aNode->myLeftChild);
		}
		else if (aNode->myData < aValue)
		{
			return HasElement(aValue, aNode->myRightChild);
		}
		else
		{
			return true;
		}
	}

	template<class T>
	inline bool BSTSet<T>::HasElement(const T& aValue)
	{
		return HasElement(aValue, myRoot);
	}

	template <class T>
	void Common::BSTSet<T>::EmptyTree(BSTSet<T>::TreeNode*& aNode)
	{
		if (aNode != nullptr)
		{
			EmptyTree(aNode->myLeftChild);
			EmptyTree(aNode->myRightChild);

			delete aNode;
			aNode = nullptr;
		}
	}

	template<class T>
	inline void BSTSet<T>::TreeNode::ReplaceChildWith(BSTSet<T>::TreeNode*& aChild, BSTSet<T>::TreeNode*& aTreeNode)
	{
		if (aChild == myRightChild)
		{
			myRightChild = aTreeNode;
		}
		else if (aChild == myLeftChild)
		{
			myLeftChild = aTreeNode;
		}
		else
		{
			assert(false);
		}

	}

	template<class T>
	inline void BSTSet<T>::TreeNode::SetLeftSubTree(TreeNode*& aSubtree)
	{
		myLeftChild = aSubtree;
	}

	template<class T>
	inline void BSTSet<T>::TreeNode::SetRightSubTree(TreeNode*& aSubtree)
	{
		myRightChild = aSubtree;
	}
}

namespace CU = Common;