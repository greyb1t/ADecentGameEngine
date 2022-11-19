#pragma once

#include <initializer_list>
#include <array>
#include <assert.h>

namespace Common
{
	template <typename Type, int Capacity, typename CountType = unsigned short, bool UseSafeModeFlag = true>
	class VectorOnStack
	{
	public:
		VectorOnStack() = default;
		VectorOnStack(const VectorOnStack& aVectorOnStack);
		VectorOnStack(const std::initializer_list<Type>& aInitList);

		~VectorOnStack();

		VectorOnStack& operator=(const VectorOnStack& aVectorOnStack);

		inline const Type& operator[](const CountType aIndex) const;
		inline Type& operator[](const CountType aIndex);

		inline void Add(const Type& aObject);
		inline void Insert(const CountType aIndex, const Type& aObject);

		inline void RemoveCyclic(const Type& aObject);
		inline void RemoveCyclicAtIndex(const CountType aIndex);
		inline void Clear();

		inline CountType Size() const;

		using ContainerType = typename std::array<Type, Capacity>;
		using Iterator = typename ContainerType::iterator;
		using ConstIterator = typename ContainerType::const_iterator;

		typename Iterator begin();
		typename Iterator end();

		typename ConstIterator cbegin() const;
		typename ConstIterator cend() const;

	private:
		bool IsFull() const;

		ContainerType myData = { };
		CountType mySize = 0;
	};

	template<typename Type, int Capacity, typename CountType, bool UseSafeModeFlag>
	inline VectorOnStack<Type, Capacity, CountType, UseSafeModeFlag>::VectorOnStack(const VectorOnStack& aVectorOnStack)
	{
		operator=(aVectorOnStack);
	}

	template<typename Type, int Capacity, typename CountType, bool UseSafeModeFlag>
	inline VectorOnStack<Type, Capacity, CountType, UseSafeModeFlag>::VectorOnStack(const std::initializer_list<Type>& aInitList)
	{
		assert(aInitList.size() <= Capacity && "Initializer is too big for the vector");

		for (const auto& value : aInitList)
		{
			Add(value);
		}
	}

	template<typename Type, int Capacity, typename CountType, bool UseSafeModeFlag>
	inline VectorOnStack<Type, Capacity, CountType, UseSafeModeFlag>::~VectorOnStack()
	{
		mySize = 0;
	}

	template<typename Type, int Capacity, typename CountType, bool UseSafeModeFlag>
	inline VectorOnStack<Type, Capacity, CountType, UseSafeModeFlag>&
		VectorOnStack<Type, Capacity, CountType, UseSafeModeFlag>::operator=(const VectorOnStack& aVectorOnStack)
	{
		if (UseSafeModeFlag)
		{
			myData = aVectorOnStack.myData;
			mySize = aVectorOnStack.mySize;
		}
		else
		{
			memcpy(this, &aVectorOnStack, sizeof(aVectorOnStack));
		}

		return *this;
	}

	template<typename Type, int Capacity, typename CountType, bool UseSafeModeFlag>
	inline const Type& VectorOnStack<Type, Capacity, CountType, UseSafeModeFlag>::operator[](const CountType aIndex) const
	{
		assert(aIndex >= 0 && aIndex < mySize && "Index is outside the bounds");
		return myData[aIndex];
	}

	template<typename Type, int Capacity, typename CountType, bool UseSafeModeFlag>
	inline Type& VectorOnStack<Type, Capacity, CountType, UseSafeModeFlag>::operator[](const CountType aIndex)
	{
		assert(aIndex >= 0 && aIndex < mySize && "Index is outside the bounds");
		return myData[aIndex];
	}

	template<typename Type, int Capacity, typename CountType, bool UseSafeModeFlag>
	inline void VectorOnStack<Type, Capacity, CountType, UseSafeModeFlag>::Add(const Type& aObject)
	{
		assert(!IsFull() && "The internal array is full");

		myData[mySize++] = aObject;
	}

	template<typename Type, int Capacity, typename CountType, bool UseSafeModeFlag>
	inline void VectorOnStack<Type, Capacity, CountType, UseSafeModeFlag>::Insert(const CountType aIndex, const Type& aObject)
	{
		assert(!IsFull() && "The internal array is full");
		assert(aIndex >= 0 && aIndex <= mySize && "Index is outside the bounds");

		if (UseSafeModeFlag)
		{
			const auto lastElementIndex = mySize - 1;

			for (int i = lastElementIndex; i >= aIndex; --i)
			{
				myData[i + 1] = myData[i];
			}

			myData[aIndex] = aObject;
		}
		else
		{
			const auto destination = &myData[aIndex + 1];
			const auto source = &myData[aIndex];
			const auto elementToMoveCount = mySize - aIndex;

			memcpy(destination, source, sizeof(Type) * elementToMoveCount);
			memcpy(source, &aObject, sizeof(Type));
		}

		++mySize;
	}

	template<typename Type, int Capacity, typename CountType, bool UseSafeModeFlag>
	inline void VectorOnStack<Type, Capacity, CountType, UseSafeModeFlag>::RemoveCyclic(const Type& aObject)
	{
		for (int i = 0; i < mySize; ++i)
		{
			const auto& object = myData[i];

			if (aObject == object)
			{
				RemoveCyclicAtIndex(i);
				break;
			}
		}
	}

	template<typename Type, int Capacity, typename CountType, bool UseSafeModeFlag>
	inline void VectorOnStack<Type, Capacity, CountType, UseSafeModeFlag>::RemoveCyclicAtIndex(const CountType aIndex)
	{
		assert(aIndex >= 0 && aIndex < mySize && "Index is outside bounds");
		myData[aIndex] = myData[mySize - 1];
		--mySize;
	}

	template<typename Type, int Capacity, typename CountType, bool UseSafeModeFlag>
	inline void VectorOnStack<Type, Capacity, CountType, UseSafeModeFlag>::Clear()
	{
		mySize = 0;
	}

	template<typename Type, int Capacity, typename CountType, bool UseSafeModeFlag>
	inline CountType VectorOnStack<Type, Capacity, CountType, UseSafeModeFlag>::Size() const
	{
		return mySize;
	}

	template<typename Type, int Capacity, typename CountType, bool UseSafeModeFlag>
	inline typename VectorOnStack<Type, Capacity, CountType, UseSafeModeFlag>::Iterator
		VectorOnStack<Type, Capacity, CountType, UseSafeModeFlag>::begin()
	{
		return myData.begin();
	}

	template<typename Type, int Capacity, typename CountType, bool UseSafeModeFlag>
	inline typename VectorOnStack<Type, Capacity, CountType, UseSafeModeFlag>::Iterator
		VectorOnStack<Type, Capacity, CountType, UseSafeModeFlag>::end()
	{
		return myData.begin() + mySize;
	}

	template<typename Type, int Capacity, typename CountType, bool UseSafeModeFlag>
	inline typename VectorOnStack<Type, Capacity, CountType, UseSafeModeFlag>::ConstIterator
		VectorOnStack<Type, Capacity, CountType, UseSafeModeFlag>::cbegin() const
	{
		return myData.cbegin();
	}

	template<typename Type, int Capacity, typename CountType, bool UseSafeModeFlag>
	inline typename VectorOnStack<Type, Capacity, CountType, UseSafeModeFlag>::ConstIterator
		VectorOnStack<Type, Capacity, CountType, UseSafeModeFlag>::cend() const
	{
		return myData.cbegin() + mySize;
	}

	template<typename Type, int Capacity, typename CountType, bool UseSafeModeFlag>
	inline bool VectorOnStack<Type, Capacity, CountType, UseSafeModeFlag>::IsFull() const
	{
		return mySize >= Capacity;
	}
}

namespace CU = Common;