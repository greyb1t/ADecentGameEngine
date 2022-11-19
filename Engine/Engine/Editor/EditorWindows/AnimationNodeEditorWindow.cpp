#include "pch.h"
#include "AnimationNodeEditorWindow.h"

#include <imgui_node_editor.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <utility>

#include "Engine/Engine.h"
#include "Engine/ResourceManagement/Resources/TextureResource.h"
#include "Engine/Shortcuts.h"
#include "Engine/Animation/NormalAnimation.h"
#include "Engine/Animation/Playable.h"
#include "Engine/Animation/State/AnimationMachine.h"
#include "Engine/Animation/State/AnimationMachineLayer.h"
#include "Engine/Animation/Transitions/FadeTransition.h"
#include "Engine/Animation/State/AnimationState.h"
#include "Engine/Editor/Editor.h"
#include "Engine/Editor/ImGuiHelper.h"
#include "Engine/Renderer/Texture/Texture2D.h"

#include "Engine/GraphManager/BlueprintUtilities/Include/ax/Builders.h"
#include "Engine/GraphManager/BlueprintUtilities/Include/ax/Widgets.h"
#include "Engine/ResourceManagement/Resources/AnimationClipResource.h"

#include "Engine/Editor/EditorWindows/AssetBrowser/AssetBrowserWindow.h"
#include "Engine/Editor/EditorWindows/Inspector/InspectorWindow.h"
#include "Engine/Renderer/Animation/AnimationClip.h"
#include "Engine/ResourceManagement/Resources/AnimationStateMachineResource.h"
#include "Engine/Editor/DragDropConstants.h"
#include "Engine/Editor/FileTypes.h"
#include "Engine/Animation/Blending/BlendTree1D.h"
#include "Engine/Animation/Blending/BlendTree2D.h"
#include "Engine/Animation/Blending/BlendNode1D.h"
#include "Engine/Renderer/GraphicsEngine.h"
#include "Engine/Animation/AnimationEvent.h"
#include "Engine/ResourceManagement/Resources/ModelResource.h"
#include "Engine/Renderer/Model/Model.h"
#include "Engine/Animation/Visitor/FindAnimationClipsVisitor.h"
#include "AnimationEditor/BlendTree2DEditor.h"

namespace ed = ax::NodeEditor;
namespace util = ax::NodeEditor::Utilities;

using namespace ax;

using ax::Widgets::IconType;

static ImRect ImRect_Expanded(const ImRect& rect, float x, float y)
{
	auto result = rect;
	result.Min.x -= x;
	result.Min.y -= y;
	result.Max.x += x;
	result.Max.y += y;
	return result;
}

static inline ImRect ImGui_GetItemRect()
{
	return ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
}

static bool Splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size = -1.0f)
{
	using namespace ImGui;
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	ImGuiID id = window->GetID("##Splitter");
	ImRect bb;
	bb.Min = window->DC.CursorPos + (split_vertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1));
	bb.Max = bb.Min + CalcItemSize(split_vertically ? ImVec2(thickness, splitter_long_axis_size) : ImVec2(splitter_long_axis_size, thickness), 0.0f, 0.0f);
	return SplitterBehavior(bb, id, split_vertically ? ImGuiAxis_X : ImGuiAxis_Y, size1, size2, min_size1, min_size2, 0.0f);
}

Engine::AnimationNodeEditorWindow::AnimationNodeEditorWindow(Editor& aEditor)
	: EditorWindow(aEditor, Flags::CanBeClosed | Flags::CanBeUnsaved),
	myBlendTree2DChildWindow(*this)
{
}

Engine::AnimationNodeEditorWindow::~AnimationNodeEditorWindow()
{
	Application_Finalize();
}

bool Engine::AnimationNodeEditorWindow::InitExistingMachine(AnimationMachine* aMachine, const Path& aSavePath)
{
	myMachine = aMachine;
	mySavePath = aSavePath;

	mySave = GResourceManager->CreateRef<TextureResource>(
		"Assets/Engine/Editor/Graph/ic_save_white_24dp.dds");
	mySave->Load();

	myRestore = GResourceManager->CreateRef<TextureResource>(
		"Assets/Engine/Editor/Graph/ic_restore_white_24dp.dds");
	myRestore->Load();

	myBlueprintBackground = GetEngine().GetResourceManager().CreateRef<TextureResource>(
		"Assets/Engine/Editor/Graph/BlueprintBackground.dds");
	myBlueprintBackground->Load();

	mySaveIconWidth = mySave->Get().GetImageSize().x;
	mySaveIconHeight = mySave->Get().GetImageSize().y;

	myRestoreIconWidth = myRestore->Get().GetImageSize().x;
	myRestoreIconHeight = myRestore->Get().GetImageSize().y;

	myHeaderBackgroundWidth = myBlueprintBackground->Get().GetImageSize().x;
	myHeaderBackgroundHeight = myBlueprintBackground->Get().GetImageSize().y;

	Application_Initialize();

	return true;
}

bool Engine::AnimationNodeEditorWindow::InitNewMachineInstance(
	Owned<AnimationMachine> aMachine,
	const Path& aSavePath)
{
	myMachineFileHolder = std::move(aMachine);

	return InitExistingMachine(myMachineFileHolder.get(), aSavePath);
}

ImGuiWindowFlags Engine::AnimationNodeEditorWindow::GetWindowFlags() const
{
	return ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse |
		ImGuiWindowFlags_MenuBar;
}

void Engine::AnimationNodeEditorWindow::Draw(const float aDeltaTime)
{
	ZoneNamedN(zone1, "AnimationNodeEditorWindow::Update", true);

	// ImGui::SetNextWindowSize({ 800, 800 }, ImGuiCond_Once);
	// ImGui::Begin("Animation Machine Editor", &open, flags);
	// 
	// if (!open)
	// {
	// 	myEditor.CloseExtraWindow(GetName());
	// 	ImGui::End();
	// 	// Must return because we de-allocate ourself
	// 	return;
	// }

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Save"))
			{
				Save();
			}

			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}

	Application_Frame();

	// ImGui::End();
}

void Engine::AnimationNodeEditorWindow::OnEditorChangeMode(const EditorMode aMode)
{
	if (aMode == EditorMode::Editing)
	{
		// Must close this window if we go into Editing mode and we have an instance open
		if (!IsEditingFile())
		{
			myEditor.CloseExtraWindow(GetName());
		}
	}
}

Engine::AnimationMachine* Engine::AnimationNodeEditorWindow::GetMachine()
{
	return myMachine;
}

void Engine::AnimationNodeEditorWindow::Application_Frame()
{
	ed::SetCurrentEditor(myNodeEditor);

	bool changedLayer = false;

	// If we changed layer, focus on the new nodes
	if (myNextSelectedLayer)
	{
		mySelectedLayer = myNextSelectedLayer;
		myNextSelectedLayer = nullptr;
		changedLayer = true;
	}

	ed::NodeId selectedNode;
	ed::GetSelectedNodes(&selectedNode, 1);

	ed::LinkId selectedLink;
	ed::GetSelectedLinks(&selectedLink, 1);

	// If changed selection, focus on the inspector tab
	if (myPrevSelectedLink != selectedLink || myPrevSelectedNode != selectedNode)
	{
		myPrevSelectedNode = selectedNode;
		myPrevSelectedLink = selectedLink;

		myShowInspectionTab = true;
	}

	ReCreateNodesOfStates();
	ReCreateStateTransitionsLinks();

	UpdateTouch();

	static ed::NodeId contextNodeId = 0;
	static ed::LinkId contextLinkId = 0;
	static ed::PinId contextPinId = 0;
	static bool createNewNode = false;
	static Pin* newNodeLinkPin = nullptr;
	static Pin* newLinkPin = nullptr;

	static float leftPaneWidth = 450.0f;
	static float rightPaneWidth = 800.0f;

	const float splitterThiccness = 6.f;

	Splitter(true, splitterThiccness, &leftPaneWidth, &rightPaneWidth, 50.0f, 50.0f);

	const float leftWidth = std::min(leftPaneWidth, ImGui::GetContentRegionAvailWidth());

	ShowLeftPane(leftWidth - splitterThiccness);

	ImGui::SameLine(0.0f, 12.0f);

	// This is a fix to the issue with IMGUI disappearing completely when the
	// window is small enough on the width. Simply to do
	// draw the node editor if window is too small in width
	if (leftWidth < 100.f)
	{
		return;
	}

	// This is a fix to the issue with IMGUI disappearing completely when the
	// window is small enough on the width. Simply to do
	// draw the node editor if window is too small in width
	if (ImGui::GetWindowSize().x < (leftPaneWidth + 50.f))
	{
		return;
	}

	ed::Begin("Node editor");
	{
		auto cursorTopLeft = ImGui::GetCursorScreenPos();

		//util::BlueprintNodeBuilder builder(
		//	myHeaderBackground,
		//	myHeaderBackgroundWidth,
		//	myHeaderBackgroundHeight);

		for (auto& node : myNodes)
		{
			if (node->Type != NodeType::State)
				continue;

			// Just bcuz we need to save pos
			node->myPosition = ed::GetNodePosition(node->ID);

			const float rounding = 5.0f;
			const float padding = 12.0f;

			const auto pinBackground = ed::GetStyle().Colors[ed::StyleColor_NodeBg];

			const auto nodeState = static_cast<AnimationState*>(node);

			bool isActiveState = false;

			// Draw a yellow border around the active state
			for (const auto& layer : myMachine->myLayers)
			{
				if (nodeState == layer->myActiveState)
				{
					isActiveState = true;

					break;
				}
			}

			if (isActiveState)
			{
				ed::PushStyleColor(ed::StyleColor_NodeBorder, ImColor(255, 242, 0, 255));
				ed::PushStyleVar(ed::StyleVar_NodeBorderWidth, 1.f);
				ed::PushStyleColor(ed::StyleColor_NodeBg, ImColor(118, 118, 118, 200));
			}
			else
			{
				ed::PushStyleColor(ed::StyleColor_NodeBorder, ImColor(32, 32, 32, 200));
				ed::PushStyleVar(ed::StyleVar_NodeBorderWidth, 1.f);
				ed::PushStyleColor(ed::StyleColor_NodeBg, ImColor(108, 108, 108, 200));
			}

			ed::PushStyleColor(ed::StyleColor_PinRect, ImColor(60, 180, 255, 150));
			ed::PushStyleColor(ed::StyleColor_PinRectBorder, ImColor(60, 180, 255, 150));

			// jag glömmer poppa dessas, frame drops

			ed::PushStyleVar(ed::StyleVar_NodePadding, ImVec4(0, 0, 0, 0));
			ed::PushStyleVar(ed::StyleVar_NodeRounding, rounding);
			ed::PushStyleVar(ed::StyleVar_SourceDirection, ImVec2(0.0f, 1.0f));
			ed::PushStyleVar(ed::StyleVar_TargetDirection, ImVec2(0.0f, -1.0f));
			ed::PushStyleVar(ed::StyleVar_LinkStrength, 0.0f);
			ed::PushStyleVar(ed::StyleVar_PinBorderWidth, 1.0f);
			ed::PushStyleVar(ed::StyleVar_PinRadius, 5.0f);

			ed::BeginNode(node->ID);

			ImGui::BeginVertical(node->ID.AsPointer());
			ImGui::BeginHorizontal("inputs");
			ImGui::Spring(0, padding * 2);

			bool hasBothInputOutput = !node->Inputs.empty() && !node->Outputs.empty();

			ImRect inputsRect;
			int inputAlpha = 200;
			if (!node->Inputs.empty())
			{
				auto& pin = node->Inputs[0];
				ImGui::Dummy(ImVec2(0, padding));
				ImGui::Spring(1, 0);
				inputsRect = ImGui_GetItemRect();

				inputsRect.Min.x -= 20.f;
				inputsRect.Max.x -= 80.f;

				// ed::PinRect(inputsRect.GetTL() + ImVec2(-20, 0.f), inputsRect.GetBR() + ImVec2(-70.f, 0.f));

				ed::PushStyleVar(ed::StyleVar_PinArrowSize, 20.0f);
				ed::PushStyleVar(ed::StyleVar_PinArrowWidth, 30.0f);
				ed::PushStyleVar(ed::StyleVar_PinCorners, 12);
				ed::BeginPin(pin.ID, ed::PinKind::Input);
				ed::PinPivotRect(inputsRect.GetTL(), inputsRect.GetBR());
				ed::PinRect(inputsRect.GetTL(), inputsRect.GetBR());
				//ed::PinPivotScale(ImVec2(1.5, 1));
				// ed::PinPivotAlignment(ImVec2(0.5f, 0.5f));
				ed::PinPivotSize(ImVec2(140, 50));
				//ed::PinPivotScale(ImVec2(20,20));
				/*
				void PinPivotSize(const ImVec2& size);
				void PinPivotScale(const ImVec2& scale);
				void PinPivotAlignment(const ImVec2& alignment);
				*/
				ed::EndPin();
				ed::PopStyleVar(3);

				if (newLinkPin && !CanCreateLink(newLinkPin, &pin) && &pin != newLinkPin)
					inputAlpha = (int)(255 * ImGui::GetStyle().Alpha * (48.0f / 255.0f));
			}
			else
				ImGui::Dummy(ImVec2(0, padding));

			ImGui::Spring(0, padding * 2);
			ImGui::EndHorizontal();

			ImGui::BeginHorizontal("content_frame");
			ImGui::Spring(1, padding);

			ImGui::BeginVertical("content", ImVec2(0.0f, 0.0f));
			ImGui::Dummy(ImVec2(160, 0));
			ImGui::Spring(1);
			ImGui::TextUnformatted(node->Name.c_str());
			ImGui::Spring(1);
			ImGui::EndVertical();
			auto contentRect = ImGui_GetItemRect();

			ImGui::Spring(1, padding);
			ImGui::EndHorizontal();

			ImRect content2Rect;

			{
				ImGui::BeginHorizontal("content_frame2");
				ImGui::Spring(1, padding);

				ImGui::BeginVertical("content2", ImVec2(0.0f, 0.0f));
				ImGui::Dummy(ImVec2(160, 10));
				//ImGui::Spring(1);
				// ImGui::TextUnformatted(node->Name.c_str());
				//ImGui::Spring(1);
				ImGui::EndVertical();
				content2Rect = ImGui_GetItemRect();

				ImGui::Spring(1, padding);
				ImGui::EndHorizontal();
			}

			ImGui::BeginHorizontal("outputs");
			ImGui::Spring(0, padding * 2);

			ImRect outputsRect;
			int outputAlpha = 200;
			if (!node->Outputs.empty())
			{
				auto& pin = node->Outputs[0];
				ImGui::Dummy(ImVec2(0, padding));
				ImGui::Spring(1, 0);
				outputsRect = ImGui_GetItemRect();

				outputsRect.Min.x += 80.f;
				outputsRect.Max.x += 20.f;

				ed::PushStyleVar(ed::StyleVar_PinCorners, 3);
				ed::BeginPin(pin.ID, ed::PinKind::Output);
				//ed::PinPivotRect(outputsRect.GetTL() + ImVec2(0, -50), outputsRect.GetBR() + ImVec2(40, 0));
				////ed::PinRect(outputsRect.GetTL(), outputsRect.GetBR());
				//ed::PinRect(outputsRect.GetTL(), outputsRect.GetBR());

				ed::PinPivotRect(outputsRect.GetTL(), outputsRect.GetBR());
				ed::PinRect(outputsRect.GetTL(), outputsRect.GetBR());
				ed::PinPivotSize(ImVec2(40, 0));
				//ed::PinPivotScale(ImVec2(1.5, 1));
				//ed::PinPivotAlignment(ImVec2(0.f, -2.0f));
				//ed::PinPivotSize(ImVec2(90, -50));

				ed::EndPin();
				ed::PopStyleVar();

				if (newLinkPin && !CanCreateLink(newLinkPin, &pin) && &pin != newLinkPin)
					outputAlpha = (int)(255 * ImGui::GetStyle().Alpha * (48.0f / 255.0f));
			}
			else
				ImGui::Dummy(ImVec2(0, padding));

			ImGui::Spring(0, padding * 2);
			ImGui::EndHorizontal();

			ImGui::EndVertical();

			ed::EndNode();
			ed::PopStyleVar(8);
			ed::PopStyleColor(4);

			const auto nodeRect = ImGui_GetItemRect();

			auto drawList = ed::GetNodeBackgroundDrawList(node->ID);

			// rect filled attach point top left
			drawList->AddRectFilled(inputsRect.GetTL() + ImVec2(0, 1), inputsRect.GetBR(), IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), inputAlpha), 4.0f, 12);
			//ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);

			// border around attach point top left
			drawList->AddRect(inputsRect.GetTL() + ImVec2(0, 1), inputsRect.GetBR(), IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), inputAlpha), 4.0f, 12);
			//ImGui::PopStyleVar();

			// rect filled attach point right bottom
			drawList->AddRectFilled(outputsRect.GetTL(), outputsRect.GetBR() - ImVec2(0, 1), IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), outputAlpha), 4.0f, 3);
			//ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);

			// border around attach point right bottom
			drawList->AddRect(outputsRect.GetTL(), outputsRect.GetBR() - ImVec2(0, 1), IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), outputAlpha), 4.0f, 3);

			if (nodeState->HasPlayable() && dynamic_cast<NormalAnimation*>(&nodeState->GetPlayable()))
			{
				// i dont think blend trees have a "time", unsure tho
				const float progressBarWidth = (content2Rect.GetBR().x - content2Rect.GetTL().x) * (1.f - nodeState->GetElapsedTimeNormalized());

				content2Rect.Expand(-1);

				//ImGui::PopStyleVar();
				drawList->AddRectFilled(
					content2Rect.GetTL(),
					content2Rect.GetBR() - ImVec2(progressBarWidth, 0.f), IM_COL32(26, 186, 255, 255), 0.0f);
				//ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);

				content2Rect.Expand(1);

				// progress border
				drawList->AddRect(
					content2Rect.GetTL(),
					content2Rect.GetBR(),
					IM_COL32(0, 90, 130, 255),
					0.0f);
			}
			else if (nodeState->HasPlayable() && dynamic_cast<BlendTree1D*>(&nodeState->GetPlayable()))
			{
				// i dont think blend trees have a "time", unsure tho
				const float progressBarWidth = (content2Rect.GetBR().x - content2Rect.GetTL().x) * (1.f - nodeState->GetElapsedTimeNormalized());

				content2Rect.Expand(-1);

				//ImGui::PopStyleVar();
				drawList->AddRectFilled(
					content2Rect.GetTL(),
					content2Rect.GetBR() - ImVec2(progressBarWidth, 0.f), IM_COL32(26, 186, 255, 255), 0.0f);
				//ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);

				content2Rect.Expand(1);

				// progress border
				drawList->AddRect(
					content2Rect.GetTL(),
					content2Rect.GetBR(),
					IM_COL32(0, 90, 130, 255),
					0.0f);
			}
			else if (nodeState->HasPlayable() && dynamic_cast<BlendTree2D*>(&nodeState->GetPlayable()))
			{
				// i dont think blend trees have a "time", unsure tho
				const float progressBarWidth = (content2Rect.GetBR().x - content2Rect.GetTL().x) * (1.f - nodeState->GetElapsedTimeNormalized());

				content2Rect.Expand(-1);

				//ImGui::PopStyleVar();
				drawList->AddRectFilled(
					content2Rect.GetTL(),
					content2Rect.GetBR() - ImVec2(progressBarWidth, 0.f), IM_COL32(26, 186, 255, 255), 0.0f);
				//ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);

				content2Rect.Expand(1);

				// progress border
				drawList->AddRect(
					content2Rect.GetTL(),
					content2Rect.GetBR(),
					IM_COL32(0, 90, 130, 255),
					0.0f);
			}
			else
			{
				const std::string noPlayableText = "No playable";

				const ImVec2 textSize = ImGui::CalcTextSize(noPlayableText.c_str());

				const ImVec2 centerPos = content2Rect.GetTL() + (content2Rect.GetBR() - content2Rect.GetTL()) * 0.5f - textSize * 0.5f;

				drawList->AddText(
					centerPos,
					IM_COL32(255, 255, 255, 255),
					noPlayableText.c_str());
			}

			//ImGui::PopStyleVar();
		}

		for (auto& link : myLinks)
			ed::Link(link->ID, link->StartPinID, link->EndPinID, link->Color, 2.0f);

		if (!createNewNode)
		{
			if (ed::BeginCreate(ImColor(255, 255, 255), 2.0f))
			{
				auto showLabel = [](const char* label, ImColor color)
				{
					ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetTextLineHeight());
					auto size = ImGui::CalcTextSize(label);

					auto padding = ImGui::GetStyle().FramePadding;
					auto spacing = ImGui::GetStyle().ItemSpacing;

					ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(spacing.x, -spacing.y));

					auto rectMin = ImGui::GetCursorScreenPos() - padding;
					auto rectMax = ImGui::GetCursorScreenPos() + size + padding;

					auto drawList = ImGui::GetWindowDrawList();
					drawList->AddRectFilled(rectMin, rectMax, color, size.y * 0.15f);
					ImGui::TextUnformatted(label);
				};

				ed::PinId startPinId = 0, endPinId = 0;
				if (ed::QueryNewLink(&startPinId, &endPinId))
				{
					auto startPin = FindPin(startPinId);
					auto endPin = FindPin(endPinId);

					newLinkPin = startPin ? startPin : endPin;

					if (startPin->Kind == PinKind::Input)
					{
						std::swap(startPin, endPin);
						std::swap(startPinId, endPinId);
					}

					if (startPin && endPin)
					{
						assert(startPin->Node->Type == NodeType::State);
						assert(endPin->Node->Type == NodeType::State);

						if (endPin == startPin)
						{
							ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
						}
						else if (endPin->Kind == startPin->Kind)
						{
							showLabel("x Incompatible Pin Kind", ImColor(45, 32, 32, 180));
							ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
						}
						else if (endPin->Node == startPin->Node)
						{
							showLabel("x Cannot connect to self", ImColor(45, 32, 32, 180));
							ed::RejectNewItem(ImColor(255, 0, 0), 1.0f);
						}
						else if (endPin->Type != startPin->Type)
						{
							showLabel("x Incompatible Pin Type", ImColor(45, 32, 32, 180));
							ed::RejectNewItem(ImColor(255, 128, 128), 1.0f);
						}
						else
						{
							bool alreadyHasTransition = false;

							const auto startState = static_cast<AnimationState*>(startPin->Node);
							const auto endState = static_cast<AnimationState*>(endPin->Node);

							if (std::any_of(
								startState->myTransitions.begin(),
								startState->myTransitions.end(),
								[endState](Owned<Transition>& aTransition)
								{
									return aTransition->GetTargetState() == endState;
								}))
							{
								alreadyHasTransition = true;
							}

								if (alreadyHasTransition)
								{
									showLabel("x Transition already exist", ImColor(45, 32, 32, 180));
									ed::RejectNewItem(ImColor(255, 128, 128), 1.0f);
								}
								else
								{
									showLabel("+ Create Link", ImColor(32, 45, 32, 180));
									if (ed::AcceptNewItem(ImColor(128, 255, 128), 4.0f))
									{
										// if we came here, we have more nodes, but the assume
										// that a Node can only be a AnimationState at the moment
										// below we implicilty cast to the type
										// this will not work
										assert(startPin->Node->Type == NodeType::State);
										assert(endPin->Node->Type == NodeType::State);

										// Create new transition
										auto transition = MakeOwned<FadeTransition>();
										transition->myTargetState = static_cast<AnimationState*>(endPin->Node);
										transition->myTargetStateName = transition->myTargetState->GetName();

										// Link stuff
										transition->ID = GetNextId();
										transition->StartPinID = startPinId;
										transition->EndPinID = endPinId;
										transition->Color = GetIconColor(startPin->Type);

										// NOTE(We assume that the node is an AnimationState)
										// this will not work in future if a node can be more
										static_cast<AnimationState*>(startPin->Node)->AddTransition(std::move(transition));


										//myLinks.emplace_back(Link(GetNextId(), startPinId, endPinId));
										//myLinks.back().Color = GetIconColor(startPin->Type);
									}
								}
						}
					}
				}

				ed::PinId pinId = 0;
				if (ed::QueryNewNode(&pinId))
				{
					newLinkPin = FindPin(pinId);
					if (newLinkPin)
						showLabel("+ Create Node", ImColor(32, 45, 32, 180));

					if (ed::AcceptNewItem())
					{
						//createNewNode = true;
						//newNodeLinkPin = FindPin(pinId);
						//newLinkPin = nullptr;
						//ed::Suspend();
						//ImGui::OpenPopup("Create New Node");
						//ed::Resume();
					}
				}
			}
			else
				newLinkPin = nullptr;

			ed::EndCreate();

			if (ed::BeginDelete())
			{
				ed::LinkId linkId = 0;
				while (ed::QueryDeletedLink(&linkId))
				{
					if (ed::AcceptDeletedItem())
					{
						auto id = std::find_if(myLinks.begin(), myLinks.end(), [linkId](auto& link)
							{
								return link->ID == linkId;
							});
						if (id != myLinks.end())
						{
							Link* lnk = *id;

							assert(mySelectedLayer);

							// Find the transition and remove it from the state
							for (auto& [name, state] : mySelectedLayer->myStates)
							{
								for (int transitionIndex = 0;
									transitionIndex < state->myTransitions.size();
									++transitionIndex)
								{
									if (state->myTransitions[transitionIndex]->ID == lnk->ID)
									{
										state->myTransitions.erase(state->myTransitions.begin() + transitionIndex);
										break;
									}
								}
							}

							// Remove the link directly as well because
							// we removed the actual owned pointer above
							// otherwise, we could access a delete pointer :S
							myLinks.erase(id);
						}
					}
				}

				ed::NodeId nodeId = 0;
				while (ed::QueryDeletedNode(&nodeId))
				{
					if (ed::AcceptDeletedItem())
					{
						assert(mySelectedLayer);

						auto id = std::find_if(myNodes.begin(), myNodes.end(), [nodeId](auto& node)
							{
								return node->ID == nodeId;
							});
						if (id != myNodes.end())
						{
							Node* node = *id;

							assert(node->Type == NodeType::State);

							for (auto& [name, state] : mySelectedLayer->myStates)
							{
								if (state->ID == node->ID && state->GetName() != "Any State")
								{
									// If its the entry state, remove the entry state
									if (mySelectedLayer->myInitialState == node)
									{
										mySelectedLayer->myInitialState = nullptr;
									}

									mySelectedLayer->myStates.erase(name);

									for (auto& [name, state] : mySelectedLayer->myStates)
									{
										if (name == "Any State")
										{
											continue;
										}

										mySelectedLayer->myInitialState = state.get();
									}

									// To avoid crash if the state we just removed was the active one
									mySelectedLayer->ResetToInitialState();

									myNodes.erase(id);

									break;
								}
							}
						}
					}
				}
			}
			ed::EndDelete();
		}

		ImGui::SetCursorScreenPos(cursorTopLeft);
	}

#if 1
	auto openPopupPosition = ImGui::GetMousePos();
	ed::Suspend();
	if (ed::ShowNodeContextMenu(&contextNodeId))
		ImGui::OpenPopup("Node Context Menu");
	else if (ed::ShowPinContextMenu(&contextPinId))
		ImGui::OpenPopup("Pin Context Menu");
	else if (ed::ShowLinkContextMenu(&contextLinkId))
		ImGui::OpenPopup("Link Context Menu");
	else if (ed::ShowBackgroundContextMenu())
	{
		ImGui::OpenPopup("Create New Node");
		newNodeLinkPin = nullptr;
	}
	ed::Resume();

	ed::Suspend();
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
	if (ImGui::BeginPopup("Node Context Menu"))
	{
		auto node = FindNode(contextNodeId);

		ImGui::TextUnformatted("Node Context Menu");
		ImGui::Separator();
		if (node)
		{
			ImGui::Text("ID: %p", node->ID.AsPointer());
			ImGui::Text("Type: %s", node->Type == NodeType::State ? "Tree" : "unknown");
			ImGui::Text("Inputs: %d", (int)node->Inputs.size());
			ImGui::Text("Outputs: %d", (int)node->Outputs.size());
		}
		else
			ImGui::Text("Unknown node: %p", contextNodeId.AsPointer());
		ImGui::Separator();
		if (ImGui::MenuItem("Delete"))
			ed::DeleteNode(contextNodeId);
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup("Pin Context Menu"))
	{
		auto pin = FindPin(contextPinId);

		ImGui::TextUnformatted("Pin Context Menu");
		ImGui::Separator();
		if (pin)
		{
			ImGui::Text("ID: %p", pin->ID.AsPointer());
			if (pin->Node)
				ImGui::Text("Node: %p", pin->Node->ID.AsPointer());
			else
				ImGui::Text("Node: %s", "<none>");
		}
		else
			ImGui::Text("Unknown pin: %p", contextPinId.AsPointer());

		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup("Link Context Menu"))
	{
		auto link = FindLink(contextLinkId);

		ImGui::TextUnformatted("Link Context Menu");
		ImGui::Separator();
		if (link)
		{
			ImGui::Text("ID: %p", link->ID.AsPointer());
			ImGui::Text("From: %p", link->StartPinID.AsPointer());
			ImGui::Text("To: %p", link->EndPinID.AsPointer());
		}
		else
			ImGui::Text("Unknown link: %p", contextLinkId.AsPointer());
		ImGui::Separator();
		if (ImGui::MenuItem("Delete"))
			ed::DeleteLink(contextLinkId);
		ImGui::EndPopup();
	}

	if (mySelectedLayer && ImGui::BeginPopup("Create New Node"))
	{
		auto newNodePostion = openPopupPosition;

		Node* node = nullptr;
		if (ImGui::MenuItem("New State"))
		{
			std::string newStateName = "New State";

			int stateNameCounter = 0;
			while (mySelectedLayer->TryGetState(newStateName))
			{
				newStateName = "New State " + std::to_string(stateNameCounter);
				++stateNameCounter;
			}

			node = SpawnStateNode(newStateName);
		}

		if (node)
		{
			BuildNodes();

			createNewNode = false;

			ed::SetNodePosition(node->ID, newNodePostion);

			if (auto startPin = newNodeLinkPin)
			{
				auto& pins = startPin->Kind == PinKind::Input ? node->Outputs : node->Inputs;

				for (auto& pin : pins)
				{
					if (CanCreateLink(startPin, &pin))
					{
						/*
						auto endPin = &pin;
						if (startPin->Kind == PinKind::Input)
							std::swap(startPin, endPin);

						assert(false && "removed this when i turned it into vector of pointers, fix this");
						//myLinks.emplace_back(Link(GetNextId(), startPin->ID, endPin->ID));
						//myLinks.back()->Color = GetIconColor(startPin->Type);
						*/

						break;
					}
				}
			}
		}

		ImGui::EndPopup();
	}
	else
		createNewNode = false;
	ImGui::PopStyleVar();
	ed::Resume();
#endif

	// Must navigate to content after creating nodes
	// and must change mySelectedLayer to new value BEFORE
	// calling ReCreateNodes/States
	// MUST ALSO DO THIS AT END OF FRAME
	// OTHERWISE THE NODE EDITOR NAVIGATE TO THE PREVIOUS LAYERS NODES
	if (changedLayer)
	{
		ed::NavigateToContent();
	}

	ed::PushStyleVar(NodeEditor::StyleVar_FlowDuration, 0.4f);

	for (const auto& layer : myMachine->myLayers)
	{
		if (layer->myActiveTransition)
		{
			ed::Flow(layer->myActiveTransition->ID);
		}
	}

	ed::PopStyleVar();

	//for (auto i : myFlowsToBeShown)
	//{
	//	ed::Flow(i);
	//}

	ed::End();
}

void Engine::AnimationNodeEditorWindow::ShowLeftPane(float paneWidth)
{
	auto& io = ImGui::GetIO();

	if (ImGui::BeginChild("##leftpane", ImVec2(paneWidth, 0)))
	{
		// This is only valid if we are editing a file because we need to create our own instance
		// of AnimationMachine
		if (IsEditingFile())
		{
			ImGui::TextColored(ImVec4(1, 0.5, 0, 1), "%s", "Editing [FILE]");
		}
		else
		{
			ImGui::TextColored(ImVec4(0, 1, 0, 1), "%s", "Editing [INSTANCE]");
		}

		std::vector<AnimationClip*> allClips;

		FindAnimationClipsVisitor compabilityVisitor(allClips);
		myMachine->VisitPlayables(compabilityVisitor);

		bool hasIssue = false;

		if (myMachine->myModel && myMachine->myModel->IsValid())
		{
			const int bindPoseBoneCount = myMachine->myBindPose.myEntries.size();

			std::vector<AnimationClip*> incompatibleClips;

			for (auto& clip : allClips)
			{
				if (clip->GetBoneNodes().size() != bindPoseBoneCount)
				{
					incompatibleClips.push_back(clip);
				}
			}

			if (!incompatibleClips.empty())
			{
				for (const auto& clip : incompatibleClips)
				{
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));

					ImGui::TextWrapped(
						"Clip incompatible with model bindpose: %s, BoneCount: %d",
						std::filesystem::path(clip->GetPath()).stem().string().c_str(), clip->GetBoneNodes().size());

					ImGui::PopStyleColor();

					hasIssue = true;
				}

				ImGui::TextColored(ImVec4(1, 0, 0, 1), "BindPose Bone Count: %d", myMachine->myBindPose.myEntries.size());
			}
		}

		for (auto& clipOuter : allClips)
		{
			for (auto& clipInner : allClips)
			{
				if (clipOuter->GetBoneNodes().size() != clipInner->GetBoneNodes().size())
				{
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));

					ImGui::TextWrapped(
						"Clip %s, BoneCount: %d incompatible with clip: %s, BoneCount: %d",
						std::filesystem::path(clipOuter->GetPath()).stem().string().c_str(),
						clipOuter->GetBoneNodes().size(),
						std::filesystem::path(clipInner->GetPath()).stem().string().c_str(),
						clipInner->GetBoneNodes().size());

					ImGui::PopStyleColor();

					hasIssue = true;
				}
			}
		}

		if (hasIssue)
		{
			if (ImGui::Button("TRY FIX ISSUES"))
			{
				if (myMachine->myModel && myMachine->myModel->IsValid())
				{
					myMachine->GenerateBindPose(myMachine->myModel->Get());
				}
			}
		}

		ImGui::Separator();

		const float leftPaneHeight = std::max(ImGui::GetContentRegionAvail().y, 300.f);

		if (ImGui::BeginChild("##leftpane2", ImVec2(paneWidth, leftPaneHeight)))
		{
			if (ImGui::BeginTabBar("TabbarForFan"))
			{
				if (ImGui::BeginTabItem("Layers"))
				{
					DrawLayers();

					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Inspection", nullptr,
					myShowInspectionTab ? ImGuiTabItemFlags_SetSelected : 0))
				{
					myShowInspectionTab = false;

					DrawInspection();

					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Parameters"))
				{
					DrawParameters();

					ImGui::EndTabItem();
				}

				ImGui::EndTabBar();
			}
		}
		ImGui::EndChild();

		//if (ImGui::BeginTabBar("TabbarForFan2"))
		//{
		//	if (ImGui::BeginTabItem("Inspection"))
		//	{
		//		DrawInspection();

		//		ImGui::EndTabItem();
		//	}

		//	ImGui::EndTabBar();
		//}

	}
	ImGui::EndChild();

	//ImGui::BeginChild("Selection", ImVec2(paneWidth, 0));

	//if (ImGui::BeginTabBar("tabbar"))
	//{
	//	if (ImGui::BeginTabItem("Inspection"))
	//	{
	//		DrawInspection();

	//		ImGui::EndTabItem();
	//	}

	//	if (ImGui::BeginTabItem("Layers"))
	//	{
	//		DrawLayers();

	//		ImGui::EndTabItem();
	//	}

	//	if (ImGui::BeginTabItem("Parameters"))
	//	{
	//		DrawParameters();
	//		ImGui::EndTabItem();
	//	}

	//	ImGui::EndTabBar();
	//}

	//ImGui::EndChild();
}

void Engine::AnimationNodeEditorWindow::DrawPinIcon(const Pin& pin, bool connected, int alpha)
{
	IconType iconType;
	ImColor color = GetIconColor(pin.Type);
	color.Value.w = alpha / 255.0f;
	switch (pin.Type)
	{
	case PinType::Flow:
		iconType = IconType::Flow;
		break;
	case PinType::Bool:
		iconType = IconType::Circle;
		break;
	case PinType::Int:
		iconType = IconType::Circle;
		break;
	case PinType::Float:
		iconType = IconType::Circle;
		break;
	case PinType::String:
		iconType = IconType::Circle;
		break;
	case PinType::Object:
		iconType = IconType::Circle;
		break;
	default:
		return;
	}

	ax::Widgets::Icon(ImVec2(myPinIconSize, myPinIconSize), iconType, connected, color, ImColor(32, 32, 32, alpha));
}

ImColor Engine::AnimationNodeEditorWindow::GetIconColor(PinType type)
{
	switch (type)
	{
	default:
	case PinType::Flow:
		return ImColor(255, 255, 255);
	case PinType::Bool:
		return ImColor(220, 48, 48);
	case PinType::Int:
		return ImColor(68, 201, 156);
	case PinType::Float:
		return ImColor(147, 226, 74);
	case PinType::String:
		return ImColor(124, 21, 153);
	case PinType::Object:
		return ImColor(51, 150, 215);
	}
}

void Engine::AnimationNodeEditorWindow::Application_Finalize()
{
	if (myNodeEditor)
	{
		ed::DestroyEditor(myNodeEditor);
		myNodeEditor = nullptr;
	}
}

void Engine::AnimationNodeEditorWindow::Application_Initialize()
{
	ed::Config config;
	config.SettingsFile = nullptr;

	myNodeEditor = ed::CreateEditor(&config);
	ed::SetCurrentEditor(myNodeEditor);

	DetermineNextId();

	if (!myMachine->myLayers.empty())
	{
		// mySelectedLayer = myMachine->myLayers.front().get();
		myNextSelectedLayer = myMachine->myLayers.front().get();
	}

	//if (!mySelectedLayer->TryGetState("Any State"))
	//{
	//	SpawnAnyStateNode();
	//}

	// SpawnEntryStateNode();

	ReCreateNodesOfStates();
	ReCreateStateTransitionsLinks();

	// Set the node positions
	//for (auto& node : myNodes)
	//{
	//	ed::SetNodePosition(node->ID, node->myPosition);
	//}

	// Set the initial node positions
	for (const auto& layer : myMachine->myLayers)
	{
		for (auto& [_, node] : layer->myStates)
		{
			ed::SetNodePosition(node->ID, node->myPosition);
		}
	}

	Node* node;
	//node = SpawnAnyStateNode();
	//ed::SetNodePosition(node->ID, ImVec2(1028, 329));
	//node = SpawnEntryStateNode();
	//ed::SetNodePosition(node->ID, ImVec2(1204, 458));
	//node = SpawnStateNode("Attack22");
	//ed::SetNodePosition(node->ID, ImVec2(468, 538));

	ed::NavigateToContent();

	BuildNodes();
}

void Engine::AnimationNodeEditorWindow::BuildNodes()
{
	for (auto& node : myNodes)
		BuildNode(node);
}

int Engine::AnimationNodeEditorWindow::GetNextId()
{
	return myNextId++;
}

ed::LinkId Engine::AnimationNodeEditorWindow::GetNextLinkId()
{
	return ed::LinkId(GetNextId());
}

void Engine::AnimationNodeEditorWindow::TouchNode(ed::NodeId id)
{
	myNodeTouchTime[id] = myTouchTime;
}

float Engine::AnimationNodeEditorWindow::GetTouchProgress(ed::NodeId id)
{
	auto it = myNodeTouchTime.find(id);
	if (it != myNodeTouchTime.end() && it->second > 0.0f)
		return (myTouchTime - it->second) / myTouchTime;
	else
		return 0.0f;
}

void Engine::AnimationNodeEditorWindow::UpdateTouch()
{
	const auto deltaTime = ImGui::GetIO().DeltaTime;
	for (auto& entry : myNodeTouchTime)
	{
		if (entry.second > 0.0f)
			entry.second -= deltaTime;
	}
}

Engine::AnimationNodeEditorWindow::Node* Engine::AnimationNodeEditorWindow::FindNode(ed::NodeId id)
{
	for (auto& node : myNodes)
		if (node->ID == id)
			return node;

	return nullptr;
}

Engine::AnimationNodeEditorWindow::Link* Engine::AnimationNodeEditorWindow::FindLink(ed::LinkId id)
{
	for (auto& link : myLinks)
		if (link->ID == id)
			return link;

	return nullptr;
}

Engine::AnimationNodeEditorWindow::Pin* Engine::AnimationNodeEditorWindow::FindPin(ed::PinId id)
{
	if (!id)
		return nullptr;

	for (auto& node : myNodes)
	{
		for (auto& pin : node->Inputs)
			if (pin.ID == id)
				return &pin;

		for (auto& pin : node->Outputs)
			if (pin.ID == id)
				return &pin;
	}

	return nullptr;
}

bool Engine::AnimationNodeEditorWindow::IsPinLinked(ed::PinId id) const
{
	if (!id)
		return false;

	for (auto& link : myLinks)
		if (link->StartPinID == id || link->EndPinID == id)
			return true;

	return false;
}

bool Engine::AnimationNodeEditorWindow::CanCreateLink(Pin* a, Pin* b)
{
	if (!a || !b || a == b || a->Kind == b->Kind || a->Type != b->Type || a->Node == b->Node)
		return false;

	return true;
}

void Engine::AnimationNodeEditorWindow::BuildNode(Node* node)
{
	for (auto& input : node->Inputs)
	{
		input.Node = node;
		input.Kind = PinKind::Input;
	}

	for (auto& output : node->Outputs)
	{
		output.Node = node;
		output.Kind = PinKind::Output;
	}
}

Engine::AnimationNodeEditorWindow::Node* Engine::AnimationNodeEditorWindow::SpawnStateNode(const std::string& aName)
{
	// myNodes.emplace_back(GetNextId(), aName.c_str());

	auto state = MakeOwned<AnimationState>(aName, /*hasPose*/nullptr);

	//myNodes.push_back(state);


	state->ID = GetNextId();
	state->Name = aName;
	state->Type = NodeType::State;
	state->Inputs.emplace_back(GetNextId(), "", PinType::Flow);
	state->Outputs.emplace_back(GetNextId(), "", PinType::Flow);

	BuildNode(state.get());

	AnimationState* stateptr = state.get();

	ed::SetNodePosition(state->ID, ImVec2(168, 538));

	mySelectedLayer->AddState(std::move(state));

	// Make this state the entry if no entry exists
	if (!mySelectedLayer->HasInitialState())
	{
		mySelectedLayer->myInitialState = stateptr;

		mySelectedLayer->ResetToInitialState();
		// mySelectedLayer->SetActiveState(stateptr);
	}

	return stateptr;
}

Engine::AnimationNodeEditorWindow::Node* Engine::AnimationNodeEditorWindow::SpawnAnyStateNode()
{
	const std::string name = "Any State";

	auto myAnyStateNode = MakeOwned<AnimationState>(name, /*hasPose*/nullptr);

	myAnyStateNode->ID = GetNextId();
	myAnyStateNode->Name = name;
	myAnyStateNode->Type = NodeType::State;
	myAnyStateNode->Outputs.emplace_back(GetNextId(), "", PinType::Flow);

	// TODO: set the color to something different!
	myAnyStateNode->Color;

	BuildNode(myAnyStateNode.get());

	AnimationState* ret = myAnyStateNode.get();

	ed::SetNodePosition(myAnyStateNode->ID, ImVec2(-300.f, -150.f));

	mySelectedLayer->AddState(std::move(myAnyStateNode));

	mySelectedLayer->myAnyState = &mySelectedLayer->GetState(name);

	return ret;
}

//Engine::AnimationNodeEditorWindow::Node* Engine::AnimationNodeEditorWindow::SpawnEntryStateNode()
//{
//	const std::string name = "Entry";
//
//	myEntryStateNode = MakeOwned<AnimationState>(*myMachine, name, /*hasPose*/nullptr);
//
//	myEntryStateNode->ID = GetNextId();
//	myEntryStateNode->Name = name;
//	myEntryStateNode->Type = NodeType::State;
//	myEntryStateNode->Outputs.emplace_back(GetNextId(), "", PinType::Flow);
//
//	// TODO: set the color to something different!
//	myEntryStateNode->Color;
//
//	BuildNode(myEntryStateNode.get());
//
//	AnimationState* ret = myEntryStateNode.get();
//
//	ed::SetNodePosition(myEntryStateNode->ID, ImVec2(-300.f, 150.f));
//
//	return ret;
//}

void Engine::AnimationNodeEditorWindow::CreateTransition()
{
	auto transition = MakeOwned<FadeTransition>();

	// return transition;
}

void Engine::AnimationNodeEditorWindow::ReCreateNodesOfStates()
{
	myNodes.clear();

	if (mySelectedLayer)
	{
		// To support multiple nodes easily
		for (auto& [name, node] : mySelectedLayer->myStates)
		{
			myNodes.push_back(node.get());
		}
	}


	//myNodes.push_back(myAnyStateNode.get());
	//myNodes.push_back(myEntryStateNode.get());
}

void Engine::AnimationNodeEditorWindow::ReCreateStateTransitionsLinks()
{
	myLinks.clear();

	if (mySelectedLayer)
	{
		for (auto& [_, node] : mySelectedLayer->myStates)
		{
			for (const auto& transition : node->GetTransitions())
			{
				myLinks.push_back(transition.get());
			}
		}
	}
}

void Engine::AnimationNodeEditorWindow::Save()
{
	EditorWindow::Save();

	// Regenerate the bindpose in case the .model was changed
	if (myMachine->myModel && myMachine->myModel->IsValid())
	{
		myMachine->GenerateBindPose(myMachine->myModel->Get());
	}

	if (myMachine->GetFirstAnimationClip() && myMachine->GetFirstAnimationClip()->IsValid())
	{
		// if the model was changed, the mask size might not match the skeleton of the new model
		// therefore, fix this here
		const int boneCount = myMachine->GetFirstAnimationClip()->Get().GetBoneNodes().size();

		for (const auto& layer : myMachine->myLayers)
		{
			// skip if not enabled
			if (!layer->myBlendMask.myIsEnabled)
			{
				continue;
			}

			if (boneCount != layer->myBlendMask.myMask.size())
			{
				layer->myBlendMask.myMask.resize(boneCount);
				LOG_INFO(LogType::Animation) << "The blend mask of layer: " << layer->GetName() << ", did not match model skeleton. Regenerated it.";
			}
		}
	}

	myAreChangesMadeWithoutSaving = false;

	FileIO::RemoveReadOnly(mySavePath);

	std::ofstream file(mySavePath.ToWString());

	if (file.is_open())
	{
		file << std::setw(4) << myMachine->ToJson();
		file.close();
	}
	else
	{
		LOG_ERROR(LogType::Engine) << "Failed to save anim node graph, is it read only?";
	}
}

void Engine::AnimationNodeEditorWindow::DetermineNextId()
{
	// The myNextId is used for everything!
	// Meaning: Nodes, Pins, Links

	for (const auto& layer : myMachine->myLayers)
	{
		for (const auto& [_, state] : layer->myStates)
		{
			for (const auto& transition : state->myTransitions)
			{
				if (transition->ID.Get() > myNextId)
				{
					myNextId = transition->ID.Get();
				}
			}

			if (state->ID.Get() > myNextId)
			{
				myNextId = state->ID.Get();
			}

			for (const auto& inputPin : state->Inputs)
			{
				if (inputPin.ID.Get() > myNextId)
				{
					myNextId = inputPin.ID.Get();
				}
			}

			for (const auto& outputPin : state->Outputs)
			{
				if (outputPin.ID.Get() > myNextId)
				{
					myNextId = outputPin.ID.Get();
				}
			}
		}
	}

	// Move to the next so we never get the same ID of the last one saved
	++myNextId;
}

void Engine::AnimationNodeEditorWindow::DrawInspection()
{
	std::vector<ed::NodeId> selectedNodes;
	std::vector<ed::LinkId> selectedLinks;
	selectedNodes.resize(ed::GetSelectedObjectCount());
	selectedLinks.resize(ed::GetSelectedObjectCount());

	int nodeCount = ed::GetSelectedNodes(selectedNodes.data(), static_cast<int>(selectedNodes.size()));
	int linkCount = ed::GetSelectedLinks(selectedLinks.data(), static_cast<int>(selectedLinks.size()));

	selectedNodes.resize(nodeCount);
	selectedLinks.resize(linkCount);

	for (auto& linkPtr : myLinks)
	{
		const auto& link = *linkPtr;

		// NOTE(filip): we assume all links are FadeTransition's atm, we
		// probably wont have other types of transitions
		auto transition = static_cast<FadeTransition*>(linkPtr);

		ImGui::PushID(link.ID.AsPointer());

		const bool isSelected =
			std::find(selectedLinks.begin(), selectedLinks.end(), link.ID) != selectedLinks.end();

		if (isSelected)
		{
			ImGuiHelper::AlignedWidget("Can Interrupt", ourAlignPercent);

			ImGui::Checkbox("##caninterrupt", &transition->myCanInterruptCurrentPlayingAnimation);

			// If we can interrupt, no need to draw Start At because it
			// will not matter because it will be interrupted
			if (!transition->myCanInterruptCurrentPlayingAnimation)
			{
				ImGuiHelper::AlignedWidget("Start At", ourAlignPercent);

				ImGui::DragFloat("##startat", &transition->myExitTimeNormalized, 0.01f, 0.f, 1.f);
			}

			ImGuiHelper::AlignedWidget("Fade Duration Seconds", ourAlignPercent);

			ImGui::DragFloat("##fadeduration", &transition->myDurationSeconds, 0.01f, 0.f, 5.f);
			//ImGui::DragFloat("Fade Duration Seconds", &transition->myDurationSeconds, 0.01f, 0.f, 5.f);

			bool useTrigger = !transition->myTriggerName.empty();

			ImGuiHelper::AlignedWidget("Use Trigger", ourAlignPercent);

			if (ImGui::Checkbox("##usertrigger", &useTrigger))
			{
				if (useTrigger)
				{
					if (myMachine->myTriggers.empty())
					{
						useTrigger = false;
						LOG_ERROR(LogType::Editor) << "No triggers exists, create one first!";
					}
					else
					{
						transition->myTriggerName = myMachine->myTriggers.begin()->first;
						transition->SetTriggerPtr(myMachine->myTriggers.begin()->second.get());
					}
				}
				else
				{
					transition->myTriggerName = "";
					transition->myTrigger = nullptr;
				}
			}

			if (useTrigger)
			{
				ImGuiHelper::AlignedWidget("Trigger", ourAlignPercent);

				const std::string& triggerComboPreview = transition->myTriggerName;

				if (ImGui::BeginCombo("##trigger", triggerComboPreview.c_str()))
				{
					for (const auto& [name, triggerBool] : myMachine->myTriggers)
					{
						if (ImGui::Selectable(name.c_str(), triggerComboPreview == name))
						{
							transition->myTriggerName = name;
							transition->SetTriggerPtr(triggerBool.get());
						}
					}

					ImGui::EndCombo();
				}
			}

			ImGui::Dummy(ImVec2(0.f, 20.f));

			// Conditions
			static ImGuiTableFlags flags =
				ImGuiTableFlags_SizingFixedFit |
				ImGuiTableFlags_RowBg |
				ImGuiTableFlags_Borders |
				ImGuiTableFlags_Resizable |
				ImGuiTableFlags_Reorderable |
				ImGuiTableFlags_Hideable;

			int conditionIndexToRemove = -1;
			int condGroupToRemove = -1;

			for (int condGroupIndex = 0; condGroupIndex < transition->myConditionGroups.size(); ++condGroupIndex)
			{
				ImGui::PushID(condGroupIndex);
				ImGui::Text(("Group: " + std::to_string(condGroupIndex)).c_str());

				if (IsEditingInstance())
				{
					ImGui::SameLine();

					if (transition->myConditionGroups[condGroupIndex].IsTrue())
					{
						ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "Evaluates: TRUE");
					}
					else
					{
						ImGui::TextColored(ImVec4(191.f / 255.f, 151.f / 255.f, 11.f / 255.f, 1.f), "Evaluates: FALSE");
					}
				}

				for (int conditionIndex = 0; conditionIndex < transition->myConditionGroups[condGroupIndex].myConditions.size(); ++conditionIndex)
				{
					ImGui::PushID(conditionIndex);

					auto& cond = transition->myConditionGroups[condGroupIndex].myConditions[conditionIndex];
					const auto& name = cond.GetValueName();

					if (ImGuiHelper::RedRemoveButton("##remove", 24.f))
					{
						conditionIndexToRemove = conditionIndex;
						condGroupToRemove = condGroupIndex;
					}

					ImGui::SameLine();

					ImGui::Text(name.c_str());

					ImGui::SameLine();

					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth() * 0.35f);
					DrawConditionComparisonCombo(cond.myComparison);

					ImGui::SameLine();

					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth() * 0.35f);
					switch (cond.GetValueType())
					{
					case ConditionValueType::Bool:
					{
						ImGui::Checkbox("##comparevaluebool", &std::get<bool>(cond.myCompareValue));
					} break;
					case ConditionValueType::Int:
					{
						int indexd = cond.myCompareValue.index();
						int& in = std::get<int>(cond.myCompareValue);
						ImGui::DragInt("##comparevalueint", &in);
					} break;
					case ConditionValueType::Float:
					{
						ImGui::DragFloat("##comparevaluefloat", &std::get<float>(cond.myCompareValue));
					} break;
					default:
						ImGui::Text("Unsupported value type");
						break;
					}

					if (IsEditingInstance())
					{
						ImGui::SameLine();

						if (cond.IsTrue())
						{
							ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "Evaluates: TRUE");
						}
						else
						{
							ImGui::TextColored(ImVec4(191.f / 255.f, 151.f / 255.f, 11.f / 255.f, 1.f), "Evaluates: FALSE");
						}
					}

					ImGui::PopID();
				}

				ImGui::PopID();
			}

			if (conditionIndexToRemove != -1 && condGroupToRemove != -1)
			{
				auto& condGroup = transition->myConditionGroups[condGroupToRemove];

				condGroup.myConditions.erase(
					condGroup.myConditions.begin() + conditionIndexToRemove);

				if (condGroup.myConditions.empty())
				{
					transition->myConditionGroups.erase(transition->myConditionGroups.begin() + condGroupToRemove);
				}
			}

			static std::string selectedParameterName;
			static bool newConditionFailed = false;
			static int selectedGroupIndex = 0;

			if (ImGui::Button("Create Condition"))
			{
				ImGui::OpenPopup("NewCondition2");
				newConditionFailed = false;
				selectedParameterName = "";
				selectedGroupIndex = 0;
			}

			ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_Always);

			ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

			if (ImGui::BeginPopupModal("NewCondition2"))
			{
				static ConditionValueType selectedConditionType = ConditionValueType::Bool;

				ImGuiHelper::AlignedWidget("Type", ourAlignPercent);

				if (ImGui::BeginCombo(
					"##conditiontype",
					GlobalConditionValueTypeStrings[static_cast<int>(selectedConditionType)].c_str()))
				{
					for (int i = 0; i < static_cast<int>(ConditionValueType::Count); ++i)
					{
						if (ImGui::Selectable(GlobalConditionValueTypeStrings[i].c_str(), i == static_cast<int>(selectedConditionType)))
						{
							selectedConditionType = static_cast<ConditionValueType>(i);
						}
					}

					ImGui::EndCombo();
				}

				//ImGui::SameLine();

				const std::string& parameterPreview = selectedParameterName;

				ImGuiHelper::AlignedWidget("Parameter", ourAlignPercent);

				if (ImGui::BeginCombo("##parameter", parameterPreview.c_str()))
				{
					switch (selectedConditionType)
					{
					case ConditionValueType::Bool:
						for (const auto& [name, conditionBool] : myMachine->myBools)
						{
							if (ImGui::Selectable(name.c_str(), parameterPreview == name))
							{
								selectedParameterName = name;
							}
						}
						break;
					case ConditionValueType::Int:
						for (const auto& [name, conditionBool] : myMachine->myInts)
						{
							if (ImGui::Selectable(name.c_str(), parameterPreview == name))
							{
								selectedParameterName = name;
							}
						}
						break;
					case ConditionValueType::Float:
						for (const auto& [name, conditionBool] : myMachine->myFloats)
						{
							if (ImGui::Selectable(name.c_str(), parameterPreview == name))
							{
								selectedParameterName = name;
							}
						}
						break;
					default:
						ImGui::Selectable("Unsupported value type");
						break;
					}

					ImGui::EndCombo();
				}

				static ConditionComparison selectedConditionComparison = ConditionComparison::Equals;

				ImGuiHelper::AlignedWidget("Condition", ourAlignPercent);

				DrawConditionComparisonCombo(selectedConditionComparison);

				static bool compareBoolValue = false;
				static int compareIntValue = 0;
				static float compareFloatValue = 0;

				ImGuiHelper::AlignedWidget("Compare Value", ourAlignPercent);

				switch (selectedConditionType)
				{
				case ConditionValueType::Bool:
					ImGui::Checkbox("##comparevaluebool", &compareBoolValue);
					break;
				case ConditionValueType::Int:
					ImGui::DragInt("##comparevalueint", &compareIntValue);
					break;
				case ConditionValueType::Float:
					ImGui::DragFloat("##comparevaluefloat", &compareFloatValue);
					break;
				default:
					ImGui::Text("Unsupported value type");
					break;
				}

				int max = 0;

				if (!transition->myConditionGroups.empty())
				{
					if (!transition->myConditionGroups.back().GetConditions().empty())
					{
						max = transition->myConditionGroups.size();
					}
					else
					{
						max = transition->myConditionGroups.size();
					}
				}

				// if (transition->myConditionGroups.back().GetConditions().empty())
				// {
				// 	max = transition->myConditionGroups.size();
				// }
				// else
				// {
				// 
				// }

				ImGuiHelper::AlignedWidget("Group Index", ourAlignPercent);
				ImGui::DragInt("##group_index", &selectedGroupIndex, 1.0f, -1, max);
				selectedGroupIndex = std::max(0, selectedGroupIndex);
				/*
				if (ImGui::BeginCombo("##groupindex", std::to_string(selectedGroupIndex).c_str()))
				{
					for (int groupIndex = 0; groupIndex < transition->GetConditionGroups().size(); ++groupIndex)
					{
						if (ImGui::Selectable(
							std::to_string(groupIndex).c_str(),
							selectedGroupIndex == groupIndex))
						{
							selectedGroupIndex = groupIndex;
						}
					}

					ImGui::EndCombo();
				}
				*/

				if (ImGui::Button("Create"))
				{
					//if (transition->myConditions.find(selectedConditionName) == transition->myConditions.end())
					{
						Condition newCond;
						newCond.myValueName = selectedParameterName;
						newCond.myComparison = selectedConditionComparison;
						newCond.myValueType = selectedConditionType;

						switch (selectedConditionType)
						{
						case ConditionValueType::Bool:
							newCond.myCompareValue = compareBoolValue;
							newCond.myValuePointer = myMachine->myBools[selectedParameterName].get();
							break;
						case ConditionValueType::Int:
							newCond.myCompareValue = compareIntValue;
							newCond.myValuePointer = myMachine->myInts[selectedParameterName].get();
							break;
						case ConditionValueType::Float:
							newCond.myCompareValue = compareFloatValue;
							newCond.myValuePointer = myMachine->myFloats[selectedParameterName].get();
							break;
						default:
							break;
						}

						if (selectedGroupIndex >= transition->myConditionGroups.size())
						{
							transition->myConditionGroups.push_back({});
						}

						transition->myConditionGroups[selectedGroupIndex].myConditions.push_back(newCond);

						ImGui::CloseCurrentPopup();
					}
					//else
					//{
					//	// already exists
					//	newConditionFailed = true;
					//}
				}

				if (ImGui::Button("Cancel"))
				{
					ImGui::CloseCurrentPopup();
				}

				if (newConditionFailed)
				{
					ImGui::TextColored(ImVec4(1, 0, 0, 1), "Failed to create, might already exist, or has no name");
				}

				ImGui::EndPopup();
			}
		}

		ImGui::PopID();
	}

	for (auto& nodePtr : myNodes)
	{
		const auto& node = *nodePtr;

		// Assumed to be of type AnimationState
		assert(node.Type == NodeType::State);

		// DO NOT DRAW ANY STATE TO PREVNT USER FROM TOUCHING IT
		if (node.Name == "Any State")
		{
			continue;
		}

		auto state = static_cast<AnimationState*>(nodePtr);

		ImGui::PushID(node.ID.AsPointer());

		const bool isSelected =
			std::find(selectedNodes.begin(), selectedNodes.end(), node.ID) != selectedNodes.end();

		if (isSelected)
		{
			const std::string stateNameBeforeChange = state->myName;

			ImGuiHelper::AlignedWidget("State Name", ourAlignPercent);

			if (ImGui::InputText("##statename", &state->myName))
			{
				// Rename the state in the machine
				if (mySelectedLayer->myStates.find(state->myName) == mySelectedLayer->myStates.end())
				{
					auto findResult = std::find_if(mySelectedLayer->myStates.begin(), mySelectedLayer->myStates.end(),
						[state](const auto& aState)
						{
							return aState.second->ID == state->ID;
						});

					if (findResult != mySelectedLayer->myStates.end())
					{
						AnimationState* stateWeRenamed = findResult->second.get();

						const std::string newName = state->myName;

						auto value = std::move(findResult->second);
						// node name
						value->Name = state->myName;
						mySelectedLayer->myStates.erase(findResult);
						mySelectedLayer->myStates.insert(std::make_pair(state->myName, std::move(value)));

						// Must update the transitions state names when changing name
						for (const auto& innerState : mySelectedLayer->myStates)
						{
							for (auto& transition : innerState.second->myTransitions)
							{
								if (transition->myTargetState == stateWeRenamed)
								{
									// the new name
									transition->myTargetStateName = newName;
								}
							}
						}
					}
					else
					{
						assert(false);
					}
				}
			}

			bool isEntry = false;
			if (mySelectedLayer->HasInitialState())
			{
				isEntry = &mySelectedLayer->GetInitialState() == state;
			}
			/*assert(myMachine->HasInitialState());
			bool isEntry = &myMachine->GetInitialState() == state;*/

			ImGuiHelper::AlignedWidget("Is Entry", ourAlignPercent);

			if (ImGui::Checkbox("##isentry", &isEntry))
			{
				if (isEntry)
				{
					// Make it the entry
					mySelectedLayer->myInitialState = state;

					mySelectedLayer->ResetToInitialState();
				}
				else
				{
					mySelectedLayer->myInitialState = nullptr;
					mySelectedLayer->ResetToInitialState();
				}
			}


			// If no transitions, we can choose to stay when this state
			// finished or not
			if (state->GetTransitions().empty())
			{
				ImGuiHelper::AlignedWidget("Stay when finished", ourAlignPercent);
				ImGui::Checkbox("##staywhenfinished", &state->myStayWhenFinished);
			}

			// fortsätt här, när jag drar en .anim in i denna widget, så funkar det inte ;(

			//AnimationClipRefclip;

			//if (state->myPlayable)
			//{
			//	clip = state->GetPlayable();
			//}

			//if (myEditor.DrawReflectedResource(clip, "Playable", "AnimationClip", ".anim"))
			//{
			//	auto animation = MakeOwned<NormalAnimation>();

			//	animation->myClip = clip;
			//	animation->myName = clip->GetPath();

			//	state->myPlayable = std::move(animation);
			//}

			enum PlayableType
			{
				PlayableType_Animation,
				PlayableType_BlendTree1D,
				PlayableType_BlendTree2D,

				PlayableType_Unknown,
			};

			const char* playableTypes[]
			{
				"Animation",
				"BlendTree 1D",
				"BlendTree 2D"
			};

			PlayableType selectedPlayableType = PlayableType_Unknown;

			if (state->myPlayable != nullptr)
			{
				// ugly, but not required to be pretty since its only in the editor babyyy
				if (dynamic_cast<NormalAnimation*>(state->myPlayable.get()) != nullptr)
				{
					selectedPlayableType = PlayableType_Animation;
				}
				else if (dynamic_cast<BlendTree1D*>(state->myPlayable.get()) != nullptr)
				{
					selectedPlayableType = PlayableType_BlendTree1D;
				}
				else if (dynamic_cast<BlendTree2D*>(state->myPlayable.get()) != nullptr)
				{
					selectedPlayableType = PlayableType_BlendTree2D;
				}
				else
				{
					// unhandled
					assert(false);
				}
			}
			else
			{
				// if not yet initialized, default to an animation
				selectedPlayableType = PlayableType_Animation;
				state->myPlayable = MakeOwned<NormalAnimation>();
			}

			ImGuiHelper::AlignedWidget("Playable Type", ourAlignPercent);

			if (ImGui::BeginCombo("##playabletype", playableTypes[selectedPlayableType]))
			{
				for (int i = 0; i < std::size(playableTypes); ++i)
				{
					if (ImGui::Selectable(playableTypes[i], i == selectedPlayableType))
					{
						selectedPlayableType = static_cast<PlayableType>(i);

						switch (i)
						{
						case PlayableType_Animation:
						{
							state->myPlayable = MakeOwned<NormalAnimation>();
						} break;

						case PlayableType_BlendTree1D:
						{
							state->myPlayable = MakeOwned<BlendTree1D>();
						} break;

						case PlayableType_BlendTree2D:
						{
							state->myPlayable = MakeOwned<BlendTree2D>();
						} break;

						default:
							break;
						}
					}
				}

				ImGui::EndCombo();
			}

			switch (selectedPlayableType)
			{
				// Animation
			case PlayableType_Animation:
			{
				NormalAnimation* animation = dynamic_cast<NormalAnimation*>(state->myPlayable.get());

				DrawAnimationInput(state);

				if (state->myPlayable)
				{
					DrawAnimationProperties(animation);
				}
			} break;

			// Blendtree 1D
			case PlayableType_BlendTree1D:
			{
				DrawBlendTree1DAnimTable(state);
			} break;

			// Blendtree 2D
			case PlayableType_BlendTree2D:
			{
				BlendTree2D* blendtree2d = dynamic_cast<BlendTree2D*>(state->myPlayable.get());

				const std::string& conditionComboPreview = blendtree2d->myBlendVec2Name;

				ImGuiHelper::AlignedWidget("Blend Value", ourAlignPercent);
				if (ImGui::BeginCombo("##Vec2Combo2", conditionComboPreview.c_str()))
				{
					for (const auto& [name, vec2Value] : myMachine->myVec2s)
					{
						if (ImGui::Selectable(name.c_str(), conditionComboPreview == name))
						{
							blendtree2d->myBlendVec2Name = name;
							blendtree2d->myBlendValue = vec2Value.get();
						}
					}

					ImGui::EndCombo();
				}

				myBlendTree2DChildWindow.Draw(*blendtree2d);
			} break;

			default:
				break;
			}
		}

		ImGui::PopID();
	}
}

void Engine::AnimationNodeEditorWindow::DrawParameters()
{
	ImGuiHelper::TextTitle("Triggers");

	DrawTriggers();

	ImGui::Dummy(ImVec2(0.f, 10.f));
	ImGui::Separator();
	ImGui::Dummy(ImVec2(0.f, 10.f));

	ImGuiHelper::TextTitle("Bools");

	DrawBools();

	ImGui::Dummy(ImVec2(0.f, 10.f));
	ImGui::Separator();
	ImGui::Dummy(ImVec2(0.f, 10.f));

	ImGuiHelper::TextTitle("Floats");

	DrawFloats();

	ImGui::Dummy(ImVec2(0.f, 10.f));
	ImGui::Separator();
	ImGui::Dummy(ImVec2(0.f, 10.f));

	ImGuiHelper::TextTitle("Vec2's");

	DrawVec2s();

	ImGui::Dummy(ImVec2(0.f, 10.f));
	ImGui::Separator();
	ImGui::Dummy(ImVec2(0.f, 10.f));

	ImGuiHelper::TextTitle("Ints");

	DrawInts();
}

void Engine::AnimationNodeEditorWindow::DrawTriggers()
{
	static ImGuiTableFlags flags =
		ImGuiTableFlags_SizingFixedFit |
		ImGuiTableFlags_RowBg |
		ImGuiTableFlags_Borders |
		ImGuiTableFlags_Resizable |
		ImGuiTableFlags_Reorderable |
		ImGuiTableFlags_Hideable;

	std::string triggerToRemove;

	if (ImGui::BeginTable("Triggers", 3, flags))
	{
		ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed);
		ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableHeadersRow();

		for (const auto& [name, triggerBool] : myMachine->myTriggers)
		{
			ImGui::TableNextRow();

			for (int column = 0; column < 3; column++)
			{
				ImGui::TableSetColumnIndex(column);

				switch (column)
				{
				case 0:
					ImGui::AlignTextToFramePadding();
					ImGui::Text(name.c_str());
					break;
				case 1:
					if (IsEditingInstance())
					{
						ImGui::PushID(name.c_str());
						if (ImGui::Button("Activate"))
						{
							myMachine->Trigger(name);
						}
						ImGui::PopID();
					}
					else
					{
						ImGui::AlignTextToFramePadding();
						ImGui::TextColored(ImVec4(1, 0, 0, 1), "Must edit instance");
					}
					break;
				case 2:
					ImGui::PushID(name.c_str());
					if (ImGui::Button("Remove"))
					{
						triggerToRemove = name;
					}
					ImGui::PopID();
					break;
				default:
					break;
				}
			}
		}
		ImGui::EndTable();
	}

	if (!triggerToRemove.empty())
	{
		myMachine->RemoveTrigger(triggerToRemove);
	}

	static std::string newTriggerName;
	static bool newTriggerFailed = false;

	if (ImGui::Button("New Trigger"))
	{
		ImGui::OpenPopup("NewTrigger");
		newTriggerName = "";
		newTriggerFailed = false;
	}

	ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_Always);

	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal("NewTrigger"))
	{
		ImGui::InputText("Name", &newTriggerName);

		if (ImGui::Button("Create"))
		{
			if (myMachine->myTriggers.find(newTriggerName) != myMachine->myTriggers.end())
			{
				// already exist
				newTriggerFailed = true;
			}
			else if (newTriggerName.empty())
			{
				// empty name
				newTriggerFailed = true;
			}
			else
			{
				// good
				myMachine->myTriggers.insert(std::make_pair(newTriggerName, MakeOwned<bool>()));

				ImGui::CloseCurrentPopup();
			}
		}

		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}

		if (newTriggerFailed)
		{
			ImGui::TextColored(ImVec4(1, 0, 0, 1), "Failed to create, might already exist, or has no name");
		}

		ImGui::EndPopup();
	}
}

void Engine::AnimationNodeEditorWindow::DrawBools()
{
	static ImGuiTableFlags flags =
		ImGuiTableFlags_SizingFixedFit |
		ImGuiTableFlags_RowBg |
		ImGuiTableFlags_Borders |
		ImGuiTableFlags_Resizable |
		ImGuiTableFlags_Reorderable |
		ImGuiTableFlags_Hideable;

	std::string boolToRemove;

	if (ImGui::BeginTable("Bools", 3, flags))
	{
		ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed);
		ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableHeadersRow();

		for (const auto& [name, _] : myMachine->myBools)
		{
			ImGui::TableNextRow();

			for (int column = 0; column < 3; column++)
			{
				ImGui::TableSetColumnIndex(column);

				switch (column)
				{
				case 0:
					ImGui::AlignTextToFramePadding();
					ImGui::Text(name.c_str());
					break;
				case 1:
				{
					ImGui::PushID(name.c_str());

					if (IsEditingInstance())
					{
						bool value = myMachine->GetBool(name);

						if (ImGui::Checkbox("Value", &value))
						{
							myMachine->SetBool(name, value);
						}
					}
					else
					{
						ImGui::AlignTextToFramePadding();
						ImGui::TextColored(ImVec4(1, 0, 0, 1), "Must edit instance");
					}

					ImGui::PopID();

				} break;
				case 2:
					ImGui::PushID(name.c_str());

					if (ImGui::Button("Remove"))
					{
						boolToRemove = name;
					}

					ImGui::PopID();
					break;
				default:
					break;
				}
			}
		}
		ImGui::EndTable();
	}

	if (!boolToRemove.empty())
	{
		myMachine->RemoveBool(boolToRemove);
	}

	static std::string newBoolName;
	static bool newConditionFailed = false;

	if (ImGui::Button("New Bool"))
	{
		ImGui::OpenPopup("NewBool");
		newBoolName = "";
		newConditionFailed = false;
	}

	ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_Always);

	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal("NewBool"))
	{
		ImGui::InputText("Name", &newBoolName);

		if (ImGui::Button("Create"))
		{
			if (myMachine->myBools.find(newBoolName) != myMachine->myBools.end())
			{
				// already exists
				newConditionFailed = true;
			}
			else if (newBoolName.empty())
			{
				// empty name
				newConditionFailed = true;
			}
			else
			{
				// good
				myMachine->myBools.insert(std::make_pair(newBoolName, MakeOwned<bool>()));

				ImGui::CloseCurrentPopup();
			}
		}

		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}

		if (newConditionFailed)
		{
			ImGui::TextColored(ImVec4(1, 0, 0, 1), "Failed to create, might already exist, or has no name");
		}

		ImGui::EndPopup();
	}
}

void Engine::AnimationNodeEditorWindow::DrawLayers()
{
	if (ImGui::BeginChild("##dds"))
	{
		const bool isModelValid = myMachine->myModel && myMachine->myModel->IsValid();

		if (!isModelValid)
		{
			ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.f, 0.f, 0.f, 1.f));
		}

		if (myEditor.DrawReflectedResource(
			myMachine->myModel,
			"Model",
			DragDropConstants::Model,
			FileType::Extension_Model))
		{
			if (myMachine->myModel && myMachine->myModel->IsValid())
			{
				myMachine->GenerateBindPose(myMachine->myModel->Get());
			}
			else
			{
				myMachine->ClearBindPose();
			}
		}

		if (!isModelValid)
		{
			ImGui::PopStyleColor();
		}

		if (ImGui::BeginListBox("##layerlistbox"))
		{
			for (auto& layer : myMachine->myLayers)
			{
				if (ImGui::Selectable(layer->GetName().c_str(), layer.get() == mySelectedLayer))
				{
					// mySelectedLayer = layer.get();
					myNextSelectedLayer = layer.get();
				}
			}

			ImGui::EndListBox();
		}

		ImGui::SameLine();

		ImGui::BeginGroup();

		if (ImGui::Button("New layer"))
		{
			std::string newLayerName = "New Layer";

			int layerNameCounter = 0;
			while (std::find_if(myMachine->GetLayers().begin(),
				myMachine->GetLayers().end(),
				[newLayerName](const Owned<AnimationMachineLayer>& aLayer)
				{
					return aLayer->GetName() == newLayerName;
				}) != myMachine->GetLayers().end())
			{
				newLayerName = "New Layer " + std::to_string(layerNameCounter);
				++layerNameCounter;
			}

				auto animLayer = MakeOwned<AnimationMachineLayer>(newLayerName);

				myMachine->GetLayers().push_back(std::move(animLayer));

				mySelectedLayer = myMachine->FindLayer(newLayerName);

				SpawnAnyStateNode();
		}

		if (mySelectedLayer != nullptr)
		{
			if (ImGui::Button("Delete Selected"))
			{
				myMachine->RemoveLayer(mySelectedLayer);
				mySelectedLayer = nullptr;

				// These vectors have pointers to the layer states, therefore clear them
				myNodes.clear();
				myLinks.clear();
			}
		}

		ImGui::EndGroup();

		ImGui::Dummy(ImVec2(0.f, 5.f));
		ImGui::Separator();
		ImGui::Dummy(ImVec2(0.f, 5.f));

		if (mySelectedLayer)
		{
			ImGuiHelper::AlignedWidget("Name", ourAlignPercent);

			ImGui::InputText("##layername", &mySelectedLayer->myName);

			ImGuiHelper::AlignedWidget("Type", ourAlignPercent);

			if (ImGui::BeginCombo("##layertype",
				AnimationMachineLayer::TypeToString(mySelectedLayer->myType).c_str()))
			{
				for (int i = 0; i < static_cast<int>(AnimationMachineLayer::Type::Count); ++i)
				{
					const bool isSelected = i == static_cast<int>(mySelectedLayer->myType);

					if (ImGui::Selectable(
						AnimationMachineLayer::TypeToString(
							static_cast<AnimationMachineLayer::Type>(i)).c_str(),
						isSelected))
					{
						mySelectedLayer->myType = static_cast<AnimationMachineLayer::Type>(i);
					}
				}

				ImGui::EndCombo();
			}

			ImGuiHelper::AlignedWidget("Weight", ourAlignPercent);

			ImGui::SliderFloat("##layerweight", &mySelectedLayer->myWeight, 0.f, 1.f);

			ImGuiHelper::AlignedWidget("Enable Mask", ourAlignPercent);

			if (ImGui::Checkbox("##enablemask", &mySelectedLayer->myBlendMask.myIsEnabled))
			{
				if (mySelectedLayer->myBlendMask.myIsEnabled)
				{
					// Just to make it not empty for this checkbox to work
					// mySelectedLayer->myBlendMask.myMask.push_back(false);

					//if (!myMachine->GetFirstAnimationClipPath().empty())
					//{
					//	const auto clip = GResourceManager->CreateRef<AnimationClipResource>(
					//		myMachine->GetFirstAnimationClipPath());

					//	mySelectedLayer->myBlendMask.Init(clip);
					//}

					// The mask must be same size as the bindpose & other clips because we compare against that
					// and expect it to be 1:1
					mySelectedLayer->myBlendMask.Init(myMachine->GetFirstAnimationClip()->Get().GetBoneNodes().size());
				}
				else
				{
					mySelectedLayer->myBlendMask.Clear();
				}
			}

			if (mySelectedLayer->myBlendMask.myIsEnabled)
			{
				// ImGuiHelper::AlignedWidget("Skeleton Reference", ourAlignPercent);
				// 
				// if (myEditor.DrawReflectedResource(
				// 	mySelectedLayer->myBlendMask.myClipForSkeleton,
				// 	"Playable",
				// 	DragDropConstants::AnimationClip,
				// 	FileType::Extension_AnimationClip))
				// {
				// 	// fortsätt här, med att ta bort att man måste ge ett clip vid en mask,
				// 	// 	sedan gör så att denna masken regenas när man sparar
				// 	mySelectedLayer->myBlendMask.Init(mySelectedLayer->myBlendMask.myClipForSkeleton);
				// }

				ImGuiHelper::AlignedWidget("Mask", ourAlignPercent);

				if (ImGui::Button("Edit mask", ImVec2(ImGui::GetContentRegionAvailWidth(), 0.f)))
				{
					ImGui::OpenPopup("Edit mask");
				}
			}

			ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), 0, ImVec2(0.5f, 0.5f));
			ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_Appearing);

			if (ImGui::BeginPopupModal("Edit mask"))
			{
				if (!mySelectedLayer->myBlendMask.IsEmpty())
				{
					// if the model was changed, the mask size might not match the skeleton of the new model
					// therefore, fix this here
					const int boneCount = myMachine->GetFirstAnimationClip()->Get().GetBoneNodes().size();

					if (boneCount != mySelectedLayer->myBlendMask.myMask.size())
					{
						mySelectedLayer->myBlendMask.myMask.resize(boneCount);
					}

					if (ImGui::BeginChild("##bonetree",
						ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y - 30.f)))
					{
						DrawBoneMaskHierarchy();
					}
					ImGui::EndChild();
				}
				else
				{
					ImGui::Text("You must add a clip to the animation machine, otherwise I dont know what skeleton to use!");
				}

				if (ImGui::Button("Close", ImGui::GetContentRegionAvail()))
				{
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
		}
	}
	ImGui::EndChild();
}

void Engine::AnimationNodeEditorWindow::DrawBoneMaskHierarchy()
{
	if (!myMachine->myModel || !myMachine->myModel->IsValid())
	{
		ImGui::Text("Must select a model for the animation machine!");
		return;
	}

	// if (!mySelectedLayer->myBlendMask.myClipForSkeleton ||
	// 	!mySelectedLayer->myBlendMask.myClipForSkeleton->IsValid())
	// {
	// 	ImGui::Text("Must select a skeleton!");
	// 	return;
	// }

	std::vector<TempNode> nodeHierarchy;

	const auto& aBoneNodes = myMachine->GetFirstAnimationClip()->Get().GetBoneNodes();

	nodeHierarchy.resize(aBoneNodes.size());

	// Copy the tree first
	for (size_t i = 0; i < aBoneNodes.size(); ++i)
	{
		nodeHierarchy[i].myNode = &aBoneNodes[i];
		nodeHierarchy[i].myIndex = i;
	}

	for (size_t i = 0; i < aBoneNodes.size(); ++i)
	{
		// Find the children
		for (size_t j = 0; j < aBoneNodes.size(); ++j)
		{
			if (i != j)
			{
				if (aBoneNodes[j].myParentIndex == i)
				{
					nodeHierarchy[i].myChildren.push_back(&nodeHierarchy[j]);
					nodeHierarchy[j].myParent = &nodeHierarchy[i];
				}
			}
		}
	}

	static bool drawFullHierarchy = false;
	ImGui::Checkbox("Draw Full Hierarchy", &drawFullHierarchy);

	// Draw the top level bones
	for (const auto& node : nodeHierarchy)
	{
		if (node.myParent == nullptr)
		{
			DrawBoneMaskBranch(&node, 4.f, drawFullHierarchy);
		}
	}
}

bool Engine::AnimationNodeEditorWindow::IsEditingFile() const
{
	return myMachineFileHolder != nullptr;
}

bool Engine::AnimationNodeEditorWindow::IsEditingInstance() const
{
	return !IsEditingFile();
}

void Engine::AnimationNodeEditorWindow::DrawBoneMaskBranch(
	const TempNode* aNode,
	const float aIndent,
	const bool aDrawFullHierarchy)
{
	// NOTE(filip): a temporary hack, proper way to do this
	// is to just compare with the bone nodes map, but then I
	// need to know what model this animation is for, which I do not :(
	if (!aDrawFullHierarchy)
	{
		if (aNode->myNode->myName.find("$AssimpFbx$") != std::string::npos)
		{
			// If its a assimp node, dont draw it, just continue walkign the tree
			for (const auto* child : aNode->myChildren)
			{
				DrawBoneMaskBranch(child, aIndent, aDrawFullHierarchy);
			}

			return;
		}
	}

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_AllowItemOverlap;

	if (aNode->myChildren.empty())
	{
		flags |= ImGuiTreeNodeFlags_Leaf;
	}

	// Pushing this ID fixes the issue when opening another animation
	// that has the same TreeNode names, then the treenodes stayed open, which was weird
	// now they reset properly
	ImGui::PushID(myMachine);
	const bool open = ImGui::TreeNodeEx(aNode->myNode->myName.c_str(), flags);

	ImGui::SameLine();

	bool v = mySelectedLayer->myBlendMask.IsEnabled(aNode->myIndex);
	ImGui::PushID(aNode->myNode->myName.c_str());
	if (ImGui::Checkbox(("##" + aNode->myNode->myName).c_str(), &v))
	{
		std::stack<const TempNode*> todo;

		todo.push(aNode);

		while (!todo.empty())
		{
			const TempNode* n = todo.top();
			todo.pop();

			for (auto child : n->myChildren)
			{
				todo.push(child);
			}

			mySelectedLayer->myBlendMask.SetIsEnabled(n->myIndex, v);
		}

		// Since assimp is fuckin' ass, I must iterate upwards and find the
		// $AssimpFbx$_PreRotation, etc that is located above this Bone
		const TempNode* n = aNode;

		while (n &&
			n->myNode->myName.find(aNode->myNode->myName) != std::string::npos)
		{
			mySelectedLayer->myBlendMask.SetIsEnabled(n->myIndex, v);

			n = n->myParent;
		}

		int test = 0;
	}
	ImGui::PopID();

	if (open)
	{
		for (const auto* child : aNode->myChildren)
		{
			DrawBoneMaskBranch(child, aIndent, aDrawFullHierarchy);
		}

		ImGui::TreePop();
	}

	ImGui::PopID();
}

void Engine::AnimationNodeEditorWindow::DrawBlendTree1DAnimTable(AnimationState* aState)
{
	auto blendTree = dynamic_cast<BlendTree1D*>(&aState->GetPlayable());

	// must be a NormalAnimation
	assert(blendTree != nullptr);

	ImGui::Text("Min Threshold %f", blendTree->myMinThreshold);
	ImGui::Text("Max Threshold %f", blendTree->myMaxThreshold);
	ImGui::Text("Duration %f sec", blendTree->myDurationSeconds);

	const std::string& conditionComboPreview = blendTree->myBlendFloatName;

	ImGuiHelper::AlignedWidget("Blend Value", ourAlignPercent);
	if (ImGui::BeginCombo("##FloatCombo2", conditionComboPreview.c_str()))
	{
		for (const auto& [name, floatValue] : myMachine->myFloats)
		{
			if (ImGui::Selectable(name.c_str(), conditionComboPreview == name))
			{
				blendTree->myBlendFloatName = name;
				blendTree->myBlendValue = floatValue.get();
			}
		}

		ImGui::EndCombo();
	}

	/*
	ImGui::SliderFloat(
		"##blendvalue",
		&blendTree->myBlendValue,
		blendTree->myMinThreshold,
		blendTree->myMaxThreshold);
	*/

	static ImGuiTableFlags flags =
		ImGuiTableFlags_SizingFixedFit |
		ImGuiTableFlags_Borders;

	int blendNodeIndexToRemove = -1;

	if (ImGui::BeginTable("Animations", 1, flags))
	{
		ImGui::TableSetupColumn("Animations", ImGuiTableColumnFlags_WidthStretch);
		// ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
		// ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableHeadersRow();

		for (int i = 0; i < blendTree->myNodes.size(); ++i)
		{
			auto& blendNode = blendTree->myNodes[i];

			ImGui::TableNextRow();

			for (int column = 0; column < 1; column++)
			{
				ImGui::TableSetColumnIndex(column);

				switch (column)
				{
				case 0:
				{
					// NOTE(filip): atm blend trees only support other animations
					// however, they can support other blend trees as well if we make the
					// editor able to, since it all works on a Playable interface

					static ImDrawListSplitter lol;

					// the table already uses a splitter interally, and imgui does not support
					// recursive splitting, therefore we use our own instance to fix that
					lol.Split(ImGui::GetWindowDrawList(), 2);
					lol.SetCurrentChannel(ImGui::GetWindowDrawList(), 1);

					bool modifiedNode = false;

					ImGui::PushID(&blendNode);

					ImGui::BeginGroup();

					AnimationClipRef clip;

					if (blendNode.GetPlayable())
					{
						if (auto animation = dynamic_cast<NormalAnimation*>(blendNode.GetPlayable()))
						{
							clip = animation->myClip;
						}
						else
						{
							// only supports animation atm
							assert(false);
						}
					}

					if (myEditor.DrawReflectedResource(
						clip,
						"Animation",
						DragDropConstants::AnimationClip,
						FileType::Extension_AnimationClip))
					{
						auto animation = MakeOwned<NormalAnimation>();

						animation->myClip = clip;
						animation->myName = clip->GetPath();

						blendNode.myPlayable = std::move(animation);
						modifiedNode = true;

						// If we have no first anim clip, we must set it, otherwise
						// the animation controller will never update
						if (!myMachine->GetFirstAnimationClip())
						{
							myMachine->myFirstAnimationClip = clip;

							if (myMachine->myModel && myMachine->myModel->IsValid())
							{
								myMachine->GenerateBindPose(myMachine->myModel->Get());
							}
						}
					}

					ImGuiHelper::AlignedWidget("Threshold", ourAlignPercent);
					if (ImGui::DragFloat("##threshold", &blendNode.myThreshold, 0.1f))
					{
					}

					if (ImGui::IsItemDeactivatedAfterEdit())
					{
						// Must do here bcuz it changes order of myNodes in blendtree
						modifiedNode = true;
					}

					if (blendNode.myPlayable)
					{
						if (auto anim = dynamic_cast<NormalAnimation*>(blendNode.myPlayable.get()))
						{
							if (DrawAnimationProperties(anim))
							{
								modifiedNode = true;
							}

							/*ImGuiHelper::AlignedWidget("Loop", ourAlignPercent);

							bool loop = anim->IsLooping();
							if (ImGui::Checkbox("##loop", &loop))
							{
								anim->SetIsLooping(loop);

								modifiedNode = true;
							}

							ImGuiHelper::AlignedWidget("TimeScale", ourAlignPercent);

							float timeScale = anim->GetTimeScale();
							if (ImGui::DragFloat("##TimeScale", &timeScale, 0.01f, 0.f, 100.f))
							{
								anim->SetTimeScale(timeScale);

								modifiedNode = true;
							}*/
						}
					}

					if (ImGui::Button("Remove"))
					{
						blendNodeIndexToRemove = i;
					}

					if (modifiedNode)
					{
						blendTree->RecalculateTree();
					}

					ImGui::EndGroup();

					lol.SetCurrentChannel(ImGui::GetWindowDrawList(), 0);

					auto min = ImGui::GetItemRectMin();
					auto max = ImGui::GetItemRectMax();

					if (clip && clip->IsValid())
					{
						const auto& cellPadding = ImGui::GetStyle().CellPadding;

						min.x -= cellPadding.x;
						min.y -= cellPadding.y;

						max.x += cellPadding.x;
						max.y += cellPadding.y;

						auto color = ImGuiHelper::CalculateColorFromString(clip->GetPath());

						ImGui::GetWindowDrawList()->AddRectFilled(
							min,
							max,
							ImGui::ColorConvertFloat4ToU32(color));
					}

					lol.Merge(ImGui::GetWindowDrawList());

					ImGui::PopID();
				} break;

				default:
					break;
				}
			}
		}
		ImGui::EndTable();
	}

	if (blendNodeIndexToRemove != -1)
	{
		assert(blendNodeIndexToRemove >= 0 && blendNodeIndexToRemove < blendTree->myNodes.size());
		blendTree->myNodes.erase(blendTree->myNodes.begin() + blendNodeIndexToRemove);
	}

	if (ImGui::Button("New blendnode"))
	{
		BlendNode1D node(0.f, nullptr);

		blendTree->AddNode(std::move(node));
	}
}

void Engine::AnimationNodeEditorWindow::DrawAnimationInput(AnimationState* aState)
{
	auto animation = dynamic_cast<NormalAnimation*>(&aState->GetPlayable());

	// must be a NormalAnimation
	assert(animation != nullptr);

	{
		AnimationClipRef clip;

		if (aState->myPlayable)
		{
			clip = animation->myClip;
		}

		if (myEditor.DrawReflectedResource(
			clip,
			"Animation",
			DragDropConstants::AnimationClip,
			FileType::Extension_AnimationClip))
		{
			animation->myClip = clip;
			animation->myName = clip->GetPath();

			// If we have no first anim clip, we must set it, otherwise
			// the animation controller will never update
			if (!myMachine->GetFirstAnimationClip())
			{
				myMachine->myFirstAnimationClip = clip;

				if (myMachine->myModel && myMachine->myModel->IsValid())
				{
					myMachine->GenerateBindPose(myMachine->myModel->Get());
				}
			}
		}
	}
}

void Engine::AnimationNodeEditorWindow::DrawEvents(NormalAnimation* aAnimation)
{
	static ImGuiTableFlags flags =
		ImGuiTableFlags_SizingFixedFit |
		ImGuiTableFlags_Borders;

	int blendNodeIndexToRemove = -1;

	if (ImGui::BeginTable("Animations", 1, flags))
	{
		ImGui::TableSetupColumn("Animations", ImGuiTableColumnFlags_WidthStretch);
		// ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
		// ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableHeadersRow();

		for (int i = 0; i < aAnimation->myEvents.size(); ++i)
		{
			auto& ev = aAnimation->myEvents[i];

			ImGui::TableNextRow();

			for (int column = 0; column < 1; column++)
			{
				ImGui::TableSetColumnIndex(column);

				switch (column)
				{
				case 0:
				{
					// NOTE(filip): atm blend trees only support other animations
					// however, they can support other blend trees as well if we make the
					// editor able to, since it all works on a Playable interface

					static ImDrawListSplitter lol;

					// the table already uses a splitter interally, and imgui does not support
					// recursive splitting, therefore we use our own instance to fix that
					lol.Split(ImGui::GetWindowDrawList(), 2);
					lol.SetCurrentChannel(ImGui::GetWindowDrawList(), 1);

					bool modifiedNode = false;

					ImGui::PushID(&ev);

					ImGui::BeginGroup();

					ImGuiHelper::AlignedWidget("Name", ourAlignPercent);
					ImGui::InputText("##name", &ev.myEventName);

					ImGuiHelper::AlignedWidget("Type", ourAlignPercent);

					const std::string typePreview =
						AnimationEvent::ourTypes[static_cast<int>(ev.myType)];

					if (ImGui::BeginCombo("##type", typePreview.c_str()))
					{
						for (int i = 0; i < std::size(AnimationEvent::ourTypes); ++i)
						{
							if (ImGui::Selectable(
								AnimationEvent::ourTypes[i],
								typePreview == AnimationEvent::ourTypes[i]))
							{
								ev.myType = static_cast<AnimationEvent::Type>(i);
							}
						}

						ImGui::EndCombo();
					}

					switch (ev.myType)
					{
					case AnimationEvent::Type::ByFrame:
					{
						ImGuiHelper::AlignedWidget("Frame", ourAlignPercent);
						ImGui::DragInt("##frame", &ev.myFrame, 1.f, 0, 100000);
					} break;
					case AnimationEvent::Type::ByNormalizedTime:
					{
						ImGuiHelper::AlignedWidget("Normalized Time", ourAlignPercent);
						ImGui::DragFloat("##normtime", &ev.myNormalizedTime, 0.001f, 0.f, 1.f);
					} break;
					default:
						assert(false);
						break;
					}

					if (ImGui::Button("Remove"))
					{
						blendNodeIndexToRemove = i;
					}

					ImGui::EndGroup();

					lol.SetCurrentChannel(ImGui::GetWindowDrawList(), 0);

					auto min = ImGui::GetItemRectMin();
					auto max = ImGui::GetItemRectMax();

					const auto& cellPadding = ImGui::GetStyle().CellPadding;

					min.x -= cellPadding.x;
					min.y -= cellPadding.y;

					max.x += cellPadding.x;
					max.y += cellPadding.y;

					const auto color = ImGuiHelper::CalculateColorFromString(ev.GetName());

					ImGui::GetWindowDrawList()->AddRectFilled(
						min,
						max,
						ImGui::ColorConvertFloat4ToU32(color));

					lol.Merge(ImGui::GetWindowDrawList());

					ImGui::PopID();
				} break;

				default:
					break;
				}
			}
		}
		ImGui::EndTable();
	}

	if (blendNodeIndexToRemove != -1)
	{
		assert(blendNodeIndexToRemove >= 0 && blendNodeIndexToRemove < aAnimation->myEvents.size());
		aAnimation->myEvents.erase(aAnimation->myEvents.begin() + blendNodeIndexToRemove);
	}

	if (ImGui::Button("New event"))
	{
		AnimationEvent newEvent;
		aAnimation->myEvents.push_back(newEvent);
	}
}

bool Engine::AnimationNodeEditorWindow::DrawAnimationProperties(NormalAnimation* aAnimation)
{
	bool anyPropertyModified = false;

	ImGuiHelper::AlignedWidget("Events", ourAlignPercent);

	if (ImGui::Button("Edit Events", ImVec2(ImGui::GetContentRegionAvailWidth(), 0.f)))
	{
		ImGui::OpenPopup("Edit events");
	}

	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), 0, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_Appearing);

	if (ImGui::BeginPopupModal("Edit events"))
	{
		if (ImGui::BeginChild("##eventschild",
			ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y - 30.f)))
		{
			DrawEvents(aAnimation);
		}
		ImGui::EndChild();

		if (ImGui::Button("Close", ImGui::GetContentRegionAvail()))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	ImGuiHelper::AlignedWidget("Loop", ourAlignPercent);

	bool loop = aAnimation->IsLooping();
	if (ImGui::Checkbox("##loop", &loop))
	{
		aAnimation->SetIsLooping(loop);
		anyPropertyModified = true;
	}

	ImGuiHelper::AlignedWidget("TimeScale", ourAlignPercent);

	float timeScale = aAnimation->GetTimeScale();
	if (ImGui::DragFloat("##TimeScale", &timeScale, 0.01f, 0.f, 100.f))
	{
		aAnimation->SetTimeScale(timeScale);
		anyPropertyModified = true;
	}

	ImGuiHelper::AlignedWidget("Root Motion", ourAlignPercent);
	bool rootMotion = aAnimation->IsRootMotion();
	if (ImGui::Checkbox("##rootmotion", &rootMotion))
	{
		aAnimation->SetIsRootMotion(rootMotion);
		anyPropertyModified = true;
	}

	return anyPropertyModified;
}

bool Engine::AnimationNodeEditorWindow::DrawConditionComparisonCombo(ConditionComparison& aComparison)
{
	bool result = false;

	if (ImGui::BeginCombo(
		"##conditioncomparison",
		GlobalConditionComparison[static_cast<int>(aComparison)].c_str()))
	{
		for (int i = 0; i < static_cast<int>(ConditionComparison::Count); ++i)
		{
			if (ImGui::Selectable(
				GlobalConditionComparison[i].c_str(),
				i == static_cast<int>(aComparison)))
			{
				aComparison = static_cast<ConditionComparison>(i);
				result = true;
			}
		}

		ImGui::EndCombo();
	}

	return result;
}

void Engine::AnimationNodeEditorWindow::DrawFloats()
{
	static ImGuiTableFlags flags =
		ImGuiTableFlags_SizingFixedFit |
		ImGuiTableFlags_RowBg |
		ImGuiTableFlags_Borders |
		ImGuiTableFlags_Resizable |
		ImGuiTableFlags_Reorderable |
		ImGuiTableFlags_Hideable;

	std::string floatToRemove;

	if (ImGui::BeginTable("Floats", 3, flags))
	{
		ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed);
		ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableHeadersRow();

		for (const auto& [name, floatValue] : myMachine->myFloats)
		{
			ImGui::TableNextRow();

			for (int column = 0; column < 3; column++)
			{
				ImGui::TableSetColumnIndex(column);

				switch (column)
				{
				case 0:
					ImGui::AlignTextToFramePadding();
					ImGui::Text(name.c_str());
					break;
				case 1:
					if (IsEditingInstance())
					{
						ImGui::PushID(name.c_str());
						ImGui::DragFloat("##floatvalue", floatValue.get(), 0.1f);
						ImGui::PopID();
					}
					else
					{
						ImGui::AlignTextToFramePadding();
						ImGui::TextColored(ImVec4(1, 0, 0, 1), "Must edit instance");
					}
					break;
				case 2:
					ImGui::PushID(name.c_str());
					if (ImGui::Button("Remove"))
					{
						floatToRemove = name;
					}
					ImGui::PopID();
					break;
				default:
					break;
				}
			}
		}
		ImGui::EndTable();
	}

	if (!floatToRemove.empty())
	{
		myMachine->RemoveFloat(floatToRemove);
	}

	static std::string newFloatName;
	static bool newFloatFailed = false;

	if (ImGui::Button("New Float"))
	{
		ImGui::OpenPopup("NewFloat");
		newFloatName = "";
		newFloatFailed = false;
	}

	ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_Always);

	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal("NewFloat"))
	{
		ImGui::InputText("Name", &newFloatName);

		if (ImGui::Button("Create"))
		{
			if (myMachine->myFloats.find(newFloatName) != myMachine->myFloats.end())
			{
				newFloatFailed = true;
			}
			else if (newFloatName.empty())
			{
				newFloatFailed = true;
			}
			else
			{
				myMachine->myFloats.insert(std::make_pair(newFloatName, MakeOwned<float>()));

				ImGui::CloseCurrentPopup();
			}
		}

		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}

		if (newFloatFailed)
		{
			ImGui::TextColored(ImVec4(1, 0, 0, 1), "Failed to create, might already exist, or has no name");
		}

		ImGui::EndPopup();
	}
}

void Engine::AnimationNodeEditorWindow::DrawVec2s()
{
	static ImGuiTableFlags flags =
		ImGuiTableFlags_SizingFixedFit |
		ImGuiTableFlags_RowBg |
		ImGuiTableFlags_Borders |
		ImGuiTableFlags_Resizable |
		ImGuiTableFlags_Reorderable |
		ImGuiTableFlags_Hideable;

	std::string floatToRemove;

	if (ImGui::BeginTable("Vec2s", 3, flags))
	{
		ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed);
		ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableHeadersRow();

		for (const auto& [name, vec2Value] : myMachine->myVec2s)
		{
			ImGui::TableNextRow();

			for (int column = 0; column < 3; column++)
			{
				ImGui::TableSetColumnIndex(column);

				switch (column)
				{
				case 0:
					ImGui::AlignTextToFramePadding();
					ImGui::Text(name.c_str());
					break;
				case 1:
					if (IsEditingInstance())
					{
						ImGui::PushID(name.c_str());
						ImGui::DragFloat2(name.c_str(), &vec2Value.get()->x, 0.1f);
						ImGui::PopID();
					}
					else
					{
						ImGui::AlignTextToFramePadding();
						ImGui::TextColored(ImVec4(1, 0, 0, 1), "Must edit instance");
					}
					break;
				case 2:
					ImGui::PushID(name.c_str());
					if (ImGui::Button("Remove"))
					{
						floatToRemove = name;
					}
					ImGui::PopID();
					break;
				default:
					break;
				}
			}
		}
		ImGui::EndTable();
	}

	if (!floatToRemove.empty())
	{
		myMachine->RemoveFloat(floatToRemove);
	}

	static std::string newVec2Name;
	static bool newVec2Failed = false;

	if (ImGui::Button("New Vec2"))
	{
		ImGui::OpenPopup("NewVec2");
		newVec2Name = "";
		newVec2Failed = false;
	}

	ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_Always);

	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal("NewVec2"))
	{
		ImGui::InputText("Name", &newVec2Name);

		if (ImGui::Button("Create"))
		{
			if (myMachine->myVec2s.find(newVec2Name) != myMachine->myVec2s.end())
			{
				newVec2Failed = true;
			}
			else if (newVec2Name.empty())
			{
				newVec2Failed = true;
			}
			else
			{
				myMachine->myVec2s.insert(std::make_pair(newVec2Name, MakeOwned<Vec2f>()));

				ImGui::CloseCurrentPopup();
			}
		}

		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}

		if (newVec2Failed)
		{
			ImGui::TextColored(ImVec4(1, 0, 0, 1), "Failed to create, might already exist, or has no name");
		}

		ImGui::EndPopup();
	}
}

void Engine::AnimationNodeEditorWindow::DrawInts()
{
	static ImGuiTableFlags flags =
		ImGuiTableFlags_SizingFixedFit |
		ImGuiTableFlags_RowBg |
		ImGuiTableFlags_Borders |
		ImGuiTableFlags_Resizable |
		ImGuiTableFlags_Reorderable |
		ImGuiTableFlags_Hideable;

	std::string floatToRemove;

	if (ImGui::BeginTable("Ints", 3, flags))
	{
		ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed);
		ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableHeadersRow();

		for (const auto& [name, intValue] : myMachine->myInts)
		{
			ImGui::TableNextRow();

			for (int column = 0; column < 3; column++)
			{
				ImGui::TableSetColumnIndex(column);

				switch (column)
				{
				case 0:
					ImGui::AlignTextToFramePadding();
					ImGui::Text(name.c_str());
					break;
				case 1:
					if (IsEditingInstance())
					{
						ImGui::PushID(name.c_str());
						ImGui::DragInt(name.c_str(), &*intValue, 0.1f);
						ImGui::PopID();
					}
					else
					{
						ImGui::AlignTextToFramePadding();
						ImGui::TextColored(ImVec4(1, 0, 0, 1), "Must edit instance");
					}
					break;
				case 2:
					ImGui::PushID(name.c_str());
					if (ImGui::Button("Remove"))
					{
						floatToRemove = name;
					}
					ImGui::PopID();
					break;
				default:
					break;
				}
			}
		}
		ImGui::EndTable();
	}

	if (!floatToRemove.empty())
	{
		myMachine->RemoveInt(floatToRemove);
	}

	static std::string intName2;
	static bool int2Failed = false;

	if (ImGui::Button("New Int"))
	{
		ImGui::OpenPopup("NewInt");
		intName2 = "";
		int2Failed = false;
	}

	ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_Always);

	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal("NewInt"))
	{
		ImGui::InputText("Name", &intName2);

		if (ImGui::Button("Create"))
		{
			if (myMachine->myInts.find(intName2) != myMachine->myInts.end())
			{
				int2Failed = true;
			}
			else if (intName2.empty())
			{
				int2Failed = true;
			}
			else
			{
				myMachine->myInts.insert(std::make_pair(intName2, MakeOwned<int>()));

				ImGui::CloseCurrentPopup();
			}
		}

		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}

		if (int2Failed)
		{
			ImGui::TextColored(ImVec4(1, 0, 0, 1), "Failed to create, might already exist, or has no name");
		}

		ImGui::EndPopup();
	}
}
