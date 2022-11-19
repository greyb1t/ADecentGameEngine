#pragma once

#include <vector>
#include <cassert>

namespace Common
{
	template <class T>
	class Stack
	{
	public:
		Stack() = default;

		//Returnerar antal element i stacken
		int GetSize() const;

		//Returnerar det �versta elementet i stacken. Kraschar med en assert om
		//stacken �r tom.
		const T& GetTop() const;

		//Returnerar det �versta elementet i stacken. Kraschar med en assert om
		//stacken �r tom.
		T& GetTop();

		//L�gger in ett nytt element �verst p� stacken
		void Push(const T& aValue);

		//Tar bort det �versta elementet fr�n stacken och returnerar det. Kraschar
		//med en assert om stacken �r tom.
		T Pop();

	private:
		std::vector<T> myElements;
	};

	template<class T>
	inline int Stack<T>::GetSize() const
	{
		return static_cast<int>(myElements.size());
	}

	template<class T>
	inline const T& Stack<T>::GetTop() const
	{
		assert(myElements.empty() == false);

		return myElements.back();
	}

	template<class T>
	inline T& Stack<T>::GetTop()
	{
		assert(myElements.empty() == false);

		return myElements.back();
	}

	template<class T>
	inline void Stack<T>::Push(const T& aValue)
	{
		myElements.push_back(aValue);
	}

	template<class T>
	inline T Stack<T>::Pop()
	{
		assert(myElements.empty() == false);

		T temp = GetTop();

		myElements.pop_back();

		return temp;
	}
}

namespace CU = Common;