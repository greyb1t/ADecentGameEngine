#include "pch.h"
#include "BlendTree2DEditor.h"
#include "Engine/Animation/Blending/BlendTree2D.h"
#include "imgui_internal.h"
#include "Engine/Editor/ImGuiHelper.h"
#include "Engine/Editor/Editor.h"
#include "Engine/Animation/NormalAnimation.h"
#include "Engine/Editor/DragDropConstants.h"
#include "Engine/Editor/FileTypes.h"
#include "Engine/ResourceManagement/Resources/AnimationClipResource.h"
#include "Engine/ResourceManagement/Resources/ModelResource.h"
#include "Engine/Editor/EditorWindows/AnimationNodeEditorWindow.h"
#include "Engine/Animation/State/AnimationMachine.h"
#include "Engine/Editor/EditorWindows/AssetBrowser/AssetBrowserWindow.h"
#include "Engine/Editor/EditorWindows/Inspector/InspectorWindow.h"
#include "Engine/ResourceManagement/Resources/TextureResource.h"
#include "Engine/Renderer/Texture/Texture2D.h"
#include "Engine/Engine.h"
#include "Engine/Renderer/GraphicsEngine.h"
#include "Engine/Renderer/Texture/FullscreenTextureFactory.h"
#include "Engine/Utils/Utils.h"

const ImU32 CircleColor = IM_COL32(255, 255, 255, 160);
const ImU32 CircleColorHover = IM_COL32(255, 255, 255, 255);
const ImU32 CircleColorHoverAndSelected = IM_COL32(255, 255, 255, 255);
const ImU32 CircleColorSelected = IM_COL32(255, 255, 255, 255);
const ImU32 CircleColorDragging = IM_COL32(0, 0, 255, 255);
const ImU32 CircleColorBlendValue = IM_COL32(25, 186, 255, 255);
const ImU32 CircleColorBlendValueHoverDragging = IM_COL32(95, 236, 255, 255);
const ImU32 CircleWeightColor = IM_COL32(185, 20, 20, 255);
const ImU32 GridLineColor = IM_COL32(255, 255, 255, 50);
const ImU32 NodeTextColor = IM_COL32(255, 255, 255, 80);
const ImU32 NodeTextColorSelected = IM_COL32(255, 255, 255, 255);
const ImU32 CircleAroundNodeCircle = IM_COL32(255, 255, 255, 40);
const ImU32 CircleAroundNodeCircleSelected = IM_COL32(255, 255, 255, 255);
const ImU32 TriangleEdgeColor = IM_COL32(255, 255, 255, 100);

Engine::BlendTree2DEditor::BlendTree2DEditor(AnimationNodeEditorWindow& aAnimEditor)
	: myAnimEditor(aAnimEditor)
{
}

void Engine::BlendTree2DEditor::Draw(BlendTree2D& aBlendTree)
{
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1f, 0.1f, 0.1f, 1));

	auto pos = ImGui::GetCursorPos();
	pos.x += myMinPadding.x;
	pos.y += myMinPadding.y;

	ImGui::Dummy(ImVec2(myMinPadding.x, myMinPadding.y));

	ImGui::Checkbox("Snapping", &mySnapping);

	ImGui::Dummy(ImVec2(myMinPadding.x, myMinPadding.y));

	ImGui::SameLine();

	if (ImGui::BeginChild(
		"2dblendtreechild",
		ImVec2(myWindowSize.x, myWindowSize.y),
		true,
		ImGuiWindowFlags_NoScrollbar))
	{
		DrawGrid();
		DrawNodeTriangulation(aBlendTree);

		DrawLiveInstanceBlendValue(aBlendTree);
		DrawNodeCircles(aBlendTree);

		UpdateInputs(aBlendTree);
	}
	ImGui::EndChild();
	Vec2f padding(16.f, 16.f);
	myChildMin = ImVec2ToVec2(ImGui::GetItemRectMin()) + padding;
	myChildMax = ImVec2ToVec2(ImGui::GetItemRectMax()) - padding;
	ImGui::PopStyleColor();

	ImGui::Separator();

	if (mySelectedNodeIndex != -1)
	{
		if (mySelectedNodeIndex < aBlendTree.myNodes.size())
		{
			auto& node = aBlendTree.myNodes[mySelectedNodeIndex];

			static ImDrawListSplitter lol;

			// the table already uses a splitter interally, and imgui does not support
			// recursive splitting, therefore we use our own instance to fix that
			lol.Split(ImGui::GetWindowDrawList(), 2);
			lol.SetCurrentChannel(ImGui::GetWindowDrawList(), 1);

			ImGui::BeginGroup();
			DrawNodeProperties(aBlendTree, node);
			ImGui::EndGroup();

			lol.SetCurrentChannel(ImGui::GetWindowDrawList(), 0);

			auto min = ImGui::GetItemRectMin();
			auto max = ImGui::GetItemRectMax();

			if (node.GetPlayable())
			{
				const auto& cellPadding = ImGui::GetStyle().CellPadding;

				min.x -= cellPadding.x;
				min.y -= cellPadding.y;

				max.x += cellPadding.x;
				max.y += cellPadding.y;

				auto color = ImGuiHelper::CalculateColorFromString(node.GetPlayable()->GetName());

				//ImGui::GetWindowDrawList()->AddRectFilled(
				//	min,
				//	max,
				//	ImGui::ColorConvertFloat4ToU32(color));
			}

			lol.Merge(ImGui::GetWindowDrawList());
		}
	}

#if 0
	for (int i = 0; i < aBlendTree.myNodes.size(); ++i)
	{
		auto& node = aBlendTree.myNodes[i];

		ImGui::PushID(i);

		static ImDrawListSplitter lol;

		// the table already uses a splitter interally, and imgui does not support
		// recursive splitting, therefore we use our own instance to fix that
		lol.Split(ImGui::GetWindowDrawList(), 2);
		lol.SetCurrentChannel(ImGui::GetWindowDrawList(), 1);

		ImGui::BeginGroup();
		DrawNodeProperties(aBlendTree.myNodes[i]);
		ImGui::EndGroup();

		lol.SetCurrentChannel(ImGui::GetWindowDrawList(), 0);

		auto min = ImGui::GetItemRectMin();
		auto max = ImGui::GetItemRectMax();

		if (node.GetPlayable())
		{
			const auto& cellPadding = ImGui::GetStyle().CellPadding;

			min.x -= cellPadding.x;
			min.y -= cellPadding.y;

			max.x += cellPadding.x;
			max.y += cellPadding.y;

			auto color = ImGuiHelper::CalculateColorFromString(node.GetPlayable()->GetName());

			ImGui::GetWindowDrawList()->AddRectFilled(
				min,
				max,
				ImGui::ColorConvertFloat4ToU32(color));
		}

		lol.Merge(ImGui::GetWindowDrawList());

		ImGui::PopID();

		ImGui::Spacing();
		ImGui::Spacing();
	}
#endif

	/*
	// Render the Quad to the texture, and then draw that texture through imgui
	myAnimEditor.GetEditor().GetRendererScene().Submit([&]()
		{
			auto& dx = GetEngine().GetGraphicsEngine().GetDxFramework();
			auto& context = *dx.GetContext();

			myTexture.SetAsActiveTarget();

			context.PSSetShader();
			context.VSSetShader();

			context.Draw();
		});
	*/

	// ImGui::Image();

}

void Engine::BlendTree2DEditor::Reset()
{
	mySelectedNodeIndex = -1;
}

void Engine::BlendTree2DEditor::DrawNodeCircles(BlendTree2D& aBlendTree)
{
	for (int i = 0; i < aBlendTree.myNodes.size(); ++i)
	{
		DrawNodeCircle(i, aBlendTree.myNodes[i], aBlendTree);
	}
}

void Engine::BlendTree2DEditor::DrawNodeCircle(const int aIndex, BlendNode2D& aNode, BlendTree2D& aBlendTree)
{
	ImGui::PushID(&aNode);

	const float keyCircleRadius = 10.f;

	const auto nodePos = Vec2ToImVec2(NormalizedSpaceToScreenSpace(aNode.GetPosition()));

	ImU32 keyCircleColor = CircleColor;
	ImU32 textColor = NodeTextColor;

	InvisibleSelectable("key", nodePos, ImVec2(keyCircleRadius, keyCircleRadius));

	bool isDragging = false;

	if (ImGui::IsMouseDragging(0) && ImGui::IsItemActive())
	{
		keyCircleColor = CircleColorDragging;
		isDragging = true;

		auto newPos = ScreenSpaceToNormalizedSpace(ImVec2ToVec2(ImGui::GetMousePos()));

		if (mySnapping)
		{
			Utils::ComputeSnap(&newPos.x, myGridIncrement);
			Utils::ComputeSnap(&newPos.y, myGridIncrement);
		}

		aNode.SetPosition(newPos);

		aBlendTree.RecalculateTriangles();
	}

	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
	{
		mySelectedNodeIndex = aIndex;
	}

	if (aIndex == mySelectedNodeIndex)
	{
		keyCircleColor = CircleColorSelected;
		textColor = NodeTextColorSelected;

		ImGui::GetCurrentWindow()->DrawList->AddCircle(nodePos, keyCircleRadius * 0.75f, CircleAroundNodeCircleSelected);
	}
	else
	{
		ImGui::GetCurrentWindow()->DrawList->AddCircle(nodePos, keyCircleRadius * 0.75f, CircleAroundNodeCircle);
	}

	if (ImGui::IsItemHovered() || isDragging)
	{
		keyCircleColor = CircleColorHover;

		// On right click
		ImGui::OpenPopupOnItemClick("EditNode");

		// ImGui::SetTooltip("X: %.2f, Y: %.2f", aNode.GetPosition().x, aNode.GetPosition().y);
	}

	if (ImGui::IsItemHovered() && aIndex == mySelectedNodeIndex)
	{
		keyCircleColor = CircleColorHoverAndSelected;
	}

	char buffer[50] = {};

	sprintf_s(buffer, "(%.1f, %.1f)", aNode.GetPosition().x, aNode.GetPosition().y);

	ImGui::GetWindowDrawList()->AddText(
		GetEngine().GetGraphicsEngine().ourFont16,
		16.f,
		ImGui::GetItemRectMax(),
		textColor,
		buffer);

	if (ImGui::BeginPopup("EditNode"))
	{
		DrawNodeProperties(aBlendTree, aNode);

		if (ImGui::Button("Delete"))
		{
			aBlendTree.RemoveNode(aIndex);
			aBlendTree.RecalculateTriangles();

			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	ImGui::GetCurrentWindow()->DrawList->AddCircleFilled(nodePos, keyCircleRadius * 0.5f, keyCircleColor);

	ImGui::PopID();
}

void Engine::BlendTree2DEditor::InvisibleSelectable(const void* aId, const ImVec2& aPosition, const ImVec2& aSize)
{
	auto old = ImGui::GetCursorScreenPos();

	const ImVec2 d(aPosition.x - aSize.x * 0.5f, aPosition.y - aSize.y * 0.5f);
	ImGui::SetCursorScreenPos(d);
	ImGui::PushID(aId);
	ImGui::InvisibleButton("yoo", aSize);
	ImGui::PopID();

	ImGui::SetCursorScreenPos(old);
}

Vec2f Engine::BlendTree2DEditor::NormalizedSpaceToScreenSpace(const Vec2f aPosition) const
{
	const float percentX = Math::InverseLerp(-1.f, 1.f, aPosition.x);
	const float percentY = Math::InverseLerp(-1.f, 1.f, aPosition.y);

	return Vec2f(
		Math::Lerp(myChildMin.x, myChildMax.x, percentX),
		Math::Lerp(myChildMin.y, myChildMax.y, percentY));
}

Vec2f Engine::BlendTree2DEditor::ScreenSpaceToNormalizedSpace(const Vec2f aPosition) const
{
	const float percentX = Math::InverseLerp(myChildMin.x, myChildMax.x, aPosition.x);
	const float percentY = Math::InverseLerp(myChildMin.y, myChildMax.y, aPosition.y);

	return Vec2f(
		Math::Lerp(-1.f, 1.f, percentX),
		Math::Lerp(-1.f, 1.f, percentY));
}

void Engine::BlendTree2DEditor::UpdateInputs(BlendTree2D& aBlendTree)
{
	// Handle double click to create new point
	if (ImGui::IsMouseDoubleClicked(0) && ImGui::IsWindowHovered())
	{
		ImGui::GetWindowPos();

		const auto mousePos = ImGui::GetMousePos();

		const auto pos = ScreenSpaceToNormalizedSpace(ImVec2ToVec2(mousePos));

		if (pos.x > -1.f && pos.x < 1.f && pos.y > -1.f && pos.y < 1.f)
		{
			aBlendTree.AddNode(BlendNode2D(pos, nullptr));
		}

		// const auto resultPos = WindowSpaceToCurve({ mousePos.x, mousePos.y });
		// 
		// Key key(CU::Vector2f{ resultPos.x, resultPos.y });
		// 
		// mySelectedCurve->AddKey(key);
	}
}

void Engine::BlendTree2DEditor::DrawNodeProperties(BlendTree2D& aBlendTree, BlendNode2D& aNode)
{
	AnimationClipRef clip;

	if (aNode.GetPlayable())
	{
		if (auto animation = dynamic_cast<NormalAnimation*>(aNode.GetPlayable()))
		{
			clip = animation->myClip;
		}
		else
		{
			// only supports animation atm
			assert(false);
		}
	}

	if (myAnimEditor.GetEditor().DrawReflectedResource(
		clip,
		"Animation",
		DragDropConstants::AnimationClip,
		FileType::Extension_AnimationClip))
	{
		auto animation = MakeOwned<NormalAnimation>();

		animation->myClip = clip;
		animation->myName = clip->GetPath();

		aNode.myPlayable = std::move(animation);

		AnimationMachine* machine = myAnimEditor.GetMachine();

		// If we have no first anim clip, we must set it, otherwise
		// the animation controller will never update
		if (!machine->myFirstAnimationClip)
		{
			machine->myFirstAnimationClip = clip;

			if (machine->myModel && machine->myModel->IsValid())
			{
				machine->GenerateBindPose(machine->myModel->Get());
			}
		}
	}

	ImGuiHelper::AlignedWidget("Position", Editor::ourAlignPercent);
	if (ImGui::DragFloat2("##position", &aNode.myPosition.x, 0.01f, -1.f, 1.f))
	{
		aBlendTree.RecalculateTriangles();
	}

	if (aNode.myPlayable)
	{
		if (auto anim = dynamic_cast<NormalAnimation*>(aNode.myPlayable.get()))
		{
			if (myAnimEditor.DrawAnimationProperties(anim))
			{
				// comes here if node was modified
			}
		}
	}
}

void Engine::BlendTree2DEditor::DrawGrid()
{
	for (float x = -1.f; x <= 1.f; x += myGridIncrement)
	{
		const auto startPos = NormalizedSpaceToScreenSpace(Vec2f(x, -1.f));
		const auto endPos = NormalizedSpaceToScreenSpace(Vec2f(x, 1.f));

		ImGui::GetWindowDrawList()->AddLine(
			Vec2ToImVec2(startPos),
			Vec2ToImVec2(endPos),
			GridLineColor);
	}

	for (float y = -1.f; y <= 1.f; y += myGridIncrement)
	{
		const auto startPos = NormalizedSpaceToScreenSpace(Vec2f(-1.f, y));
		const auto endPos = NormalizedSpaceToScreenSpace(Vec2f(1.f, y));

		ImGui::GetWindowDrawList()->AddLine(
			Vec2ToImVec2(startPos),
			Vec2ToImVec2(endPos),
			GridLineColor);
	}
}

void Engine::BlendTree2DEditor::DrawNodeTriangulation(BlendTree2D& aBlendTree)
{
	const auto triangulation = aBlendTree.TriangulateNodes();

	for (const auto& edge : triangulation.edges)
	{
		const auto p0 = NormalizedSpaceToScreenSpace(Vec2f(edge.p0.x, edge.p0.y));
		const auto p1 = NormalizedSpaceToScreenSpace(Vec2f(edge.p1.x, edge.p1.y));

		ImGui::GetWindowDrawList()->AddLine(
			Vec2ToImVec2(p0),
			Vec2ToImVec2(p1),
			TriangleEdgeColor,
			2.f);
	}
}

void Engine::BlendTree2DEditor::DrawLiveInstanceBlendValue(BlendTree2D& aBlendTree)
{
	if (myAnimEditor.IsEditingInstance())
	{
		if (!aBlendTree.myBlendValue) return;

		const auto& blendValue = aBlendTree.GetBlendValue();

		float u = 0.f, v = 0.f, w = 0.f;
		const auto triangle = aBlendTree.DetermineTriangleToBlendWith(u, v, w);

		ImGui::GetCurrentWindow()->DrawList->AddTriangleFilled(
			Vec2ToImVec2(NormalizedSpaceToScreenSpace(aBlendTree.myNodes[triangle->myNodeIndex1].GetPosition())),
			Vec2ToImVec2(NormalizedSpaceToScreenSpace(aBlendTree.myNodes[triangle->myNodeIndex2].GetPosition())),
			Vec2ToImVec2(NormalizedSpaceToScreenSpace(aBlendTree.myNodes[triangle->myNodeIndex3].GetPosition())),
			IM_COL32(0, 255, 0, 30));

		const float weightRadiusMultiplier = 30.f;
		ImGui::GetCurrentWindow()->DrawList->AddCircle(Vec2ToImVec2(NormalizedSpaceToScreenSpace(aBlendTree.myNodes[triangle->myNodeIndex1].GetPosition())), u * weightRadiusMultiplier, CircleWeightColor);
		ImGui::GetCurrentWindow()->DrawList->AddCircle(Vec2ToImVec2(NormalizedSpaceToScreenSpace(aBlendTree.myNodes[triangle->myNodeIndex2].GetPosition())), v * weightRadiusMultiplier, CircleWeightColor);
		ImGui::GetCurrentWindow()->DrawList->AddCircle(Vec2ToImVec2(NormalizedSpaceToScreenSpace(aBlendTree.myNodes[triangle->myNodeIndex3].GetPosition())), w * weightRadiusMultiplier, CircleWeightColor);

		const auto p = NormalizedSpaceToScreenSpace(blendValue);

		const float keyCircleRadius = 16.f;
		InvisibleSelectable("key", Vec2ToImVec2(p), ImVec2(keyCircleRadius, keyCircleRadius));

		bool isDragging = ImGui::IsMouseDragging(0) && ImGui::IsItemActive();

		ImU32 circleColor = CircleColorBlendValue;

		if (ImGui::IsItemHovered() || isDragging)
		{
			circleColor = CircleColorBlendValueHoverDragging;

			ImGui::SetTooltip("X: %.2f, Y: %.2f", blendValue.x, blendValue.y);
		}

		ImGui::GetCurrentWindow()->DrawList->AddCircleFilled(
			Vec2ToImVec2(p),
			keyCircleRadius * 0.5f,
			circleColor);

		if (isDragging)
		{
			auto newPos = ScreenSpaceToNormalizedSpace(ImVec2ToVec2(ImGui::GetMousePos()));

			*aBlendTree.myBlendValue = newPos;
		}
	}
}
