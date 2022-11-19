#pragma once

#include <vector>

#include "Math.h"

class BezierSpline
{
public:
	void AddPoint(const Vec3f& aPoint);

	Vec3f Evaluate(const float aT) const;

	const std::vector<Vec3f> &GetPoints() const;
	std::vector<Vec3f>& GetPoints();

private:
	std::vector<Vec3f> myPoints;
};