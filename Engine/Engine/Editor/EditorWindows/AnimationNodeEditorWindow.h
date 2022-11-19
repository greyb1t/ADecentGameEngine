#pragma once

#include "EditorWindow.h"
#include "Engine/ResourceManagement/ResourceRef.h"

#include "imgui_node_editor.h"
#include "AnimationEditor/BlendTree2DEditor.h"
#include "Engine/Animation/Transitions/Condition.h"

namespace ed = ax::NodeEditor;

namespace Engine
{
	struct OurNode;
	class NormalAnimation;
	class AnimationMachineLayer;
	class AnimationState;
	class AnimationMachine;

	class AnimationNodeEditorWindow : public EditorWindow
	{
	public:
		struct NodeIdLess
		{
			bool operator()(const ed::NodeId& lhs, const ed::NodeId& rhs) const
			{
				return lhs.AsPointer() < rhs.AsPointer();
			}
		};

		// DO NOT MODIFY THE VALUES, THE INTS ARE SAVED INTO JSON
		enum class PinType
		{
			Flow = 0,
			Bool = 1,
			Int = 2,
			Float = 3,
			String = 4,
			Object = 5,
		};

		// DO NOT MODIFY VALUES, INT VALUES ARE SAVED INTO JSON
		enum class PinKind
		{
			Output = 0,
			Input = 1
		};

		// DO NOT MODYIFY THE VALUES, INT VALUE SAVED INTO JSON
		enum class NodeType
		{
			State = 0
		};

		struct Node;

		struct Pin
		{
			ed::PinId ID;
			Node* Node = nullptr;
			std::string Name;
			PinType Type;
			PinKind Kind;

			Pin() = default;

			Pin(int id, const char* name, PinType type)
				: ID(id)
				, Node(nullptr)
				, Name(name)
				, Type(type)
				, Kind(PinKind::Input)
			{
				int test = 0;
			}

			nlohmann::json ToJson() const
			{
				nlohmann::json j;
				j["ID"] = ID.Get();

				// Node = nullptr;
				j["NodeId"] = Node->ID.Get();

				j["Name"] = Name;
				j["Type"] = static_cast<int>(Type);
				j["Kind"] = static_cast<int>(Kind);
				return j;
			}

			void InitFromJson(const nlohmann::json& aJson)
			{
				ID = ed::PinId(aJson["ID"]);

				// Node = nullptr;
				// j["NodeId"] = Node->ID.Get();
				// TODO: fixup the node pointer?
				Node = nullptr;

				Name = aJson["Name"];

				const int type = aJson["Type"];
				Type = static_cast<PinType>(type);

				const int kind = aJson["Kind"];
				Kind = static_cast<PinKind>(kind);
			}
		};

		struct Node
		{
			ed::NodeId ID;
			std::string Name;
			std::vector<Pin> Inputs;
			std::vector<Pin> Outputs;
			ImColor Color;
			NodeType Type;
			ImVec2 Size;
			ImVec2 myPosition;

			Node() = default;

			Node(int id, const char* name, ImColor color = ImColor(255, 255, 255))
				: ID(id)
				, Name(name)
				, Color(color)
				, Type(NodeType::State)
				, Size(0, 0)
			{
			}

			nlohmann::json ToJson() const
			{
				nlohmann::json j;

				j["ID"] = ID.Get();

				j["Name"] = Name;

				auto inputsArrayJson = nlohmann::json::array();

				for (const auto& input : Inputs)
				{
					inputsArrayJson.push_back(input.ToJson());
				}

				j["Inputs"] = inputsArrayJson;

				auto outputsArrayJson = nlohmann::json::array();

				for (const auto& output : Outputs)
				{
					outputsArrayJson.push_back(output.ToJson());
				}

				j["Outputs"] = outputsArrayJson;

				j["Color"] = static_cast<ImU32>(Color);

				j["Type"] = static_cast<int>(Type);

				j["Size"]["x"] = Size.x;
				j["Size"]["y"] = Size.y;

				j["Position"]["x"] = myPosition.x;
				j["Position"]["y"] = myPosition.y;

				return j;
			}

			void InitFromJson2(const nlohmann::json& aJson)
			{
				ID = ed::NodeId(aJson["ID"]);

				Name = aJson["Name"];

				for (const auto& inputJson : aJson["Inputs"])
				{
					Pin pin;
					pin.InitFromJson(inputJson);
					unsigned long long nodeId = inputJson["NodeId"];
					assert(nodeId == ID.Get() && "must be same, because pin has a pointer to its parent");

					// a pointer to the parent node
					pin.Node = this;
					Inputs.push_back(pin);
				}

				for (const auto& outputJson : aJson["Outputs"])
				{
					Pin pin;
					pin.InitFromJson(outputJson);
					unsigned long long nodeId = outputJson["NodeId"];
					assert(nodeId == ID.Get() && "must be same, because pin has a pointer to its parent");

					// a pointer to the parent node
					pin.Node = this;
					Outputs.push_back(pin);
				}

				Color = ImGui::ColorConvertU32ToFloat4(aJson["Color"]);

				const int type = aJson["Type"];
				Type = static_cast<NodeType>(type);

				Size.x = aJson["Size"]["x"];
				Size.y = aJson["Size"]["y"];

				if (aJson.contains("Position"))
				{
					myPosition.x = aJson["Position"]["x"];
					myPosition.y = aJson["Position"]["y"];
				}
			}
		};

		struct Link
		{
			ed::LinkId ID;

			ed::PinId StartPinID;
			ed::PinId EndPinID;

			ImColor Color;

			Link() = default;

			Link(ed::LinkId id, ed::PinId startPinId, ed::PinId endPinId)
				: ID(id)
				, StartPinID(startPinId)
				, EndPinID(endPinId)
				, Color(255, 255, 255)
			{
			}

			void InitFromJson(const nlohmann::json& aJson)
			{
				ID = ed::LinkId(aJson["ID"]);

				StartPinID = ed::PinId(aJson["StartPinID"]);
				EndPinID = ed::PinId(aJson["EndPinID"]);

				Color = ImGui::ColorConvertU32ToFloat4(aJson["Color"]);
			}

			nlohmann::json ToJson() const
			{
				nlohmann::json j;

				j["ID"] = ID.Get();

				j["StartPinID"] = StartPinID.Get();
				j["EndPinID"] = EndPinID.Get();

				j["Color"] = static_cast<ImU32>(Color);

				return j;
			}
		};

	public:
		AnimationNodeEditorWindow(Editor& aEditor);

		virtual ~AnimationNodeEditorWindow();

		bool InitExistingMachine(AnimationMachine* aMachine, const Path& aSavePath);
		bool InitNewMachineInstance(Owned<AnimationMachine> aMachine, const Path& aSavePath);

		ImGuiWindowFlags GetWindowFlags() const override;
		void Draw(const float aDeltaTime) override;

		std::string GetName() const override { return "Animation Node Editor"; }

		void OnEditorChangeMode(const EditorMode aMode) override;

		AnimationMachine* GetMachine();

	private:
		void Application_Frame();

		void ShowLeftPane(float paneWidth);

		void DrawPinIcon(const Pin& pin, bool connected, int alpha);

		ImColor GetIconColor(PinType type);

		void Application_Finalize();

		void Application_Initialize();

		void BuildNodes();

		int GetNextId();

		ed::LinkId GetNextLinkId();

		void TouchNode(ed::NodeId id);

		float GetTouchProgress(ed::NodeId id);

		void UpdateTouch();

		Node* FindNode(ed::NodeId id);

		Link* FindLink(ed::LinkId id);

		Pin* FindPin(ed::PinId id);

		bool IsPinLinked(ed::PinId id) const;

		bool CanCreateLink(Pin* a, Pin* b);

		void BuildNode(Node* node);

		Node* SpawnStateNode(const std::string& aName);
		Node* SpawnAnyStateNode();
		//Node* SpawnEntryStateNode();

		void CreateTransition();

		void ReCreateNodesOfStates();
		void ReCreateStateTransitionsLinks();

		void Save() override;

		// Changes myNextId to a large value enough to continue on the Machine ID counter
		void DetermineNextId();
		void DrawInspection();
		void DrawParameters();
		void DrawTriggers();
		void DrawBools();
		void DrawLayers();
		void DrawBoneMaskHierarchy();
		void DrawFloats();
		void DrawVec2s();
		void DrawInts();

		// If returns false, means we are editing an instance
		bool IsEditingFile() const;
		bool IsEditingInstance() const;

		struct TempNode
		{
			const OurNode* myNode = nullptr;

			int myIndex = -1;

			const TempNode* myParent = nullptr;

			std::vector<const TempNode*> myChildren;
		};
		void DrawBoneMaskBranch(
			const TempNode* aNode, const float aIndent, const bool aDrawFullHierarchy);

		void DrawBlendTree1DAnimTable(AnimationState* aState);
		void DrawAnimationInput(AnimationState* aState);
		void DrawEvents(NormalAnimation* aAnimation);

		// Returns true if any property was modified
		bool DrawAnimationProperties(NormalAnimation* aAnimation);

		bool DrawConditionComparisonCombo(ConditionComparison& aComparison);

	private:
		friend class BlendTree2DEditor;

		// Only holds the machine in memory because if we start editing a file directly
		// we must own our own instance of AnimationMachine
		Owned<AnimationMachine> myMachineFileHolder;

		AnimationMachine* myMachine = nullptr;
		AnimationMachineLayer* mySelectedLayer = nullptr;
		AnimationMachineLayer* myNextSelectedLayer = nullptr;
		AnimationMachineLayer* myPreviousSelectedLayer = nullptr;

		ed::LinkId myPrevSelectedLink;
		ed::NodeId myPrevSelectedNode;
		bool myShowInspectionTab = false;

		// Where to save
		Path mySavePath;

		TextureRef myRestore;
		TextureRef mySave;
		TextureRef myBlueprintBackground;

		const float myTouchTime = 1.0f;
		std::map<ed::NodeId, float, NodeIdLess> myNodeTouchTime;

		// This ID is used for both links, nodes, pins
		int myNextId = 1;

		const int myPinIconSize = 24;
		std::vector<Node*> myNodes;
		// Owned<AnimationState> myAnyStateNode;
		//Owned<AnimationState> myEntryStateNode;

		std::vector<Link*> myLinks;
		ImTextureID myHeaderBackground = nullptr;
		ImTextureID mySaveIcon = nullptr;
		ImTextureID myRestoreIcon = nullptr;

		ed::EditorContext* myNodeEditor = nullptr;

		int mySaveIconWidth = 0;
		int mySaveIconHeight = 0;
		int myRestoreIconWidth = 0;
		int myRestoreIconHeight = 0;

		int myHeaderBackgroundWidth = 0;
		int myHeaderBackgroundHeight = 0;

		bool myAreChangesMadeWithoutSaving = false;

		BlendTree2DEditor myBlendTree2DChildWindow;

		static const inline float ourAlignPercent = 0.3f;
	};

}
