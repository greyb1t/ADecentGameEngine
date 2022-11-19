#pragma once

namespace Common
{
	template <class T>
	class DoublyLinkedList;

	template <class T>
	class DoublyLinkedListNode
	{
	public:
		// Copy-konstruktorn och assignment-operatorn är borttagna, så att det enda
		// sättet att skapa en nod är genom att stoppa in ett värde i en lista.
		DoublyLinkedListNode<T>(const DoublyLinkedListNode<T>&) = delete;

		DoublyLinkedListNode<T>& operator=(const DoublyLinkedListNode<T>&) = delete;

		// Returnerar nodens värde
		const T& GetValue() const;
		T& GetValue();

		// Returnerar nästa nod i listan, eller nullptr om noden är sist i listan
		DoublyLinkedListNode<T>* GetNext() const;

		// Returnerar föregående nod i listan, eller nullptr om noden är först i
		// listan
		DoublyLinkedListNode<T>* GetPrevious() const;

	private:
		// Konstruktorn och destruktorn är privat, så att man inte kan skapa eller
		// ta bort noder utifrån. List-klassen är friend, så att den kan skapa
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