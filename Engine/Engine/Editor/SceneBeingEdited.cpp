#include "pch.h"
#include "SceneBeingEdited.h"

#include "imgui_internal.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/Scene/FolderScene.h"
#include "Engine/Renderer/Animation/AssimpUtils.h"
#include "Editor.h"
#include "Engine/Utils/Utils.h"

Engine::SceneBeingEdited::SceneBeingEdited(Editor& aEditor)
	: myEditor(aEditor)
{
}

Engine::SceneBeingEdited::~SceneBeingEdited()
{
}

void Engine::SceneBeingEdited::InitFromJson(const nlohmann::json& aJson)
{
	if (aJson.contains("DrawGrid"))
	{
		myDrawGrid = aJson["DrawGrid"];
	}

	if (aJson.contains("GridY"))
	{
		myGridY = aJson["GridY"];
	}

	if (aJson.contains("Snap"))
	{
		mySnap[0] = aJson["Snap"][0];
		mySnap[1] = aJson["Snap"][1];
		mySnap[2] = aJson["Snap"][2];
	}
}

nlohmann::json Engine::SceneBeingEdited::ToJson() const
{
	nlohmann::json j;

	j["DrawGrid"] = myDrawGrid;
	j["GridY"] = myGridY;

	j["Snap"][0] = mySnap[0];
	j["Snap"][1] = mySnap[1];
	j["Snap"][2] = mySnap[2];

	return j;
}

void Engine::SceneBeingEdited::Reset()
{
	myScene = nullptr;
	mySelection = {};
	myStartTransforms = {};
	myTransformedGameObjects = {};
	myGizmosTransform = {};
	myStartTransform = {};
	myStartTransformInverse = {};
}

Engine::FolderScene* Engine::SceneBeingEdited::GetScene()
{
	return myScene.get();
}

const Engine::FolderScene* Engine::SceneBeingEdited::GetScene() const
{
	return myScene.get();
}

void Engine::SceneBeingEdited::SetScene(Shared<FolderScene> aScene)
{
	myScene = aScene;
}

Engine::Selection& Engine::SceneBeingEdited::GetSelection()
{
	return mySelection;
}

void Engine::SceneBeingEdited::Update()
{
	myStopDrawingGizmos = false;

	if (myDrawGrid)
	{
		DrawGrid();
	}
}

void Engine::SceneBeingEdited::EndFrame()
{
	mySelection.Update();
}

void Engine::SceneBeingEdited::UpdateGizmos(
	const float aWindowX,
	const float aWindowY,
	const float aWindowWidth,
	const float aWindowHeight)
{
	// ugly edge case because imguizmo does not support multiple gizmos at same time
	if (myStopDrawingGizmos)
	{
		return;
	}

	if (ImGuizmo::IsUsing())
	{
		myStopDrawingGizmos = true;
	}

	if (mySelection.GetCount() <= 0)
	{
		return;
	}

	ImGui::SetItemDefaultFocus();

	ImGuizmo::SetOrthographic(false);

	// Draws it to the current window!
	ImGuizmo::SetDrawlist(ImGui::GetCurrentWindow()->DrawList);

	ImGuizmo::SetRect(aWindowX, aWindowY, aWindowWidth, aWindowHeight);

	const Mat4f projection = myScene->GetMainCamera().GetProjectionMatrix();
	const Mat4f view = Mat4f::GetFastInverse(myScene->GetMainCamera().GetTransform().ToMatrix());

	if (ImGuizmo::IsUsing() == false)
	{
		// Since we are loading from Unreal and need to support hierarchy in there
		// we have a special GetPosition() in our Transform class
		// Therefore, using gizmos in local space causes issues when e.g. we rotated a parent
		// and moving a child in its local X axis
		// due to the rotation, it does not move directly its local X due to the gizmos being also rotated
		// the solution is either to use OPERATION::WORLD in gizmos or
		// use OPERATION::LOCAL but offset the result yourself with the inverse of the start transform rotation and scale, but not position
		// cringe as fuck tho
		myStartGizmosTransform =
			Mat4f::CreateScale(mySelection.CalculateCenterTransform().GetScaleLocal()) *
			mySelection.CalculateCenterTransform().GetRotationLocal().ToMatrix() *
			Mat4f::CreateTranslation({});
		// myStartGizmosTransform = { };

		myGizmosTransform = mySelection.CalculateCenterTransform().GetMatrix();
	}

	Mat4f deltaMat;

	ImGuizmo::Manipulate(view.GetPointer(),
		projection.GetPointer(),
		static_cast<ImGuizmo::OPERATION>(myGizmoType),
		ImGuizmo::MODE::LOCAL,
		myGizmosTransform.GetPointer(),
		deltaMat.GetPointer(),
		mySnap);

	// ImGuizmo::AllowAxisFlip(false);

	if (ImGuizmo::IsOver() && ImGui::GetIO().MouseClicked[0])
	{
		myStartTransforms.clear();
		myStartTransformsInv.clear();
		myEndTransforms.clear();
		myTransformedGameObjects.clear();

		for (int i = 0; i < mySelection.GetCount(); ++i)
		{
			//myStartTransforms.push_back(mySelection.GetAt(i).GetTransformWorld());
			auto gameObject = mySelection.GetAt(i);

			if (!gameObject)
			{
				continue;
			}

			myTransformedGameObjects.push_back(mySelection.GetAtWeak(i).lock());

			//myStartTransforms.push_back(gameObject->GetTransform().GetMatrix());

			auto m = Mat4f::CreateScale(gameObject->GetTransform().GetScaleLocal())
				* gameObject->GetTransform().GetRotationLocal().ToMatrix()
				* Mat4f::CreateTranslation(gameObject->GetTransform().GetPositionLocal());

			myStartTransforms.push_back(m);
			myEndTransforms.push_back(m);

			auto m2 = Mat4f::CreateScale(gameObject->GetTransform().GetScale()) *
				gameObject->GetTransform().GetRotation().ToMatrix() *
				Mat4f::CreateTranslation({});

			myStartTransformsInv.push_back(m2);

			//myStartTransforms.push_back(gameObject->GetTransform().GetMatrix());
		}

		myStartTransform = myGizmosTransform;

		// Cannot use FastInverse() because scaling is involved

		//myStartTransformInverse = Mat4f::GetFastInverse(myStartTransform);

		auto mm = CUMatrixToAiMatrix(myStartTransform).Inverse();
		auto mm2 = AiMatrixToCUMatrix(mm);
		//
		myStartTransformInverse = mm2;
		//myStartTransformInverse = myStartTransform.Inverse();

		myStartedUsingGizmos = true;
	}

	// Vec3f matrixTranslation, matrixRotation, matrixScale;
	// ImGuizmo::DecomposeMatrixToComponents(
	// 	deltaMat.GetPointer(),
	// 	&matrixTranslation.x,
	// 	&matrixRotation.x,
	// 	&matrixScale.x);

	if (ImGuizmo::IsUsing())
	{
		if (mySelection.GetCount() == myStartTransforms.size())
		{
			for (int i = 0; i < mySelection.GetCount(); ++i)
			{
				myEndTransforms[i] = myStartTransforms[i] * myStartTransformInverse * (myGizmosTransform/* * myStartTransform * myStartTransformsInv[i].Inverse()*/) /** myStartTransformsInv[i].Inverse()*//** myStartGizmosTransform.Inverse()*/;

				Vec3f pos;
				Quatf rot;
				Vec3f scale;
				myEndTransforms[i].Decompose(pos, rot, scale);

				const auto& gameObject = mySelection.GetAt(i);

				if (!gameObject)
				{
					continue;
				}

				{
					auto& t = gameObject->GetTransform();

					/*
						The problem explanation:

						We are using OPERATION::LOCAL which makes the gizmos drawn and move in
						the models local space. This is correct.

						The issue is when we rotate a parent, and then move a child using gizmos
						in its local X, the rotation gets calculated in and we ourselfs also rotate
						the delta between its parent with the rotation, therefore
						it becomes double because gizmos does it as well

						The solution is to use OPERATION::WORLD but then it does not draw the gizmos correct
						but the results of the gizmos are correct

						therefore the most correct solution is to either convert the gizmos result back
						to "world space" (which is the result given if it had OPERATION::WORLD)
						or to remove our own rotation calculations in Transform::GetPosition()
						But that stuff are used by the unreal importer, them we'd have to bake the unreal
						stuff into its world position directly and cannot have hierarchy anymore.
					*/

					switch (static_cast<ImGuizmo::OPERATION>(myGizmoType))
					{
					case ImGuizmo::TRANSLATE:
					{
						t.SetPositionLocal(pos);
					} break;
					case ImGuizmo::ROTATE:
						t.SetRotationLocal(rot);
						break;
					case ImGuizmo::SCALE:
						t.SetScaleLocal(scale);
						break;
					default:
						break;
					}
				}
			}
		}
	}

	if (myStartedUsingGizmos && !ImGuizmo::IsUsing())
	{
		myStartedUsingGizmos = false;

		// commands

		// TODO: Add the command when fixed the child gizmos transform problem
		for (size_t i = 0; i < myTransformedGameObjects.size(); ++i)
		{
			auto cmd = myEditor.CreateCommand_TransformGameObject(
				myTransformedGameObjects[i],
				myStartTransforms[i],
				myEndTransforms[i]);
			myEditor.PushCommand(std::move(cmd));
		}

		myEditor.FinishCommand();
	}
}

void Engine::SceneBeingEdited::DrawGizmos(
	const GizmosFlags aFlags,
	const float aWindowX,
	const float aWindowY,
	const float aWindowWidth,
	const float aWindowHeight,
	Mat4f& aMatrixOut) const
{
	if (myStopDrawingGizmos)
	{
		return;
	}

	FolderScene* scene = myEditor.GetActiveScene();

	ImGui::SetItemDefaultFocus();
	ImGuizmo::SetOrthographic(false);
	// Draws it to the current window!
	ImGuizmo::SetDrawlist(ImGui::GetCurrentWindow()->DrawList);
	ImGuizmo::SetRect(aWindowX, aWindowY, aWindowWidth, aWindowHeight);

	const Mat4f projection = scene->GetMainCamera().GetProjectionMatrix();
	const Mat4f view = Mat4f::GetFastInverse(scene->GetMainCamera().GetTransform().ToMatrix());

	if (ImGuizmo::IsUsing())
	{
		myStopDrawingGizmos = true;
	}

	Mat4f deltaMat;

	ImGuizmo::OPERATION gizmosType = ImGuizmo::OPERATION::TRANSLATE;

	if (aFlags & GizmosFlags_Translation)
	{
		gizmosType = gizmosType | ImGuizmo::OPERATION::TRANSLATE;
	}
	else if (aFlags & GizmosFlags_Rotation)
	{
		gizmosType = gizmosType | ImGuizmo::OPERATION::ROTATE;
	}
	else if (aFlags & GizmosFlags_Scaling)
	{
		gizmosType = gizmosType | ImGuizmo::OPERATION::SCALE;
	}

	float mySnap[3] = { 0.0f, 0.0f, 0.0f };

	ImGuizmo::Manipulate(view.GetPointer(),
		projection.GetPointer(),
		static_cast<ImGuizmo::OPERATION>(gizmosType),
		ImGuizmo::MODE::LOCAL,
		aMatrixOut.GetPointer(),
		deltaMat.GetPointer(),
		mySnap);

	// ImGuizmo::AllowAxisFlip(false);

	//if (ImGuizmo::IsUsing())
	//{
	//	if (mySelection.GetCount() == myStartTransforms.size())
	//	{
	//		for (int i = 0; i < mySelection.GetCount(); ++i)
	//		{
	//			myEndTransforms[i] = myStartTransforms[i] * myStartTransformInverse * (myGizmosTransform/* * myStartTransform * myStartTransformsInv[i].Inverse()*/) /** myStartTransformsInv[i].Inverse()*//** myStartGizmosTransform.Inverse()*/;

	//			Vec3f pos;
	//			Quatf rot;
	//			Vec3f scale;
	//			myEndTransforms[i].Decompose(pos, rot, scale);

	//			const auto& gameObject = mySelection.GetAt(i);

	//			if (!gameObject)
	//			{
	//				continue;
	//			}

	//			{
	//				auto& t = gameObject->GetTransform();

	//				/*
	//					The problem explanation:

	//					We are using OPERATION::LOCAL which makes the gizmos drawn and move in
	//					the models local space. This is correct.

	//					The issue is when we rotate a parent, and then move a child using gizmos
	//					in its local X, the rotation gets calculated in and we ourselfs also rotate
	//					the delta between its parent with the rotation, therefore
	//					it becomes double because gizmos does it as well

	//					The solution is to use OPERATION::WORLD but then it does not draw the gizmos correct
	//					but the results of the gizmos are correct

	//					therefore the most correct solution is to either convert the gizmos result back
	//					to "world space" (which is the result given if it had OPERATION::WORLD)
	//					or to remove our own rotation calculations in Transform::GetPosition()
	//					But that stuff are used by the unreal importer, them we'd have to bake the unreal
	//					stuff into its world position directly and cannot have hierarchy anymore.
	//				*/

	//				switch (static_cast<ImGuizmo::OPERATION>(myGizmoType))
	//				{
	//				case ImGuizmo::TRANSLATE:
	//				{
	//					t.SetPositionLocal(pos);
	//				} break;
	//				case ImGuizmo::ROTATE:
	//					t.SetRotationLocal(rot);
	//					break;
	//				case ImGuizmo::SCALE:
	//					t.SetScaleLocal(scale);
	//					break;
	//				default:
	//					break;
	//				}
	//			}
	//		}
	//	}
	//}

	//if (myStartedUsingGizmos && !ImGuizmo::IsUsing())
	//{
	//	myStartedUsingGizmos = false;

	//	// commands

	//	// TODO: Add the command when fixed the child gizmos transform problem
	//	for (size_t i = 0; i < myTransformedGameObjects.size(); ++i)
	//	{
	//		auto cmd = myEditor.CreateCommand_TransformGameObject(
	//			myTransformedGameObjects[i],
	//			myStartTransforms[i],
	//			myEndTransforms[i]);
	//		myEditor.PushCommand(std::move(cmd));
	//	}

	//	myEditor.FinishCommand();
	//}
}

void Engine::SceneBeingEdited::UpdateGizmosKeyBinds()
{
	if (ImGui::IsKeyPressed('W'))
	{
		myGizmoType = ImGuizmo::OPERATION::TRANSLATE;
	}
	if (ImGui::IsKeyPressed('E'))
	{
		myGizmoType = ImGuizmo::OPERATION::ROTATE;
	}
	if (ImGui::IsKeyPressed('R'))
	{
		myGizmoType = ImGuizmo::OPERATION::SCALE;
	}
}

void Engine::SceneBeingEdited::SetGridY(const float aY)
{
	myGridY = aY;
}

void Engine::SceneBeingEdited::SetSnapping(const float aSnapping)
{
	mySnap[0] = aSnapping;
	mySnap[1] = aSnapping;
	mySnap[2] = aSnapping;
}

float Engine::SceneBeingEdited::GetSnapping() const
{
	return mySnap[0];
}

void Engine::SceneBeingEdited::DrawGrid()
{
	Vec3f origin;

	origin.x = myScene->GetMainCameraGameObject()->GetTransform().GetPosition().x;
	origin.z = myScene->GetMainCameraGameObject()->GetTransform().GetPosition().z;
	origin.y = myGridY;

	Utils::ComputeSnap(&origin.z, 100.f);
	Utils::ComputeSnap(&origin.x, 100.f);

	int grid_line_count = 50;

	Vec4f color(0.5f, 0.5f, 0.5f, 1.f);

	const auto createLine =
		[](Vec3f pos1, Vec3f pos2)->std::array<Vec3f, 2> {
		std::array<Vec3f, 2> line = {};

		line[0] = pos1;

		line[1] = pos2;

		return line;
	};

	const Vec3f xAxisScale(50.f * 50.f, 0.f, 0.f);
	const Vec3f yAxisScale(0.f, 0.f, 50.f * 50.f);

	for (int i = 0; i <= grid_line_count; ++i)
	{
		const float percent = float(i) / float(grid_line_count);

		const float valueBetween1AndMinus1 = (percent * 2.0f) - 1.0f;

		Vec3f scale = xAxisScale * valueBetween1AndMinus1 + origin;

		const auto linePoints = createLine(scale - yAxisScale, scale + yAxisScale);

		myScene->GetRendererScene().GetDebugDrawer().DrawLine3D(
			DebugDrawFlags::Always,
			linePoints[0],
			linePoints[1],
			0.f,
			color);
	}

	for (int i = 0; i <= grid_line_count; ++i)
	{
		const float percent = float(i) / float(grid_line_count);

		const float valueBetween1AndMinus1 = (percent * 2.0f) - 1.0f;

		const Vec3f scale = yAxisScale * valueBetween1AndMinus1 + origin;

		const auto linePoints = createLine(scale - xAxisScale, scale + xAxisScale);

		myScene->GetRendererScene().GetDebugDrawer().DrawLine3D(
			DebugDrawFlags::Always,
			linePoints[0],
			linePoints[1],
			0.f,
			color);
	}
}
