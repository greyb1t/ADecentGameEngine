#include "BezierSpline.h"
#include "Bezier.h"
#include "Common/VectorOnStack.h"

void BezierSpline::AddPoint(const Vec3f& aPoint)
{
	myPoints.push_back(aPoint);
}

Vec3f BezierSpline::Evaluate(const float aT) const
{
	if (myPoints.empty())
	{
		return {};
	}

	if (myPoints.size() == 1)
	{
		return myPoints[0];
	}

	// TODO: Bad performance memory alloc
	std::vector<Vec3f> pointsCached = myPoints;

	int iterations = static_cast<int>(pointsCached.size()) - 1;

	while (iterations > 0)
	{
		for (int i = 0; i < iterations; ++i)
		{
			pointsCached[i] = Math::Lerp(pointsCached[i], pointsCached[i + 1], aT);
		}

		--iterations;
	}

	return pointsCached[0];
}

const std::vector<Vec3f>& BezierSpline::GetPoints() const
{
	return myPoints;
}

std::vector<Vec3f>& BezierSpline::GetPoints()
{
	return myPoints;
}
