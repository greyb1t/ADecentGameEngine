#pragma once

#include "DoublyLinkedListNode.hpp"

namespace Common
{
	template <class T>
	class DoublyLinkedList
	{
	public:
		// Skapar en tom lista
		DoublyLinkedList();

		// Frigör allt minne som listan allokerat
		~DoublyLinkedList();

		// Returnerar antalet element i listan
		int GetSize() const;

		// Returnerar första noden i listan, eller nullptr om listan är tom
		DoublyLinkedListNode<T>* GetFirst();

		// Returnerar sista noden i listan, eller nullptr om listan är tom
		DoublyLinkedListNode<T>* GetLast();

		// Skjuter in ett nytt element först i listan
		void InsertFirst(const T& aValue);

		// Skjuter in ett nytt element sist i listan
		void InsertLast(const T& aValue);

		// Skjuter in ett nytt element innan aNode
		void InsertBefore(DoublyLinkedListNode<T>* aNode, const T& aValue);

		// Skjuter in ett nytt element efter aNode
		void InsertAfter(DoublyLinkedListNode<T>* aNode, const T& aValue);

		// Plockar bort noden ur listan och frigör minne. (Det är ok att anta att
		// aNode är en nod i listan, och inte från en annan lista)
		void Remove(DoublyLinkedListNode<T>* aNode);

		// Hittar första elementet i listan som har ett visst värde. Jämförelsen
		// görs med operator==. Om inget element hittas returneras nullptr.
		DoublyLinkedListNode<T>* FindFirst(const T& aValue);

		// Hittar sista elementet i listan som har ett visst värde. Jämförelsen
		// görs med operator==. Om inget element hittas returneras nullptr.
		DoublyLinkedListNode<T>* FindLast(const T& aValue);

		// Plockar bort första elementet i listan som har ett visst värde.
		// Jämförelsen görs med operator==. Om inget element hittas görs ingenting.
		// Returnerar true om ett element plockades bort, och false annars.
		bool RemoveFirst(const T& aValue);

		// Plockar bort sista elementet i listan som har ett visst värde.
		// Jämförelsen görs med operator==. Om inget element hittas görs ingenting.
		// Returnerar true om ett element plockades bort, och false annars.
		bool RemoveLast(const T& aValue);

	private:
		int myCount = 0;

		DoublyLinkedListNode<T>* myHead = nullptr;
		DoublyLinkedListNode<T>* myTail = nullptr;
	};

	template<class T>
	inline DoublyLinkedList<T>::DoublyLinkedList()
	{
		myHead = DBG_NEW DoublyLinkedListNode<T>(T{ });
		myTail = DBG_NEW DoublyLinkedListNode<T>(T{ });

		myHead->myNext = myTail;
		myTail->myPrevious = myHead;
	}

	template<class T>
	inline DoublyLinkedList<T>::~DoublyLinkedList()
	{
		auto it = myHead;

		while (it != nullptr)
		{
			auto next = it->GetNext();

			delete it;
			it = nullptr;

			it = next;
		}
	}

	template<class T>
	inline int DoublyLinkedList<T>::GetSize() const
	{
		return myCount;
	}

	template<class T>
	inline DoublyLinkedListNode<T>* DoublyLinkedList<T>::GetFirst()
	{
		if (myCount != 0)
		{
			return myHead->GetNext();
		}
		else
		{
			return nullptr;
		}
	}

	template<class T>
	inline DoublyLinkedListNode<T>* DoublyLinkedList<T>::GetLast()
	{
		if (myCount != 0)
		{
			return myTail->GetPrevious();
		}
		else
		{
			return nullptr;
		}
	}

	template<class T>
	inline void DoublyLinkedList<T>::InsertFirst(const T& aValue)
	{
		DoublyLinkedListNode<T>* node = DBG_NEW DoublyLinkedListNode<T>(aValue);

		node->myNext = myHead->myNext;
		myHead->myNext->myPrevious = node;

		node->myPrevious = myHead;
		myHead->myNext = node;

		myCount++;
	}

	template<class T>
	inline void DoublyLinkedList<T>::InsertLast(const T& aValue)
	{
		DoublyLinkedListNode<T>* node = DBG_NEW DoublyLinkedListNode<T>(aValue);

		myTail->myPrevious->myNext = node;
		node->myPrevious = myTail->myPrevious;

		node->myNext = myTail;
		myTail->myPrevious = node;

		myCount++;
	}

	template<class T>
	inline void DoublyLinkedList<T>::InsertBefore(DoublyLinkedListNode<T>* aNode, const T& aValue)
	{
		DoublyLinkedListNode<T>* node = DBG_NEW DoublyLinkedListNode<T>(aValue);

		auto temp = aNode->myPrevious;

		aNode->myPrevious->myNext = node;
		aNode->myPrevious = node;

		node->myNext = aNode;
		node->myPrevious = temp;

		myCount++;
	}

	template<class T>
	inline void DoublyLinkedList<T>::InsertAfter(DoublyLinkedListNode<T>* aNode, const T& aValue)
	{
		DoublyLinkedListNode<T>* node = DBG_NEW DoublyLinkedListNode<T>(aValue);

		auto temp = aNode->myNext;

		aNode->myNext = node;
		temp->myPrevious = node;

		node->myNext = temp;
		node->myPrevious = aNode;

		myCount++;
	}

	template<class T>
	inline void DoublyLinkedList<T>::Remove(DoublyLinkedListNode<T>* aNode)
	{
		aNode->myPrevious->myNext = aNode->myNext;
		aNode->myNext->myPrevious = aNode->myPrevious;

		delete aNode;
		aNode = nullptr;

		myCount--;
	}

	template<class T>
	inline DoublyLinkedListNode<T>* DoublyLinkedList<T>::FindFirst(const T& aValue)
	{
		for (DoublyLinkedListNode<T>* it = GetFirst(); it != nullptr; it = it->GetNext())
		{
			if (aValue == it->GetValue())
			{
				return it;
			}
		}

		return nullptr;
	}

	template<class T>
	inline DoublyLinkedListNode<T>* DoublyLinkedList<T>::FindLast(const T& aValue)
	{
		for (DoublyLinkedListNode<T>* it = GetLast(); it != nullptr; it = it->GetPrevious())
		{
			if (aValue == it->GetValue())
			{
				return it;
			}
		}

		return nullptr;
	}

	template<class T>
	inline bool DoublyLinkedList<T>::RemoveFirst(const T& aValue)
	{
		DoublyLinkedListNode<T>* first = FindFirst(aValue);
		if (first != nullptr)
		{
			Remove(first);
			return true;
		}
		return false;
	}

	template<class T>
	inline bool DoublyLinkedList<T>::RemoveLast(const T& aValue)
	{
		DoublyLinkedListNode<T>* last = FindLast(aValue);
		if (last != nullptr)
		{
			Remove(last);
			return true;
		}
		return false;
	}

}

namespace CU = Common;