#include "pch.h"
#include "GraphManager.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/document.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/prettywriter.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include "Nodes/Base/GraphNodeInstance.h"
#include "Nodes/Base/GraphNodeBase.h"
#include <imgui_node_editor.h>
#include "imgui_internal.h"
#include "BlueprintUtilities/Include/ax/Drawing.h"
#include "BlueprintUtilities/Include/ax/Widgets.h"
#include "BlueprintUtilities/Include/Interop.h"
#include "Nodes/Base/NodeTypes.h"
#include <imgui_impl_dx11.h>

#include "Nodes/Base/GraphNodePin.h"
#include "GraphNodeRegistry.h"
#include "Nodes/Misc/GraphNode_Variable.h"
#include "CopyPasteManager.h"
#include "UndoRedoManager.h"
#include "AddGraphNodesCommand.h"
#include "GraphInstance.h"
#include "RemoveGraphNodesCommand.h"
#include "VisualScript.h"
#include "Engine/Engine.h"
#include "Engine/Renderer/Texture/Texture2D.h"
#include "Engine/ResourceManagement/ResourceManager.h"
#include "Engine/ResourceManagement/Resources/TextureResource.h"
#include "Engine/ResourceManagement/Resources/VisualScriptResource.h"
#include "Engine/Editor/Editor.h"
#include "Engine/Editor/EditorWindows/VisualScriptWindow.h"

using namespace rapidjson;
namespace ed = ax::NodeEditor;

using namespace ax::Drawing;

GraphManager::GraphManager(
	Engine::VisualScriptWindow& aVisualGraphWindow,
	GraphInstance* aGraphInstance,
	const Path& aPath)
	: myPath(aPath),
	myVisualGraphWindow(aVisualGraphWindow)
{
	myGraphInstance = aGraphInstance;

	// NOTE(filip): each node graph instance has its own local ID counter
	// to save time, I update the global counter to continue from the
	// local graph instance counter to fix the issues with duplicate ID's
	// this stops working properly if multiple GraphManager's are being used at same time
	UID::myGlobalUID = aGraphInstance->myMaxUID;

	myCopyPasteManager = DBG_NEW CopyPasteManager(*this);
	myUndoRedoManager = DBG_NEW UndoRedoManager(*this);
}

GraphManager::~GraphManager()
{
	delete[] myNewVarName;
	delete[] myMenuSeachField;

	ed::DestroyEditor(myContext);
	myContext = nullptr;
}
void GraphManager::Load()
{
	ed::Config config;
	config.SettingsFile = nullptr;

	myContext = ed::CreateEditor(&config);

	myMenuSeachField = DBG_NEW char[127];
	memset(&myMenuSeachField[0], 0, sizeof(myMenuSeachField));

	myBlueprintBackground = GetEngine().GetResourceManager().CreateRef<Engine::TextureResource>(
		"Assets/Engine/Editor/Graph/BlueprintBackground.dds");
	myBlueprintBackground->Load();
}

//void GraphManager::ReTriggerUpdateringTrees()
//{
//	//Locate start nodes, we support N start nodes, we might want to remove this, as we dont "support" different trees with different starrtnodes to be connected. It might work, might not
//	for (auto& nodeInstance : myGraphInstance->myNodeInstancesInGraph)
//	{
//		if (nodeInstance->myNodeType->IsStartNode() && nodeInstance->myShouldTriggerAgain)
//		{
//			nodeInstance->Enter();
//		}
//	}
//}

//void GraphManager::ReTriggerTree(std::string aSpecificNodeName)
//{
//	// Reset all variables
//	for (auto& var : myGraphInstance->myGraphVariables)
//	{
//		var.Initialize();
//	}
//
//	//Locate start nodes, we support N start nodes, we might want to remove this, as we dont "support" different trees with different starrtnodes to be connected. It might work, might not
//	for (auto& nodeInstance : myGraphInstance->myNodeInstancesInGraph)
//	{
//		if (nodeInstance->myNodeType->IsStartNode())
//		{
//			if (aSpecificNodeName.empty() && nodeInstance->GetNodeName() == "Start"
//				|| nodeInstance->GetNodeName() == aSpecificNodeName)
//			{
//				nodeInstance->Enter();
//			}
//		}
//	}
//}

void GraphManager::ExecuteTreeWithPayload(std::string aStartNodeName, Payload aPayload)
{
	//Locate start nodes, we support N start nodes, we might want to remove this, as we dont "support" different trees with different starrtnodes to be connected. It might work, might not
	for (auto& nodeInstance : myGraphInstance->myNodeInstancesInGraph)
	{
		if (nodeInstance->myNodeType->IsStartNode())
		{
			if (nodeInstance->GetNodeName() == aStartNodeName)
			{
				nodeInstance->EnterStartNode(aPayload);
			}
		}
	}
}

void GraphManager::ExecuteTreeWithPayload(std::tuple<std::string, Payload> aTuple)
{
	// Forward on ahead to the other function
	ExecuteTreeWithPayload(std::get<0>(aTuple), std::get<1>(aTuple));
}

void GraphManager::SaveTreeToFile()
{
	{
		rapidjson::StringBuffer s;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer1(s);

		{
			writer1.StartObject();
			writer1.Key("UID_MAX");

			writer1.StartObject();
			writer1.Key("Num");
			writer1.Int(UID::myGlobalUID);
			writer1.EndObject();

			writer1.Key("NodeInstances");
			writer1.StartArray();
			for (auto& nodeInstance : myGraphInstance->myNodeInstancesInGraph)
			{
				nodeInstance->Serialize(writer1);
			}
			writer1.EndArray();
		}

		{
			writer1.Key("Links");
			writer1.StartArray();
			for (auto& link : myGraphInstance->myLinks)
			{
				writer1.StartObject();
				writer1.Key("ID");
				writer1.Int(static_cast<int>(link.Id.Get()));
				writer1.Key("Input");
				writer1.Int(static_cast<int>(link.InputId.Get()));
				writer1.Key("Output");
				writer1.Int(static_cast<int>(link.OutputId.Get()));
				writer1.EndObject();
			}
			writer1.EndArray();
		}

		{
			writer1.Key("Variables");
			writer1.StartArray();
			for (auto& variable : myGraphInstance->myGraphVariables)
			{
				variable.Serialize(writer1);
			}
			writer1.EndArray();
		}

		writer1.EndObject();

		if (!myPath.ToWString().empty())
		{
			FileIO::RemoveReadOnly(myPath);

			std::ofstream of(myPath.ToWString());

			if (of.is_open())
			{
				of << s.GetString();
			}
			else
			{
				LOG_ERROR(LogType::Editor) << "Failed to save " << myPath;
			}
		}
		else
		{
			LOG_WARNING(LogType::Editor) << "Bruh, what just happened?";
		}
	}
}

std::vector<int> myFlowsToBeShown;
void GraphManager::ShowFlow(GraphInstance* aInstance, int aLinkID)
{
	if (myGraphInstance == aInstance)
	{
		myFlowsToBeShown.push_back(aLinkID);
	}
}

ax::NodeEditor::Detail::EditorContext* GraphManager::GetContext()
{
	return reinterpret_cast<ax::NodeEditor::Detail::EditorContext*>(myContext);
}

std::vector<class GraphNodeInstance*>& GraphManager::GetNodeInstancesInGraph()
{
	return myGraphInstance->myNodeInstancesInGraph;
}

bool GraphManager::IsEditingInstance() const
{
	return myVisualGraphWindow.myInstance == nullptr;
}

bool GraphManager::IsEditingFile() const
{
	return !IsEditingInstance();
}

void DrawPinIcon(const GraphNodePin& pin, bool connected, int alpha)
{
	IconType iconType;
	DataType actualType = pin.DataType;
	if (actualType != DataType::Exec && pin.Parent->GetVariable())
	{
		actualType = pin.Parent->GetVariable()->GetType();
	}

	ImColor color = DataTypeColor::Get(actualType);
	color.Value.w = alpha / 255.0f;

	switch (actualType)
	{
		case DataType::Exec:
			iconType = IconType::Flow;
			break;
		case DataType::Bool:
			iconType = IconType::Circle;
			break;
		case DataType::Int:
			iconType = IconType::Circle;
			break;
		case DataType::Float:
			iconType = IconType::Circle;
			break;
		case DataType::String:
			iconType = IconType::Circle;
			break;
		case DataType::Unknown:
			iconType = IconType::Circle;
			break;
		default:
			iconType = IconType::Circle;
			break;
			return;
	}
	const int s_PinIconSize = 24;
	ax::Widgets::Icon(ImVec2(s_PinIconSize, s_PinIconSize),
		iconType,
		connected,
		color,
		ImColor(32, 32, 32, alpha));
};

GraphNodeInstance* GraphManager::GetNodeFromNodeID(unsigned int aID)
{
	auto it = myGraphInstance->myNodeInstancesInGraph.begin();
	while (it != myGraphInstance->myNodeInstancesInGraph.end())
	{
		if ((*it)->myUID.ToUInt() == aID)
		{
			return *it;
		}
		else
		{
			++it;
		}
	}

	return nullptr;
}

GraphNodeInstance* GraphManager::GetNodeFromPinID(unsigned int aID)
{
	for (auto& nodeInstance : myGraphInstance->myNodeInstancesInGraph)
	{
		std::vector<GraphNodePin>& pins = nodeInstance->GetPins();

		for (auto& pin : pins)
		{
			if (pin.UID.ToUInt() == aID)
			{
				return nodeInstance;
			}
		}
	}
	return nullptr;
}

void GraphManager::DrawTypeSpecificPin(GraphNodePin& aPin, GraphNodeInstance* aNodeInstance)
{
	if (!aPin.AllowConstructInPlace)
	{
		ImGui::PushID(aPin.UID.ToUInt());
		ImGui::PushItemWidth(100.0f);
		DrawPinIcon(aPin, aNodeInstance->IsPinConnected(aPin), 255);
		ImGui::PopItemWidth();
		ImGui::PopID();
	}
	else
	{
		switch (aPin.DataType)
		{
			case DataType::String:
			{
				if (!aPin.Data)
				{
					aPin.Data = DataPtr::Create(aPin.DataType, 128);
				}

				ImGui::PushID(aPin.UID.ToUInt());
				ImGui::PushItemWidth(100.0f);
				if (aNodeInstance->IsPinConnected(aPin))
				{
					DrawPinIcon(aPin, true, 255);
				}
				else
				{
					ImGui::InputText("##edit", (char*)aPin.Data.Ptr, 127);
				}
				ImGui::PopItemWidth();

				ImGui::PopID();
				break;
			}
			case DataType::Int:
			{
				if (!aPin.Data)
				{
					aPin.Data = DataPtr::Create(aPin.DataType);
				}
				int* c = ((int*)aPin.Data.Ptr);
				ImGui::PushID(aPin.UID.ToUInt());
				if (aNodeInstance->IsPinConnected(aPin))
				{
					DrawPinIcon(aPin, true, 255);
				}
				else
				{
					// This vertical is here because if you have started an ImGui::BeginHorizontal then
					// ImGui::SameLine will force a line break and put things on the same vertical line.
					// If you do ImGui::BeginVertical then ImGui::SameLine puts things on the same
					// horizontal line. When you think about it it's somewhat logical but also not :P.
					ImGui::BeginVertical("IntEditWrapper");
					ImGui::PushItemWidth(100.0f);
					ImGui::InputInt("##edit", c);
					ImGui::PopItemWidth();
					ImGui::EndVertical();
				}

				ImGui::PopID();
				break;
			}
			case DataType::Bool:
			{
				if (!aPin.Data)
				{
					aPin.Data = DataPtr::Create(aPin.DataType);
				}
				bool* c = ((bool*)aPin.Data.Ptr);
				ImGui::PushID(aPin.UID.ToUInt());
				ImGui::PushItemWidth(100.0f);
				if (aNodeInstance->IsPinConnected(aPin))
				{
					DrawPinIcon(aPin, true, 255);
				}
				else
				{
					ImGui::Checkbox("##edit", c);
				}
				ImGui::PopItemWidth();

				ImGui::PopID();
				break;
			}
			case DataType::Float:
			{
				if (!aPin.Data)
				{
					aPin.Data = DataPtr::Create(aPin.DataType);
					float* c = ((float*)aPin.Data.Ptr);
					*c = 1.0f;
				}
				float* c = ((float*)aPin.Data.Ptr);
				ImGui::PushID(aPin.UID.ToUInt());
				ImGui::PushItemWidth(70.0f);
				if (aNodeInstance->IsPinConnected(aPin))
				{
					DrawPinIcon(aPin, true, 255);
				}
				else
				{
					ImGui::InputFloat("##edit", c);
				}
				ImGui::PopItemWidth();

				ImGui::PopID();
				break;
			}
			case DataType::Unknown:
			{
				ImGui::PushID(aPin.UID.ToUInt());
				ImGui::PushItemWidth(100.0f);

				int selectedIndex = -1;
				if (ImGui::RadioButton("Bool", false))
				{
					selectedIndex = (int)DataType::Bool;
				}
				if (ImGui::RadioButton("Int", false))
				{
					selectedIndex = (int)DataType::Int;
				}
				if (ImGui::RadioButton("Float", false))
				{
					selectedIndex = (int)DataType::Float;
				}
				if (ImGui::RadioButton("String", false))
				{
					selectedIndex = (int)DataType::String;
				}

				if (selectedIndex != -1)
				{
					GraphNodeInstance* instance = GetNodeFromPinID(aPin.UID.ToUInt());
					instance->ChangePinTypes((DataType)selectedIndex);
				}

				ImGui::PopItemWidth();
				ImGui::PopID();
				break;
			}
			case DataType::Variable:
			{
				break;
			}
			default:
				assert(false
					&& "Don't know how to draw this particular type! Please nodify "
					"GraphManager::DrawTypeSpecificPin");
		}
	}
}

void GraphManager::WillBeCyclic(
	GraphNodeInstance* aFirst, bool& aIsCyclic, GraphNodeInstance* aBase)
{
	if (aIsCyclic)
	{
		return;
	}

	std::vector<GraphNodePin>& pins = aFirst->GetPins();
	for (auto& pin : pins)
	{
		if (pin.Direction == PinDirection::PinDirection_OUT)
		{
			std::vector<GraphNodeEdge*> links = aFirst->GetLinksFromPin(pin.UID.ToUInt());
			if (links.size() == 0)
			{
				return;
			}
			for (auto& link : links)
			{
				GraphNodeInstance* connectedNodeToOutPut = GetNodeFromPinID(link->ToPinID);

				if (connectedNodeToOutPut == aBase)
				{
					aIsCyclic |= true;
					return;
				}
				WillBeCyclic(connectedNodeToOutPut, aIsCyclic, aBase);
			}
		}
	}
}

void GraphManager::PreFrame(float aTimeDelta)
{
	static float timer = 0;
	timer += aTimeDelta;
	auto& io = ImGui::GetIO();

	switch (myNewVarType)
	{
		case DataType::Bool:
			myNewVarTypeLabel = "Boolean";
			break;
		case DataType::Int:
			myNewVarTypeLabel = "Integer";
			break;
		case DataType::Float:
			myNewVarTypeLabel = "Float";
			break;
		case DataType::String:
			myNewVarTypeLabel = "String";
			break;
		default:
			myNewVarTypeLabel = "Select Type...";
			break;
	}

	//ImGui::SetNextWindowSize({ 400, 400 }, ImGuiCond_Once);
	//ImGui::Begin(
	//	"Visual Script Editor",
	//	&myIsOpen,
	//	ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBringToFrontOnFocus);

	// Modal Dialog for handling variables
	{
		ImVec2 pos(io.DisplaySize.x / 2.0f, io.DisplaySize.y / 2.0f);
		ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopupModal("Manage Variables",
			nullptr,
			ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize
			| ImGuiWindowFlags_NoSavedSettings))
		{
			// Adding a new variable
			{
				ImGui::Text("Name");
				ImGui::SameLine();
				ImGui::Dummy({ 5, 0 });
				ImGui::SameLine();
				ImGui::PushItemWidth(120.0f);
				ImGui::InputText("##varName", myNewVarName, 255);
				ImGui::PopItemWidth();

				ImGui::Text("Type");
				ImGui::SameLine();
				ImGui::Dummy({ 15, 0 });
				ImGui::SameLine();
				if (ImGui::BeginCombo("##varType", myNewVarTypeLabel.c_str()))
				{
					if (ImGui::Selectable("Boolean", myNewVarType == DataType::Bool))
					{
						myNewVarType = DataType::Bool;
					}
					if (ImGui::Selectable("Integer", myNewVarType == DataType::Int))
					{
						myNewVarType = DataType::Int;
					}
					if (ImGui::Selectable("Float", myNewVarType == DataType::Float))
					{
						myNewVarType = DataType::Float;
					}
					if (ImGui::Selectable("String", myNewVarType == DataType::String))
					{
						myNewVarType = DataType::String;
					}
					ImGui::EndCombo();
				}

				const float itemSpacing = ImGui::GetStyle().ItemSpacing.x;

				float buttonWidth = 100.0f;
				ImVec2 cursor = ImGui::GetCursorPos();
				cursor.x = ImGui::GetWindowWidth() - (buttonWidth + itemSpacing);
				ImGui::SetCursorPos(cursor);
				ImGui::PushItemWidth(buttonWidth);

				bool varNameExists = false;
				if (strlen(myNewVarName) > 0)
				{
					const std::string currentNewVarName(myNewVarName);
					for (auto& var : myGraphInstance->myGraphVariables)
					{
						if (std::equal(var.myName.begin(),
							var.myName.end(),
							currentNewVarName.begin(),
							currentNewVarName.end(),
							[](char a, char b)
							{
								return toupper(a) == toupper(b);
							}))
						{
							varNameExists = true;
							break;
						}
					}
				}

				const bool isAddEnabled = strlen(myNewVarName) > 0
					&& myNewVarType != DataType::Unknown && !varNameExists;

				if (!isAddEnabled)
				{
					ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.6f);
					ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				}

				if (ImGui::Button("Add", { buttonWidth, 0 }))
				{
					if (strlen(myNewVarName) != 0)
					{
						std::string newVariableName(myNewVarName);
						myGraphInstance->myGraphVariables.push_back(
							GraphVariable(newVariableName, myNewVarType));
						memset(myNewVarName, 0, 255);
						myNewVarType = DataType::Unknown;
					}
				}

				if (!isAddEnabled)
				{
					ImGui::PopItemFlag();
					ImGui::PopStyleVar();
				}

				ImGui::PopItemWidth();
			}

			// Variable list
			{
				ImGui::Text("Variables");
				if (ImGui::ListBoxHeader(
					"##lbVariables", { -FLT_MIN, 5 * ImGui::GetTextLineHeightWithSpacing() }))
				{
					for (int v = 0; v < myGraphInstance->myGraphVariables.size(); v++)
					{
						auto& var = myGraphInstance->myGraphVariables[v];

						ax::Widgets::Icon(ImVec2(24, 24),
							IconType::Circle,
							true,
							DataTypeColor::Get(var.GetType()),
							ImColor(32, 32, 32, 128));
						ImGui::SameLine();
						if (ImGui::Selectable(var.GetName().c_str(), v == mySelectedVariableIndex))
						{
							mySelectedVariableIndex = v;
							myVariableErrorMessage = "";
						}
					}
					ImGui::ListBoxFooter();
				}

				const float itemSpacing = ImGui::GetStyle().ItemSpacing.x;

				float buttonWidth = 100.0f;
				ImVec2 cursor = ImGui::GetCursorPos();
				cursor.x = ImGui::GetWindowWidth() - (buttonWidth + itemSpacing) * 2;
				ImGui::SetCursorPos(cursor);
				ImGui::PushItemWidth(buttonWidth);

				const bool isRemoveEnabled = mySelectedVariableIndex > -1;

				if (!isRemoveEnabled)
				{
					ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.6f);
					ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				}

				if (ImGui::Button("Remove", { buttonWidth, 0 }))
				{
					// We need to check if any nodes use this variable!
					auto& currentVar = myGraphInstance->myGraphVariables[mySelectedVariableIndex];
					bool canDelete = true;
					for (auto& node : myGraphInstance->myNodeInstancesInGraph)
					{
						if (node->GetVariable()
							&& node->GetVariable()->GetUID().ToInt()
							== currentVar.GetUID().ToInt()) // What xP
						{
							myVariableErrorMessage
								= "Cannot delete a variable that is used in the graph!";
							canDelete = false;
							break;
						}
					}

					if (canDelete)
					{
						myGraphInstance->myGraphVariables.erase(
							myGraphInstance->myGraphVariables.begin() + mySelectedVariableIndex);

						mySelectedVariableIndex = -1;
						myVariableErrorMessage = "";
					}
				}

				if (!isRemoveEnabled)
				{
					ImGui::PopItemFlag();
					ImGui::PopStyleVar();
				}

				ImGui::SameLine();
				if (ImGui::Button("Close", { buttonWidth, 0 }))
				{
					myVariableErrorMessage = "";
					memset(myNewVarName, 0, 255);
					myNewVarType = DataType::Unknown;
					mySelectedVariableIndex = -1;
					ImGui::CloseCurrentPopup();
				}

				ImGui::TextColored({ 255, 0, 0, 255 }, myVariableErrorMessage.c_str());
			}

			ImGui::EndPopup();
		}
	}

	// if (ImGui::Button("Retrigger"))
	// {
	// 	ReTriggerTree("Start");
	// }
	// ImGui::SameLine();
	if (ImGui::Button("Save"))
	{
		myLikeToSave = true;
	}
	static bool showFlow = false;
	if (ImGui::Checkbox("Show Flow", &showFlow))
	{
		myLikeToShowFlow = showFlow;
	}
	ImGui::SameLine();

	if (ImGui::Button("Variables"))
	{
		ImGui::OpenPopup("Manage Variables");
	}

	for (auto& nodeInstance : myGraphInstance->myNodeInstancesInGraph)
	{
		nodeInstance->DebugUpdate();
		nodeInstance->VisualUpdate(aTimeDelta);
	}

	//static int outRate = 30;
	//ImGui::SliderInt("Max framerate", &outRate, 0, 100);
	//if (timer > (1.0f / (float)outRate))
	//{
	//	ReTriggerUpdateringTrees();
	//	timer = 0;
	//}

	ed::SetCurrentEditor(myContext);

	const bool c = ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_C), false);
	const bool v = ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_V), false);
	const bool z = ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Z), false);
	const bool y = ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Y), false);
	const bool ctrl = ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_LeftControl));

	if (ctrl && c)
	{
		myCopyPasteManager->CopySelectedNodes();
	}

	if (ctrl && v)
	{
		myCopyPasteManager->PasteNodes();
	}

	if (ctrl && z)
	{
		myUndoRedoManager->Undo();
	}

	if (ctrl && y)
	{
		myUndoRedoManager->Redo();
	}

	ed::Begin("My Editor", ImVec2(0.0, 0.0f));
}

bool ArePinTypesCompatible(GraphNodePin& aFirst, GraphNodePin& aSecond)
{
	if ((aFirst.DataType == DataType::Exec && aSecond.DataType != DataType::Exec))
	{
		return false;
	}
	if ((aSecond.DataType == DataType::Exec && aFirst.DataType != DataType::Exec))
	{
		return false;
	}
	if (aFirst.IsReadOnly == true || aSecond.IsReadOnly == true)
		return false;
	return true;
}

struct SDistBestResult
{
	size_t myScore;
	GraphNodeBase* myInstance;
};

struct less_than_key
{
	inline bool operator()(const SDistBestResult& struct1, const SDistBestResult& struct2)
	{
		return (struct1.myScore < struct2.myScore);
	}
};
struct more_than_key
{
	inline bool operator()(const SDistBestResult& struct1, const SDistBestResult& struct2)
	{
		return (struct1.myScore > struct2.myScore);
	}
};

template <typename T>
size_t uiLevenshteinDistance(const T& source, const T& target)
{
	if (source.size() > target.size())
	{
		return uiLevenshteinDistance(target, source);
	}

	using TSizeType = typename T::size_type;
	const TSizeType min_size = source.size(), max_size = target.size();
	std::vector<TSizeType> lev_dist(min_size + 1);

	for (TSizeType i = 0; i <= min_size; ++i)
	{
		lev_dist[i] = i;
	}

	for (TSizeType j = 1; j <= max_size; ++j)
	{
		TSizeType previous_diagonal = lev_dist[0], previous_diagonal_save;
		++lev_dist[0];

		for (TSizeType i = 1; i <= min_size; ++i)
		{
			previous_diagonal_save = lev_dist[i];
			if (source[i - 1] == target[j - 1])
			{
				lev_dist[i] = previous_diagonal;
			}
			else
			{
				lev_dist[i]
					= std::min(std::min(lev_dist[i - 1], lev_dist[i]), previous_diagonal) + 1;
			}
			previous_diagonal = previous_diagonal_save;
		}
	}

	return lev_dist[min_size];
}

void GraphManager::ConstructEditorTreeAndConnectLinks()
{
	for (auto& nodeInstance : myGraphInstance->myNodeInstancesInGraph)
	{
		if (!nodeInstance->myHasSetEditorPos)
		{
			ed::SetNodePosition(nodeInstance->myUID.ToUInt(),
				ImVec2(nodeInstance->myEditorPos[0], nodeInstance->myEditorPos[1]));
			nodeInstance->myHasSetEditorPos = true;
		}

		// Start drawing nodes.
		ed::PushStyleVar(ed::StyleVar_NodeRounding, 3.0f);
		ed::PushStyleVar(ed::StyleVar_NodePadding, ImVec4(8, 4, 8, 8));
		ed::BeginNode(nodeInstance->myUID.ToUInt());
		ImGui::PushID(nodeInstance->myUID.ToUInt());
		ImGui::BeginVertical("node");

		ImGui::BeginHorizontal("header");
		ImGui::BeginVertical("headerLabel", ImVec2(0, 28));
		ImGui::TextUnformatted(nodeInstance->GetNodeName().c_str());
		ImGui::EndVertical();
		ImGui::Spring(1);
		ImGui::Dummy(ImVec2(0, 28));
		ImGui::Spring(0);
		ImGui::EndHorizontal();
		ax::rect HeaderRect = ImGui_GetItemRect();
		ImGui::Spring(0, ImGui::GetStyle().ItemSpacing.y * 2.0f);

		ImGui::EndVertical();

		std::vector<GraphNodePin*> InputPins;
		std::vector<GraphNodePin*> OutputPins;
		std::vector<GraphNodePin>& nodePins = nodeInstance->GetPins();
		int firstInputExec = -1;
		int firstOutputExec = -1;
		for (size_t p = 0; p < nodePins.size(); p++)
		{
			if (nodePins[p].Direction == PinDirection::PinDirection_IN)
			{
				InputPins.push_back(&nodePins[p]);
				if (nodePins[p].DataType == DataType::Exec && firstInputExec == -1)
					firstInputExec = static_cast<int>(InputPins.size() - 1);
			}
			else
			{
				OutputPins.push_back(&nodePins[p]);
				if (nodePins[p].DataType == DataType::Exec && firstOutputExec == -1)
					firstOutputExec = static_cast<int>(OutputPins.size() - 1);
			}
		}

		ImGui::BeginVertical("Body");

		const bool bDrawExecRow = firstInputExec + firstOutputExec >= -1;

		if (bDrawExecRow)
		{
			ImGui::BeginHorizontal("PinRow_Exec");
			if (firstInputExec > -1)
			{
				GraphNodePin* pin = InputPins[firstInputExec];
				ed::BeginPin(pin->UID.ToUInt(), ed::PinKind::Input);
				DrawPinIcon(*pin, nodeInstance->IsPinConnected(*pin), 255);
				ed::EndPin();
			}
			if (firstOutputExec > -1)
			{
				ImGui::Spring(1.0f);
				ImGui::Dummy({ 75, 0 });

				GraphNodePin* pin = OutputPins[firstOutputExec];
				ImGui::Text(pin->Text.c_str());
				ed::BeginPin(pin->UID.ToUInt(), ed::PinKind::Output);
				DrawPinIcon(*pin, nodeInstance->IsPinConnected(*pin), 255);
				ed::EndPin();
			}
			ImGui::EndHorizontal();
		}

		const size_t numRows
			= InputPins.size() > OutputPins.size() ? InputPins.size() : OutputPins.size();
		for (size_t row = 0; row < numRows; row++)
		{
			std::string horizontalName = std::string("PinRow_") + std::to_string(row);
			ImGui::BeginHorizontal(horizontalName.c_str());

			if (row < InputPins.size() && row != firstInputExec) // Should we draw input?
			{
				GraphNodePin* pin = InputPins[row];

				if (pin->DataType == DataType::Exec)
				{
					ed::BeginPin(pin->UID.ToUInt(), ed::PinKind::Input);
					DrawPinIcon(*pin, nodeInstance->IsPinConnected(*pin), 255);
					ed::EndPin();
				}
				else
				{
					if (nodeInstance->IsPinConnected(*pin) || !pin->AllowConstructInPlace)
					{
						ed::BeginPin(pin->UID.ToUInt(), ed::PinKind::Input);
						DrawTypeSpecificPin(*pin, nodeInstance);
						ed::EndPin();
						ImGui::Text(pin->Text.c_str());
					}
					else
					{
						ImGui::Text(pin->Text.c_str());
						ed::BeginPin(pin->UID.ToUInt(), ed::PinKind::Input);
						DrawTypeSpecificPin(*pin, nodeInstance);
						ed::EndPin();
					}
				}
			}

			const ImVec2 nodeNameSize = ImGui::CalcTextSize(nodeInstance->GetNodeName().c_str());

			float paddingSize = nodeNameSize.x * 0.75f;
			paddingSize = paddingSize > 100.0f ? paddingSize : 100.0f;

			ImGui::Dummy({ paddingSize, 0 });
			ImGui::Spring(1.0f);
			if (row < OutputPins.size() && row != firstOutputExec)
			{
				GraphNodePin* pin = OutputPins[row];
				ImGui::Text(pin->Text.c_str());
				ed::BeginPin(pin->UID.ToUInt(), ed::PinKind::Output);
				DrawPinIcon(*pin, nodeInstance->IsPinConnected(*pin), 255);
				ed::EndPin();
			}

			ImGui::EndHorizontal();
		}

		ImGui::EndVertical();

		ImVec2 NodeBodySize = ImGui::GetItemRectSize();

		HeaderRect
			= ax::rect(HeaderRect.x, HeaderRect.y, static_cast<int>(NodeBodySize.x), HeaderRect.h);
		ed::EndNode();

		if (ImGui::IsItemVisible())
		{
			auto drawList = ed::GetNodeBackgroundDrawList(nodeInstance->myUID.ToUInt());

			const auto halfBorderWidth = ed::GetStyle().NodeBorderWidth * 0.5f;
			auto headerColor = nodeInstance->GetColor();

			//static ImTextureID HeaderTextureId = ImGui_LoadTexture("Sprites/BlueprintBackground.png");

			// Fixed UVs here
			drawList->AddImageRounded(/*HeaderTextureId*/ myBlueprintBackground->Get().GetSRV(),
				to_imvec(HeaderRect.top_left()) - ImVec2(8 - halfBorderWidth, 4 - halfBorderWidth),
				to_imvec(HeaderRect.bottom_right()) + ImVec2(8 - halfBorderWidth, 0),
				ImVec2(0.0f, 0.0f),
				ImVec2(0.99f, 0.1f),
				headerColor,
				ed::GetStyle().NodeRounding,
				1 | 2);

			// Fixed this to align line at bottom of header
			auto headerSeparatorRect
				= ax::rect(HeaderRect.bottom_left(), HeaderRect.bottom_right());
			drawList->AddLine(
				to_imvec(headerSeparatorRect.top_left()) + ImVec2(-(8 - halfBorderWidth), -0.5f),
				to_imvec(headerSeparatorRect.top_right()) + ImVec2((8 - halfBorderWidth), -0.5f),
				ImColor(255, 255, 255, 255),
				1.0f);
		}
		ImGui::PopID();
		ed::PopStyleVar();
	}

	for (auto& linkInfo : myGraphInstance->myLinks)
		ed::Link(linkInfo.Id, linkInfo.InputId, linkInfo.OutputId);

	// Handle creation action, returns true if editor want to create new object (node or link)
	if (ed::BeginCreate())
	{
		ed::PinId inputPinId, outputPinId;
		if (ed::QueryNewLink(&inputPinId, &outputPinId))
		{
			if (inputPinId && outputPinId)
			{
				if (ed::AcceptNewItem())
				{
					GraphNodeInstance* firstNode
						= GetNodeFromPinID(static_cast<int>(inputPinId.Get()));
					GraphNodeInstance* secondNode
						= GetNodeFromPinID(static_cast<int>(outputPinId.Get()));
					assert(firstNode);
					assert(secondNode);

					if (firstNode == secondNode)
					{
						// User trying connect input and output on the same node :/, who does this!?!
						// SetBlueScreenOnUserComputer(true)
					}
					else
					{
						{
							GraphNodePin* firstPin
								= firstNode->GetPinFromID(static_cast<int>(inputPinId.Get()));
							GraphNodePin* secondPin
								= secondNode->GetPinFromID(static_cast<int>(outputPinId.Get()));

							bool canAddlink = true;
							if (firstPin && secondPin)
							{
								if (firstPin->Direction == PinDirection::PinDirection_IN
									&& secondPin->Direction == PinDirection::PinDirection_IN)
								{
									canAddlink = false;
								}
							}

							if (!ArePinTypesCompatible(*firstPin, *secondPin))
							{
								canAddlink = false;
							}

							if (!firstNode->CanAddLink(static_cast<int>(inputPinId.Get())))
							{
								canAddlink = false;
							}
							if (!secondNode->CanAddLink(static_cast<int>(outputPinId.Get())))
							{
								canAddlink = false;
							}

							if (firstNode->HasLinkBetween(static_cast<int>(inputPinId.Get()),
								static_cast<int>(outputPinId.Get())))
							{
								canAddlink = false;
							}

							if (canAddlink)
							{
								if (secondPin->DataType == DataType::Unknown)
								{
									secondNode->ChangePinTypes(firstPin->DataType);
								}

								if (firstNode->IsPinConnected(*firstPin)
									&& firstPin->DataType == DataType::Exec)
								{
									std::vector<GraphNodeEdge*> firstNodeLinks
										= firstNode->GetLinksFromPin(firstPin->UID.ToUInt());
									for (auto firstNodeLink : firstNodeLinks)
									{
										if (firstNodeLink->Target != firstNode)
										{
											firstNodeLink->Target->RemoveLinkToVia(
												firstNode, firstNodeLink->ToPinID);
											firstNode->RemoveLinkToVia(
												firstNodeLink->Target, firstNodeLink->FromPinID);
											for (auto& link : myGraphInstance->myLinks)
											{
												if (link.Id.Get() == firstNodeLink->LinkID)
												{
													myGraphInstance->myLinks.erase(&link);
												}
											}
										}
									}
								}

								if (secondNode->IsPinConnected(*secondPin)
									&& secondPin->DataType == DataType::Exec)
								{
									std::vector<GraphNodeEdge*> secondNodeLinks
										= secondNode->GetLinksFromPin(secondPin->UID.ToUInt());
									for (auto secondNodeLink : secondNodeLinks)
									{
										if (secondNodeLink->Target != secondNode)
										{
											secondNodeLink->Target->RemoveLinkToVia(
												secondNode, secondNodeLink->ToPinID);
											secondNode->RemoveLinkToVia(
												secondNodeLink->Target, secondNodeLink->FromPinID);
											for (auto& link : myGraphInstance->myLinks)
											{
												if (link.Id.Get() == secondNodeLink->LinkID)
												{
													myGraphInstance->myLinks.erase(&link);
												}
											}
										}
									}
								}
								int linkId = myGraphInstance->myNextLinkIdCounter++;
								firstNode->AddLinkToVia(secondNode,
									static_cast<int>(inputPinId.Get()),
									static_cast<int>(outputPinId.Get()),
									linkId);
								secondNode->AddLinkToVia(firstNode,
									static_cast<int>(outputPinId.Get()),
									static_cast<int>(inputPinId.Get()),
									linkId);

								bool aIsCyclic = false;
								WillBeCyclic(firstNode, aIsCyclic, firstNode);
								if (aIsCyclic || !canAddlink)
								{
									firstNode->RemoveLinkToVia(
										secondNode, static_cast<int>(inputPinId.Get()));
									secondNode->RemoveLinkToVia(
										firstNode, static_cast<int>(outputPinId.Get()));
								}
								else
								{
									// Depending on if you drew the new link from the output to the input we need to create the link as the flow FROM->TO to visualize the correct flow
									if (firstPin->Direction == PinDirection::PinDirection_IN)
									{
										myGraphInstance->myLinks.push_back(
											{ ed::LinkId(linkId), outputPinId, inputPinId });
									}
									else
									{
										myGraphInstance->myLinks.push_back(
											{ ed::LinkId(linkId), inputPinId, outputPinId });
									}

									//ReTriggerTree();
								}
							}
						}
					}
				}
			}
		}
	}
	ed::EndCreate();

	// Handle deletion action
	if (ed::BeginDelete())
	{
		// There may be many links marked for deletion, let's loop over them.
		ed::LinkId deletedLinkId;
		while (ed::QueryDeletedLink(&deletedLinkId))
		{
			// If you agree that link can be deleted, accept deletion.
			if (ed::AcceptDeletedItem())
			{
				// Then remove link from your data.
				for (auto& link : myGraphInstance->myLinks)
				{
					if (link.Id == deletedLinkId)
					{
						GraphNodeInstance* firstNode
							= GetNodeFromPinID(static_cast<int>(link.InputId.Get()));
						GraphNodeInstance* secondNode
							= GetNodeFromPinID(static_cast<int>(link.OutputId.Get()));
						assert(firstNode);
						assert(secondNode);

						firstNode->RemoveLinkToVia(
							secondNode, static_cast<int>(link.InputId.Get()));
						secondNode->RemoveLinkToVia(
							firstNode, static_cast<int>(link.OutputId.Get()));

						myGraphInstance->myLinks.erase(&link);

						break;
					}
				}
			}
		}

		std::vector<GraphNodeInstance*> nodeInstancesToRemove;

		ed::NodeId nodeId = 0;
		while (ed::QueryDeletedNode(&nodeId))
		{
			if (ed::AcceptDeletedItem())
			{
				auto it = myGraphInstance->myNodeInstancesInGraph.begin();
				while (it != myGraphInstance->myNodeInstancesInGraph.end())
				{
					if ((*it)->myUID.ToUInt() == nodeId.Get())
					{
						nodeInstancesToRemove.push_back(*it);
						++it;
						// break;
					}
					else
					{
						++it;
					}
				}
			}
		}

		auto cmd = DBG_NEW RemoveGraphNodesCommand(myGraphInstance, nodeInstancesToRemove);
		myUndoRedoManager->ExecuteCommand(cmd);
	}
	ed::EndDelete();

	auto openPopupPosition = ImGui::GetMousePos();
	ed::Suspend();

	if (ed::ShowBackgroundContextMenu())
	{
		ImGui::OpenPopup("Create New Node");
	}
	ed::Resume();

	ed::Suspend();
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));

	if (ImGui::BeginPopup("Create New Node"))
	{
		auto newNodePostion = openPopupPosition;
		const auto& types = GraphNodeRegistry::GetAllNodeTypes();
		unsigned short noOfTypes = GraphNodeRegistry::GetNodeTypeCount();

		std::map<std::string, std::vector<GraphNodeBase*>> cats;

		//for (int i = 0; i < noOfTypes; i++)
		//{
		//	cats[types[i]->GetNodeTypeCategory()].push_back(types[i]);
		//}

		for (const auto& [id, nodeType] : types)
		{
			cats[nodeType->GetNodeTypeCategory()].push_back(nodeType);
		}

		ImGui::PushItemWidth(100.0f);
		ImGui::InputText("##edit", (char*)myMenuSeachField, 127);
		if (mySetSearchFokus)
		{
			ImGui::SetKeyboardFocusHere(0);
		}
		mySetSearchFokus = false;
		ImGui::PopItemWidth();

		if (myMenuSeachField[0] != '\0')
		{

			std::vector<SDistBestResult> distanceResults;
			//for (int i = 0; i < noOfTypes; i++)
			//{
			//	distanceResults.push_back(SDistBestResult());
			//	SDistBestResult& dist = distanceResults.back();
			//	dist.myInstance = types[i];
			//	dist.myScore
			//		= uiLevenshteinDistance<std::string>(types[i]->GetNodeName(), myMenuSeachField);
			//}
			for (const auto& [id, nodeType] : types)
			{
				distanceResults.push_back(SDistBestResult());
				SDistBestResult& dist = distanceResults.back();
				dist.myInstance = nodeType;
				dist.myScore
					= uiLevenshteinDistance<std::string>(nodeType->GetNodeName(), myMenuSeachField);
			}

			std::sort(distanceResults.begin(), distanceResults.end(), less_than_key());

			int firstCost = static_cast<int>(distanceResults[0].myScore);
			for (int i = 0; i < distanceResults.size(); i++)
			{
				if (ImGui::MenuItem(distanceResults[i].myInstance->GetNodeName().c_str()))
				{
					GraphNodeInstance* node = nullptr;

					// WHY THE FUCK DOES THIS INPUT this IN A FUCKIN BOOL ARGUMENT?
					// WHAT THE ACTUAL FUCK?
					node = DBG_NEW GraphNodeInstance(myGraphInstance, this);

					node->myNodeType = distanceResults[i].myInstance;
					node->ConstructUniquePins();
					ed::SetNodePosition(node->myUID.ToUInt(), newNodePostion);
					node->myHasSetEditorPos = true;

					// TODO: do undo command here?
					auto cmd = DBG_NEW AddGraphNodesCommand(myGraphInstance, { node });
					myUndoRedoManager->ExecuteCommand(cmd);

					//myGraphInstance->myNodeInstancesInGraph.push_back(node);
				}
				int distance = static_cast<int>(distanceResults[i].myScore) - firstCost;
				if (distance > 3)
				{
					break;
				}
			}
		}
		else
		{
			for (auto& category : cats)
			{
				std::string theCatName = category.first;
				if (theCatName.empty())
				{
					theCatName = "General";
				}

				if (ImGui::BeginMenu(theCatName.c_str()))
				{
					GraphNodeInstance* node = nullptr;
					for (int i = 0; i < category.second.size(); i++)
					{
						GraphNodeBase* type = category.second[i];

						if (type->IsHiddenFromMenu())
							continue;

						if (ImGui::MenuItem(type->GetNodeName().c_str()))
						{
							node = DBG_NEW GraphNodeInstance(myGraphInstance, this);

							node->myNodeType = type;
							node->ConstructUniquePins();
							ed::SetNodePosition(node->myUID.ToUInt(), newNodePostion);
							node->myHasSetEditorPos = true;

							auto cmd = DBG_NEW AddGraphNodesCommand(myGraphInstance, { node });
							myUndoRedoManager->ExecuteCommand(cmd);
							//myGraphInstance->myNodeInstancesInGraph.push_back(node);
						}
					}
					ImGui::EndMenu();
				}
			}

			if (ImGui::BeginMenu("Variables"))
			{
				GraphNodeInstance* node = nullptr;
				for (size_t i = 0; i < myGraphInstance->myGraphVariables.size(); i++)
				{
					if (ImGui::BeginMenu(myGraphInstance->myGraphVariables[i].GetName().c_str()))
					{
						if (ImGui::MenuItem("Get"))
						{
							node = DBG_NEW GraphNodeInstance(myGraphInstance, this);
							node->myNodeType
								= GraphNodeRegistry::GetNodeTypePtrFromType<GraphNode_Get>();
							node->ConstructUniquePins();
							ed::SetNodePosition(node->myUID.ToUInt(), newNodePostion);
							node->myHasSetEditorPos = true;
							node->myVariable = &myGraphInstance->myGraphVariables[i];
							node->ChangePinTypes(myGraphInstance->myGraphVariables[i].GetType());
							myGraphInstance->myNodeInstancesInGraph.push_back(node);
						}
						if (ImGui::MenuItem("Set"))
						{
							node = DBG_NEW GraphNodeInstance(myGraphInstance, this);
							node->myNodeType
								= GraphNodeRegistry::GetNodeTypePtrFromType<GraphNode_Set>();
							node->ConstructUniquePins();
							ed::SetNodePosition(node->myUID.ToUInt(), newNodePostion);
							node->myHasSetEditorPos = true;
							node->myVariable = &myGraphInstance->myGraphVariables[i];
							node->ChangePinTypes(myGraphInstance->myGraphVariables[i].GetType());
							myGraphInstance->myNodeInstancesInGraph.push_back(node);
						}
						ImGui::EndMenu();
					}
				}
				node = nullptr;
				ImGui::EndMenu();
			}
		}
		ImGui::EndPopup();
	}
	else
	{
		mySetSearchFokus = true;
		memset(&myMenuSeachField[0], 0, sizeof(myMenuSeachField));
	}

	ImGui::PopStyleVar();
	ed::Resume();
}

void GraphManager::PostFram()
{
	if (myLikeToSave)
	{
		myLikeToSave = false;
		SaveTreeToFile();
	}
	if (myLikeToShowFlow)
	{
		for (int i = 0; i < myGraphInstance->myLinks.size(); i++)
		{
			ed::Flow(myGraphInstance->myLinks[i].Id);
		}
	}
	for (auto i : myFlowsToBeShown)
	{
		ed::Flow(i);
	}
	myFlowsToBeShown.clear();
	ed::End();
	ed::SetCurrentEditor(nullptr);

	//if (!myIsOpen)
	//{
	//	myVisualGraphWindow.GetEditor().CloseExtraWindow(myVisualGraphWindow.GetName());
	//}
}
