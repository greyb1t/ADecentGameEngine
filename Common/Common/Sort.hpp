#pragma once

#include <vector>
#include <algorithm>

namespace Common
{
	template <class T>
	void SelectionSort(std::vector<T>& aVector);

	template <class T>
	void BubbleSort(std::vector<T>& aVector);

	template <class T>
	void QuickSort(std::vector<T>& aVector);

	template <class T>
	void MergeSort(std::vector<T>& aVector);

	namespace Internal
	{
		template<class T>
		bool Equals(const T& aValueA, const T& aValueB)
		{
			return !(aValueA < aValueB || aValueB < aValueA);
		}

		template<class T>
		bool LessOrEquals(const T& aLeftValue, const T& aRightValue)
		{
			return aLeftValue < aRightValue || Equals(aLeftValue, aRightValue);
		}

		template<class T>
		void Merge(std::vector<T>& aVector, std::vector<T>& aTempVector,
			int aLeftStartIndex, int aLeftEndIndex, int aRightStartIndex, int aRightEndIndex)
		{
			int tempIndex = aLeftStartIndex;
			const int numElements = aRightEndIndex - aLeftStartIndex + 1;

			while (aLeftStartIndex <= aLeftEndIndex && aRightStartIndex <= aRightEndIndex)
			{
				if (LessOrEquals(aVector[aLeftStartIndex], aVector[aRightStartIndex]))
				{
					aTempVector[tempIndex] = std::move(aVector[aLeftStartIndex]);

					aLeftStartIndex++;
				}
				else
				{
					aTempVector[tempIndex] = std::move(aVector[aRightStartIndex]);

					aRightStartIndex++;
				}

				tempIndex++;
			}


			// Copy rest of first half
			while (aLeftStartIndex <= aLeftEndIndex)
			{
				aTempVector[tempIndex] = std::move(aVector[aLeftStartIndex]);

				tempIndex++;
				aLeftStartIndex++;
			}

			// Copy rest of right half
			while (aRightStartIndex <= aRightEndIndex)
			{
				aTempVector[tempIndex] = std::move(aVector[aRightStartIndex]);

				tempIndex++;
				aRightStartIndex++;
			}

			for (int i = 0; i < numElements; i++, --aRightEndIndex)
			{
				aVector[aRightEndIndex] = std::move(aTempVector[aRightEndIndex]);
			}
		}

		template<class T>
		void MergeSort(std::vector<T>& aVector, std::vector<T>& aTempVector,
			const int aLeftIndex, const int aRightIndex)
		{
			// Base case
			if (aLeftIndex < aRightIndex)
			{
				int center = (aLeftIndex + aRightIndex) / 2;

				MergeSort(aVector, aTempVector, aLeftIndex, center);
				MergeSort(aVector, aTempVector, center + 1, aRightIndex);
				Merge(aVector, aTempVector, aLeftIndex, center, center + 1, aRightIndex);
			}
		}

		template <class T>
		void InsertionSort(std::vector<T>& aVector, const int aLeftIndex, const int aRightIndex)
		{
			for (int i = aLeftIndex + 1; i < (aRightIndex + 1); ++i)
			{
				const auto temp = std::move(aVector[i]);

				int j = i;

				for (; j > 0 && temp < aVector[j - 1]; --j)
				{
					aVector[j] = std::move(aVector[j - 1]);
				}

				aVector[j] = std::move(temp);
			}
		}

		template <class T>
		const int Median3(std::vector<T>& aVector, const int aLeftIndex, const int aRightIndex)
		{
			const int centerIndex = (aLeftIndex + aRightIndex) / 2;

			T& centerValue = aVector[centerIndex];
			T& leftValue = aVector[aLeftIndex];
			T& rightValue = aVector[aRightIndex];

			if (centerValue < leftValue)
			{
				std::swap(centerValue, leftValue);
			}
			if (rightValue < leftValue)
			{
				std::swap(rightValue, leftValue);
			}
			if (rightValue < centerValue)
			{
				std::swap(rightValue, centerValue);
			}

			std::swap(aVector[centerIndex], aVector[aRightIndex - 1]);

			return aRightIndex - 1;
		}

		template <class T>
		void QuickSort(std::vector<T>& aVector, const int aLeftIndex, const int aRightIndex)
		{
			// Base case
			if ((aLeftIndex + 10) <= aRightIndex)
			{
				const auto& pivotIndex = Internal::Median3(aVector, aLeftIndex, aRightIndex);
				const auto& pivot = aVector[pivotIndex];

				int i = aLeftIndex;
				int j = aRightIndex - 1;

				while (true)
				{
					// Increment i until found value larger than pivot value
					while (aVector[++i] < pivot) { }

					// Decrement j until found value less than pivot
					while (pivot < aVector[--j]) { }

					if (i < j)
					{
						std::swap(aVector[i], aVector[j]);
					}
					else
					{
						break;
					}
				}

				// Move the pivot back to i
				std::swap(aVector[i], aVector[pivotIndex]);

				// NOTE: The pivot (i) is no longer touched 
				// because we are done with it

				// Sort the left sub array
				QuickSort(aVector, aLeftIndex, i - 1);

				// Sort the right sub array
				QuickSort(aVector, i + 1, aRightIndex);
			}
			else
			{
				// some other sort
				InsertionSort(aVector, aLeftIndex, aRightIndex);
			}
		}
	}

	template<class T>
	void SelectionSort(std::vector<T>& aVector)
	{
		for (int i = 0; i < static_cast<int>(aVector.size()) - 1; i++)
		{
			int currentMinIndex = i;

			for (int j = i + 1; j < static_cast<int>(aVector.size()); j++)
			{
				if (aVector[j] < aVector[currentMinIndex])
				{
					currentMinIndex = j;
				}
			}

			if (currentMinIndex != i)
			{
				std::swap(aVector[i], aVector[currentMinIndex]);
			}
		}
	}

	template<class T>
	void BubbleSort(std::vector<T>& aVector)
	{
		int totalSwaps = static_cast<int>(aVector.size()) - 1;

		bool swappedOnIteration = true;
		while (swappedOnIteration == true)
		{
			swappedOnIteration = false;

			for (int i = 0; i < totalSwaps; i++)
			{
				if (aVector[i + 1] < aVector[i])
				{
					std::swap(aVector[i + 1], aVector[i]);
					swappedOnIteration = true;
				}
			}
			totalSwaps--;
		}
	}

	template<class T>
	void MergeSort(std::vector<T>& aVector)
	{
		if (aVector.size() <= 1)
		{
			return;
		}

		std::vector<T> tempVector(aVector.size());

		Internal::MergeSort(aVector, tempVector, 0, static_cast<int>(aVector.size()) - 1);
	}

	template <class T>
	void QuickSort(std::vector<T>& aVector)
	{
		Internal::QuickSort(aVector, 0, static_cast<int>(aVector.size()) - 1);
	}
}

namespace CU = Common;