#pragma once

#include "Reflectable.h"

namespace Engine
{
	class Reflector;

	template <typename T>
	class Array : public Reflectable
	{
	public:
		Array(std::vector<T>& aVector, ReflectionFlags aFlag)
			: myVector(aVector), myFlags{ aFlag }
		{}

		void Reflect(Reflector& aReflector) override
		{
			int arraySize = myVector.size();

			if (aReflector.Reflect(arraySize, "Size") & ReflectorResult_Changed)
			{
				myVector.resize(std::max(arraySize, 0));
			}

			//if (arraySize != myVector.size())
			//{
			//	myVector.resize(std::max(arraySize, 0));
			//}

			for (size_t i = 0; i < myVector.size(); ++i)
			{
				// Reflect(Int& aInt)
				aReflector.Reflect(myVector[i], std::to_string(i), myFlags);
			}
		}

	private:
		std::vector<T>& myVector;
		ReflectionFlags myFlags = 0;
	};
}
