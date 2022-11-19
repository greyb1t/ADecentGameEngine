#pragma once

#include <vector>
#include <cassert>

namespace Common
{
	template <class T>
	class Queue
	{
	public:
		Queue();

		//Returnerar antal element i kön
		int GetSize() const;

		//Returnerar elementet längst fram i kön. Kraschar med en assert om kön är
		//tom
		const T& GetFront() const;

		//Returnerar elementet längst fram i kön. Kraschar med en assert om kön är
		//tom
		T& GetFront();

		//Lägger in ett nytt element längst bak i kön
		void Enqueue(const T& aValue);

		//Tar bort elementet längst fram i kön och returnerar det. Kraschar med en
		//assert om kön är tom.
		T Dequeue();

	private:
		int WrapAround(const int aIndex) const;
		bool IsFull() const;
		void Resize();

		void AssertEmpty() const;

		std::vector<T> myElements;

		int myFront = 1;
		int myRear = 0;

		int myCount = 0;
	};

	template <class T>
	void Queue<T>::AssertEmpty() const
	{
		assert(GetSize() > 0 && "The queue is empty");
	}

	template<class T>
	inline Queue<T>::Queue()
	{
		myElements.resize(4);
	}

	template<class T>
	inline int Queue<T>::GetSize() const
	{
		return myCount;
	}

	template<class T>
	inline const T& Queue<T>::GetFront() const
	{
		AssertEmpty();

		return myElements[myFront];
	}

	template<class T>
	inline T& Queue<T>::GetFront()
	{
		AssertEmpty();

		return myElements[myFront];
	}

	template<class T>
	inline void Queue<T>::Enqueue(const T& aValue)
	{
		if (IsFull())
		{
			Resize();
		}

		myRear = WrapAround(myRear + 1);
		myElements[myRear] = aValue;

		myCount++;
	}

	template<class T>
	inline T Queue<T>::Dequeue()
	{
		AssertEmpty();

		const T result = myElements[myFront];

		myFront = WrapAround(myFront + 1);

		myCount--;

		return result;
	}

	template<class T>
	inline bool Queue<T>::IsFull() const
	{
		return GetSize() >= static_cast<int>(myElements.size());
	}

	template<class T>
	inline void Queue<T>::Resize()
	{
		const int frontToEndSize = myElements.size() - myFront;
		const int startToRearSize = WrapAround(myRear + 1);

		std::vector<T> newElements;

		newElements.resize(myElements.size() * 2);

		int newElementSize = 0;

		// Copy the front to end of array elements
		for (int i = 0; i < frontToEndSize; i++)
		{
			newElements[i] = myElements[i + myFront];
			newElementSize++;
		}

		const int continueCount = newElementSize;

		// Copy the start of array to rear elements
		for (int i = 0; i < startToRearSize; i++)
		{
			newElements[i + continueCount] = myElements[i];
			newElementSize++;
		}

		myElements = (newElements);

		myFront = 0;
		myRear = newElementSize - 1;
	}

	template<class T>
	inline int Queue<T>::WrapAround(const int aIndex) const
	{
		if (aIndex > (static_cast<int>(myElements.size()) - 1))
		{
			return aIndex - static_cast<int>(myElements.size());
		}
		else
		{
			return aIndex;
		}
	}

}

namespace CU = Common;