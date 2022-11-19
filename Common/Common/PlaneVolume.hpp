#pragma once

#include "Plane.hpp"

#include <vector>

namespace Common
{
	template<typename T>
	class PlaneVolume
	{
	public:
		PlaneVolume() = default;

		// Constructor taking a list of Plane that makes up the PlaneVolume.
		PlaneVolume(const std::vector<Plane<T>>& aPlaneList);

		// Add a Plane to the PlaneVolume.
		void AddPlane(const Plane<T>& aPlane);

		// Returns whether a point is inside the PlaneVolume: it is inside when the point is on the
		// plane or on the side the normal is pointing away from for all the planes in the PlaneVolume.
		bool IsInside(const Vector3<T>& aPosition) const;
		bool IsInside(const Sphere<T>& aSphere) const;

		const std::vector<Plane<T>>& GetPlanes() const;

	private:
		std::vector<Plane<T>> myPlanes;
	};

	template<typename T>
	inline PlaneVolume<T>::PlaneVolume(const std::vector<Plane<T>>& aPlaneList)
		: myPlanes(aPlaneList)
	{
	}

	template<typename T>
	inline void PlaneVolume<T>::AddPlane(const Plane<T>& aPlane)
	{
		myPlanes.push_back(aPlane);
	}

	template<typename T>
	inline bool PlaneVolume<T>::IsInside(const Vector3<T>& aPosition) const
	{
		for (const auto& plane : myPlanes)
		{
			if (!plane.IsInside(aPosition))
			{
				return false;
			}
		}

		return true;
	}

	template<typename T>
	inline bool PlaneVolume<T>::IsInside(const Sphere<T>& aSphere) const
	{
		for (const auto& plane : myPlanes)
		{
			if (!plane.IsInside(aSphere))
			{
				return false;
			}
		}

		return true;
	}

	template<typename T>
	const std::vector<Plane<T>>& PlaneVolume<T>::GetPlanes() const
	{
		return myPlanes;
	}
}

namespace CU = Common;