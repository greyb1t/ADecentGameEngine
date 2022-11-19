#pragma once

namespace Common
{
	template <class T>
	class DoublyLinkedList;

	template <class T>
	class DoublyLinkedListNode
	{
	public:
		// Copy-konstruktorn och assignment-operatorn �r borttagna, s� att det enda
		// s�ttet att skapa en nod �r genom att stoppa in ett v�rde i en lista.
		DoublyLinkedListNode<T>(const DoublyLinkedListNode<T>&) = delete;

		DoublyLinkedListNode<T>& operator=(const DoublyLinkedListNode<T>&) = delete;

		// Returnerar nodens v�rde
		const T& GetValue() const;
		T& GetValue();

		// Returnerar n�sta nod i listan, eller nullptr om noden �r sist i listan
		DoublyLinkedListNode<T>* GetNext() const;

		// Returnerar f�reg�ende nod i listan, eller nullptr om noden �r f�rst i
		// listan
		DoublyLinkedListNode<T>* GetPrevious() const;

	private:
		// Konstruktorn och destruktorn �r privat, s� att man inte kan skapa eller
		// ta bort noder utifr�n. List-klassen �r friend, s� att den kan skapa
		// eller ta bort noder.
		friend class DoublyLinkedList<T>;

		DoublyLinkedListNode(const T& aValue);
		~DoublyLinkedListNode() { }

		T myValue;

		DoublyLinkedListNode<T>* myPrevious = nullptr;
		DoublyLinkedListNode<T>* myNext = nullptr;
	};

	template<class T>
	inline const T& DoublyLinkedListNode<T>::GetValue() const
	{
		return myValue;
	}

	template<class T>
	inline T& DoublyLinkedListNode<T>::GetValue()
	{
		return myValue;
	}

	template<class T>
	inline DoublyLinkedListNode<T>* DoublyLinkedListNode<T>::GetNext() const
	{
		if (myNext && myNext->myNext == nullptr)
		{
			return nullptr;
		}
		else
		{
			return myNext;
		}
	}

	template<class T>
	inline DoublyLinkedListNode<T>* DoublyLinkedListNode<T>::GetPrevious() const
	{
		if (myPrevious && myPrevious->myPrevious == nullptr)
		{
			return nullptr;
		}
		else
		{
			return myPrevious;
		}
	}

	template<class T>
	inline DoublyLinkedListNode<T>::DoublyLinkedListNode(const T& aValue) : myValue(aValue)
	{
	}

}

namespace CU = Common;