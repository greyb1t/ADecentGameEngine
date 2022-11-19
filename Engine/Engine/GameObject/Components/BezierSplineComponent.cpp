#include "pch.h"
#include "BezierSplineComponent.h"
#include "Engine/Reflection/Reflector.h"
#include "Engine/DebugManager/DebugDrawer.h"
#include "Common/Math/Bezier.h"
#include "Engine/Editor/SceneBeingEdited.h"
#include "Engine/Engine.h"
#include "Engine/Editor/Editor.h"
#include "Engine/Editor/EditorWindows/SceneWindow.h"

Engine::BezierSplineComponent::BezierSplineComponent(GameObject* aGameObject)
	: Component(aGameObject)
{
}

void Engine::BezierSplineComponent::Reflect(Reflector& aReflector)
{
	Component::Reflect(aReflector);

	if (aReflector.Button("Add Point"))
	{
		mySpline.AddPoint({});
	}

	aReflector.Reflect(mySpline.GetPoints(), "Points");
}

void Engine::BezierSplineComponent::Render()
{
}

void Engine::BezierSplineComponent::EditorExecute()
{

}

void Engine::BezierSplineComponent::EditorExecuteSelected()
{
	Vec3f pos;

	if (myGameObject)
	{
		pos = GetTransform().GetPosition();
	}

	static int selectedIndex = 0;

	const int precision = 100;

	for (int i = 0; i < precision; ++i)
	{
		const float percent0 = static_cast<float>(i) / static_cast<float>(precision);
		const float percent1 = static_cast<float>(i + 1) / static_cast<float>(precision);

		const Vec3f p0 = Evaluate(percent0);
		const Vec3f p1 = Evaluate(percent1);

		GDebugDrawer->DrawLine3D(
			DebugDrawFlags::Always,
			p0,
			p1);
	}

	for (int i = 0; i < mySpline.GetPoints().size(); ++i)
	{
		auto& p = mySpline.GetPoints()[i];
		GDebugDrawer->DrawSphere3D(DebugDrawFlags::Always, pos + p, 5.f);

		if (i != selectedIndex)
		{
			continue;
		}

		ImGui::PushID(&p);

		SceneBeingEdited& e = GetEngine().GetEditor().GetActiveScene2();
		SceneWindow& s = GetEngine().GetEditor().GetSceneWindow();

		// e.UpdateGizmos();

		//Mat4f mat = Mat4f::CreateTranslation(p);
		Mat4f mat = Mat4f::CreateScale(Vec3f(1, 1, 1))
			* Quatf().ToMatrix()
			* Mat4f::CreateTranslation(pos + p);

		e.DrawGizmos(
			GizmosFlags_Translation,
			s.GetWindowItemRectMin().x,
			s.GetWindowItemRectMin().y,
			s.GetWindowItemRectSize().x,
			s.GetWindowItemRectSize().y, mat);

		Vec3f trans;
		Vec3f scale;
		Quatf rot;
		mat.Decompose(trans, rot, scale);

		p = trans - pos;

		ImGui::PopID();

		// GDebugDrawer->DrawLine3D(DebugDrawFlags::Always, p, );
	}

	ImGui::Begin("fuck", 0, ImGuiWindowFlags_NoFocusOnAppearing);
	for (int i = 0; i < mySpline.GetPoints().size(); ++i)
	{
		bool isSelected = selectedIndex == i;
		if (ImGui::Checkbox(std::to_string(i).c_str(), &isSelected))
		{
			selectedIndex = i;
		}
	}
	ImGui::End();
}

Vec3f Engine::BezierSplineComponent::Evaluate(const float aT) const
{
	Mat4f m;

	if (myGameObject != nullptr)
	{
		m = GetTransform().GetMatrix();
	}

	auto test = Vec4f(mySpline.Evaluate(aT), 1.f) * m;
	return test.ToVec3();
}

BezierSpline& Engine::BezierSplineComponent::GetSpline()
{
	return mySpline;
}
