#include "pch.h"
#include "RayCastAtPlaneWindow.h"

#include "Engine/Editor/EditorWindows/AssetBrowser/AssetBrowserWindow.h"
#include "Engine/Editor/Editor.h"
#include "Engine/Editor/SceneBeingEdited.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/Scene/FolderScene.h"
#include "common/Intersection.hpp"
#include "Engine/GameObject/Components/EditorCameraMovement.h"


Engine::RayCastAtPlaneWindow::RayCastAtPlaneWindow(Editor& aEditor)
	: EditorWindow(aEditor, Flags::CanBeClosed)
{

}

std::string Engine::RayCastAtPlaneWindow::GetName() const
{
	return "RayCast Editor";
}

void Engine::RayCastAtPlaneWindow::Draw(const float aDeltaTime)
{
	if (ImGui::DragFloat("Plane Y Value", &myPlaneYValue, 0.1f))
	{

	}
	if (ImGui::Selectable((
		std::to_string(myResult.x) + ", " +
		std::to_string(myResult.y) + ", " +
		std::to_string(myResult.z)).c_str()))
	{
			
	}
	if (ImGui::BeginDragDropSource())
	{
		ImGui::SetDragDropPayload("Vec3f", &myResult.x, sizeof(myResult));
		ImGui::EndDragDropSource();
	}
	
	myEditor.GetActiveScene2().SetGridY(myPlaneYValue);

	auto& input = GetEngine().GetInputManager();
	if (input.IsKeyDown(CU::KeyCode::K))
	{
		auto dir = myEditor.GetActiveScene2().GetScene()->ScreenToWorldDirection(
			GetEngine().GetViewport().myRelativeMousePosition.CastTo<unsigned int>());

		C::Plane<float> plane;
		plane.InitWithPointAndNormal(Vec3f(0.f, myPlaneYValue, 0.f), { 0.f, 1.f, 0.f });
		C::Ray<float> ray;
		ray.InitWithOriginAndDirection(
			myEditor.GetEditorCamMovement().GetGameObject()->GetTransform().GetPosition(),
			dir);

		C::IntersectionPlaneRay(plane, ray, myResult);

	}
	myEditor.GetActiveScene2().GetScene()->GetRendererScene().GetDebugDrawer().DrawSphere3D(
		DebugDrawFlags::Always,
		myResult,
		25.f,
		0.f,
		{ 1.f, 0.f, 1.f, 1.f });
}


void Engine::RayCastAtPlaneWindow::Save()
{

}


//Vec3f Engine::RayCastAtPlaneWindow::GetRayInWorldSpaceFromMouse()
//{
//
//
//	//const Vec2f resolution = Engine::Engine::GetInstance().GetGraphicsEngine().GetWindowHandler().GetWindowSize<float>();
//	//const Vec2f mousePos = Vec2f(
//	//	static_cast<float>(GetEngine().GetInputManager().GetMousePosition().x),
//	//	static_cast<float>(GetEngine().GetInputManager().GetMousePosition().y));
//
//	//auto rNew = Vec2f(resolution.x - 1.f, resolution.y - 1.f);
//
//	//auto mNew = Vec2f(mousePos.x / rNew.x, mousePos.y / rNew.y);
//
//	//Vec2f normMousePos = mNew * 2.f;
//	//Vec2f temp = Vec2f(normMousePos.x - 1.f, normMousePos.y - 1.f);
//
//	//normMousePos.y = -normMousePos.y;
//
//	//auto& editorCam = 
//	//	myEditor.GetEditorCamMovement().GetGameObject()->GetComponent<CameraComponentNew>()->GetRendererCamera();
//
//	//const CU::Matrix4f screenToWorld = editorCam.GetProjectionMatrix().Inverse() * editorCam.GetTransform().ToMatrix();
//
//	//Vec4f nearVector;
//	//nearVector.x = normMousePos.x;
//	//nearVector.y = normMousePos.y;
//	//nearVector.z = 1.f;
//	//nearVector.w = 1.f;
//	//nearVector = nearVector * screenToWorld;
//
//	//if (nearVector.w != 0)
//	//{
//	//	nearVector.x /= nearVector.w;
//	//	nearVector.y /= nearVector.w;
//	//	nearVector.z /= nearVector.w;
//	//}
//
//	//const auto camPos = editorCam.GetTransform().GetPosition();
//
//	//{
//	//	GDebugDrawer->DrawSphere3D(
//	//		DebugDrawFlags::AI,
//	//		Vec3f(nearVector.x, nearVector.y, nearVector.z) - camPos,
//	//		25.f,
//	//		5.f,
//	//		{ 0.f, 1.f, 0.f, 1.f });
//	//}
//
//	//return (Vec3f(nearVector.x, nearVector.y, nearVector.z) - camPos).GetNormalized();
//	////return camPos.FromTo(Vec3f(nearVector.x, nearVector.y, nearVector.z));
//}