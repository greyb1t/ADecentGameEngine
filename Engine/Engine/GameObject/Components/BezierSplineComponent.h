#pragma once

#include "Component.h"

#include "Common/Math/BezierSpline.h"

namespace Engine
{
	class BezierSplineComponent : public Component
	{
	public:
		COMPONENT(BezierSplineComponent, "Bezier Spline");

		BezierSplineComponent() = default;
		BezierSplineComponent(GameObject* aGameObject);

		void Reflect(Reflector& aReflector) override;

		void Render() override;
		void EditorExecute() override;
		void EditorExecuteSelected() override;

		Vec3f Evaluate(const float aT) const;

		BezierSpline& GetSpline();

	private:
		BezierSpline mySpline;
	};
}