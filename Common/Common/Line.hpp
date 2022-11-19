#pragma once

#include "Vector2.hpp"

namespace Common
{
	template <typename T>
	class Line
	{
	public:
		// Default constructor: there is no line, the normal is the zero vector.
		Line();

		Line(const Line <T>& aLine) = default;

		// Constructor that takes two points that define the line, the direction is aPoint1 - aPoint0.
		Line(const Vector2<T>& aPoint0, const Vector2<T>& aPoint1);

		void InitWith2Points(const Vector2<T>& aPoint0, const Vector2<T>& aPoint1);

		// Init the line with a point and a direction.
		void InitWithPointAndDirection(const Vector2<T>& aPoint, const Vector2<T>& aDirection);

		// Returns whether a point is inside the line: it is inside when the point 
		// is on the line or on the side the normal is pointing away from.
		bool IsInside(const Vector2<T>& aPosition) const;

		// Returns the direction of the line.
		const Vector2<T>& GetDirection() const;

		// Returns the normal of the line, which is (-direction.y, direction.x).
		const Vector2<T>& GetNormal() const;

	private:
		Vector2<T> myPoint;
		Vector2<T> myDirection;
		Vector2<T> myNormal;
	};

	template<typename T>
	inline Line<T>::Line()
		: myPoint { }, myDirection { }, myNormal { }
	{
	}

	template<typename T>
	inline Line<T>::Line(const Vector2<T>& aPoint0, const Vector2<T>& aPoint1)
	{
		InitWith2Points(aPoint0, aPoint1);
	}

	template<typename T>
	inline void Line<T>::InitWith2Points(const Vector2<T>& aPoint0, const Vector2<T>& aPoint1)
	{
		myPoint = aPoint0;
		myDirection = (aPoint1 - aPoint0).GetNormalized();
		myNormal = Vector2<T>(-myDirection.y, myDirection.x);
	}

	template<typename T>
	inline void Line<T>::InitWithPointAndDirection(const Vector2<T>& aPoint, const Vector2<T>& aDirection)
	{
		myPoint = aPoint;
		myDirection = aDirection.GetNormalized();
		myNormal = Vector2<T>(-myDirection.y, myDirection.x);
	}

	template<typename T>
	inline bool Line<T>::IsInside(const Vector2<T>& aPosition) const
	{
		return myNormal.Dot(aPosition - myPoint) <= static_cast<T>(0);
	}

	template<typename T>
	inline const Vector2<T>& Line<T>::GetDirection() const
	{
		return myDirection;
	}

	template<typename T>
	inline const Vector2<T>& Line<T>::GetNormal() const
	{
		return myNormal;
	}
}

namespace CU = Common;