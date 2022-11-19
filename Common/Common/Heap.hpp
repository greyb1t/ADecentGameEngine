#pragma once
#include <vector>
#include <algorithm>
#include <assert.h>

namespace Common
{
	template <class T>
	class Heap
	{
	public:
		// returnerar antal element i heapen
		int GetSize() const;

		// lägger till elementet i heapen
		void Enqueue(const T& aElement);

		// returnerar det största elementet i heapen
		const T& GetTop() const;

		// Tar bort det största elementet ur heapen och returnerar det
		T Dequeue();

	private:
		enum class Child
		{
			Right,
			Left,
			None
		};

		void BubbleUp(const size_t aIndex);
		void BubbleDown(const size_t aIndex);

		Child NextBubbleDownElement(const size_t aIndex, const T& aValue) const;

		const T& ParentOf(const size_t aIndex) const;
		int ParentIndexOf(const size_t aIndex) const;

		const T& LeftChildOf(const size_t aIndex) const;
		int LeftChildIndexOf(const size_t aIndex) const;

		const T& RightChildOf(const size_t aIndex) const;
		int RightChildIndexOf(const size_t aIndex) const;

		Child GetBiggestChildOf(const size_t aIndex) const;

		std::vector<T> myData;
	};


	template<class T>
	inline int Heap<T>::GetSize() const
	{
		return static_cast<int>(myData.size());
	}

	template<class T>
	inline void Heap<T>::Enqueue(const T& aElement)
	{
		myData.push_back(aElement);

		BubbleUp(myData.size() - 1);
	}

	template <class T>
	const T& Common::Heap<T>::GetTop() const
	{
		return myData.front();
	}

	template <class T>
	T Common::Heap<T>::Dequeue()
	{
		const T saved = std::move(myData[0]);

		myData[0] = std::move(myData.back());

		myData.pop_back();

		BubbleDown(0);

		return saved;
	}

	template<class T>
	inline const T& Heap<T>::ParentOf(const size_t aIndex) const
	{
		return myData[ParentIndexOf(aIndex)];
	}

	template <class T>
	int Common::Heap<T>::ParentIndexOf(const size_t aIndex) const
	{
		return (static_cast<int>(aIndex) - 1) / 2;
	}

	template <class T>
	const T& Common::Heap<T>::LeftChildOf(const size_t aIndex) const
	{
		return myData[LeftChildIndexOf(aIndex)];
	}

	template <class T>
	int Common::Heap<T>::LeftChildIndexOf(const size_t aIndex) const
	{
		return 2 * static_cast<int>(aIndex) + 1;
	}

	template <class T>
	const T& Common::Heap<T>::RightChildOf(const size_t aIndex) const
	{
		return myData[RightChildIndexOf(aIndex)];
	}

	template <class T>
	int Common::Heap<T>::RightChildIndexOf(const size_t aIndex) const
	{
		return 2 * static_cast<int>(aIndex) + 2;
	}

	template<class T>
	inline typename Common::Heap<T>::Child Heap<T>::GetBiggestChildOf(const size_t aIndex) const
	{
		if (LeftChildOf(aIndex) < RightChildOf(aIndex))
		{
			return Child::Right;
		}
		else
		{
			return Child::Left;
		}
	}

	template<class T>
	inline void Heap<T>::BubbleUp(const size_t aIndex)
	{
		if (aIndex <= 0)
		{
			return;
		}

		size_t index = aIndex;

		const T savedData = myData[aIndex];

		while (ParentOf(index) < savedData)
		{
			myData[index] = std::move(ParentOf(index));

			index = ParentIndexOf(index);

			const bool isRoot = index <= 0;

			// STOP bubbling up if reached root
			if (isRoot)
			{
				break;
			}
		}

		myData[index] = std::move(savedData);
	}

	template<class T>
	inline void Heap<T>::BubbleDown(const size_t aIndex)
	{
		if (aIndex >= myData.size())
		{
			return;
		}

		size_t index = aIndex;

		const T savedData = myData[aIndex];

		Child currentChild = NextBubbleDownElement(index, savedData);

		while (currentChild != Child::None)
		{
			if (currentChild == Child::Right)
			{
				myData[index] = std::move(RightChildOf(index));
				index = RightChildIndexOf(index);
			}
			else if (currentChild == Child::Left)
			{
				myData[index] = std::move(LeftChildOf(index));
				index = LeftChildIndexOf(index);
			}
			else
			{
				assert(false && "should not occur");
			}

			currentChild = NextBubbleDownElement(index, savedData);
		}

		myData[index] = std::move(savedData);
	}

	template<class T>
	inline typename Common::Heap<T>::Child Heap<T>::NextBubbleDownElement(const size_t aIndex, const T& aComparableValue) const
	{
		//This works becuse this heap is vänsterbalanserad

		const bool hasRightChild = RightChildIndexOf(aIndex) >= 0 && RightChildIndexOf(aIndex) < myData.size();
		const bool hasLeftChild = LeftChildIndexOf(aIndex) >= 0 && LeftChildIndexOf(aIndex) < myData.size();

		if (hasLeftChild && hasRightChild)
		{
			//return GetBiggestChildOf(aIndex);

			const Child child = GetBiggestChildOf(aIndex);

			if (child == Child::Right)
			{
				if (aComparableValue < RightChildOf(aIndex))
				{
					return Child::Right;
				}
			}
			else if (child == Child::Left)
			{
				if (aComparableValue < LeftChildOf(aIndex))
				{
					return Child::Left;
				}
			}

			return Child::None;
		}
		else
		{
			if (hasRightChild)
			{
				if (aComparableValue < RightChildOf(aIndex))
				{
					return Child::Right;
				}
			}
			else if (hasLeftChild)
			{
				if (aComparableValue < LeftChildOf(aIndex))
				{
					return Child::Left;
				}
			}
		}

		return Child::None;
	}
}

namespace CU = Common;