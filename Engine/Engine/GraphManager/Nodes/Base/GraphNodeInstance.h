#pragma once
#include "NodeTypes.h"
#include "GraphNodeBase.h"
#include "UID.h"
#include <vector>
#include <any>
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <sstream>

#define UNDEFINED_PIN_ID INT_MAX

// Helpers macros to generate 32-bits encoded colors
#define COL32_R_SHIFT    0
#define COL32_G_SHIFT    8
#define COL32_B_SHIFT    16
#define COL32_A_SHIFT    24
#define COL32_A_MASK     0xFF000000
#define COL32(R,G,B,A)    (((unsigned int)(A)<<COL32_A_SHIFT) | ((unsigned int)(B)<<COL32_B_SHIFT) | ((unsigned int)(G)<<COL32_G_SHIFT) | ((unsigned int)(R)<<COL32_R_SHIFT))
#define COL32_WHITE       COL32(255,255,255,255)  // Opaque white = 0xFFFFFFFF
#define COL32_BLACK       COL32(0,0,0,255)        // Opaque black
#define COL32_BLACK_TRANS COL32(0,0,0,0)          // Transparent black = 0x00000000

namespace Engine
{
	class VisualScript;
}

class GraphVariable;

struct GraphNodeEdge
{
	GraphNodeEdge(class GraphNodeInstance* aLink, unsigned int aFrom, unsigned int aTo, unsigned int aLinkID)
		:Target(aLink), FromPinID(aFrom), ToPinID(aTo), LinkID(aLinkID)
	{

	}
	class GraphNodeInstance* Target = nullptr;
	unsigned int FromPinID = UNDEFINED_PIN_ID;
	unsigned int ToPinID = UNDEFINED_PIN_ID;
	unsigned int LinkID = 0;
};

class GraphInstance;
class GameObject;

class GraphNodeInstance
{
	friend class GraphManager;
	friend class GraphNodeBase;
	friend class CopyPasteManager;
	friend class AddGraphNodesCommand;
	friend class RemoveGraphNodesCommand;
	friend class GraphInstance;
	friend class Engine::VisualScript;


	class GraphNodeBase* myNodeType;
	UID myUID;

	GraphInstance* myGraphInstance = nullptr;

	// We have a lot of private things here accessed by the Graph Manager
	// A lot of this functionality could be in the Graph Manager itself but
	// it would require a refactor to be a little more sensible. This is on
	// the list of things to do. Don't worry if you feel that this is messy
	// I'm quite well aware it is and I've not had time to refactor since I
	// started :P.

	float myEditorPos[2];
	bool myHasSetEditorPos = false;
	std::unordered_map<std::string, std::any> myMetaData;
	std::vector<GraphNodeEdge> myLinks;
	std::vector<GraphNodePin> myPins;
	GraphVariable* myVariable = nullptr;
	float myEnteredTimer = 0.0f;

	void ChangePinTypes(DataType aType);
	void ConstructUniquePins();

	void Enter();
	void EnterStartNode(Payload& aPayload);

	bool CanAddLink(unsigned int aPinIdFromMe);
	bool HasLinkBetween(unsigned int aFirstPin, unsigned int aSecondPin);
	bool AddLinkToVia(GraphNodeInstance* aLink, unsigned int aPinIdFromMe, unsigned int aPinIdToMe, unsigned int aLinkID);
	void RemoveLinkToVia(GraphNodeInstance* aLink, unsigned int aPinThatIOwn);

	std::string WriteVariableType(const GraphNodePin& aPin) const;

	GraphNodePin* GetPinFromID(unsigned int aUID);
	int GetPinIndexFromPinUID(unsigned int aPinUID);


	template <typename Writer>
	inline void WritePinValue(Writer& writer, const GraphNodePin& aPin) const
	{
		if (aPin.Direction == PinDirection::PinDirection_OUT)
		{
			writer.StartArray();
			writer.EndArray();
		}
		else
		{
			writer.StartArray();
			const char* ptr = aPin.Data.GetPtr<char>();
			std::vector<char> data(ptr, ptr + aPin.Data.GetSize());
			for (auto& d : data)
			{
				writer.Int(d);
			}
			writer.EndArray();
		}
	}

	template <typename Writer>
	void Serialize(Writer& writer) const
	{
		writer.StartObject();
		writer.Key("NodeType");
		writer.Uint64(myNodeType->myID);

		writer.Key("UID");
		writer.Int(myUID.ToUInt());

		writer.Key("VUID");
		if (myVariable)
		{
			writer.Int(myVariable->GetUID().ToUInt());
		}
		else
		{
			writer.Int(-1);
		}

		ImVec2 peditorPos = ed::GetNodePosition(myUID.ToUInt());
		writer.Key("Position");
		writer.StartObject();
		{
			writer.Key("X");
			writer.Int(static_cast<int>(peditorPos.x));
			writer.Key("Y");
			writer.Int(static_cast<int>(peditorPos.y));
		}
		writer.EndObject();

		writer.Key("Pins");
		writer.StartArray();
		for (int i = 0; i < myPins.size(); i++)
		{
			writer.StartObject();
			{
				writer.Key("Index");
				writer.Int(i);
				writer.Key("UID");
				writer.Int(myPins[i].UID.ToUInt());
				writer.Key("DATA");
				WritePinValue(writer, myPins[i]);
				writer.Key("DATA_TYPE");
				writer.Uint((unsigned int)myPins[i].DataType);
			}
			writer.EndObject();

		}
		writer.EndArray();


		writer.EndObject();
	}

	void DebugUpdate();
	void VisualUpdate(float aTimeDelta);
	bool FetchData(unsigned int aPinIndex, DataPtr& aResult);
	void FetchDataRaw(DataType& outType, DataPtr& someData, size_t& outSize,
		unsigned aPinToFetchFrom);

	virtual int GetColor()
	{
		if (myEnteredTimer > 0.0f)
		{
			return COL32(0, std::min(myEnteredTimer * 255.f, 255.f), 0, 255);
		}
		if (myNodeType->IsStartNode())
		{
			return COL32(255, 128, 128, 255);
		}
		if (myNodeType->IsFlowNode())
		{
			return COL32(128, 195, 248, 255);
		}
		return COL32(255, 255, 255, 255);
	}

public:

	explicit GraphNodeInstance(GraphInstance* aGraphInstance, bool aCreateNewUID = true);
	virtual ~GraphNodeInstance();

	bool IsPinConnected(GraphNodePin& aPin);
	void ExitVia(unsigned int aPinIndex);
	std::vector< GraphNodeEdge*> GetLinksFromPin(unsigned int aPinToFetchFrom);

	__forceinline const GraphVariable* GetVariable() const { return myVariable; };

	template<typename T>
	void SetVariable(T someData)
	{
		myVariable->Set(someData);
	}

	void SetVariable(DataType someType, DataPtr aDataPtr);

	void SetVariableRaw(const char* someData, const size_t someSize);

	GameObject* GetGameObject();
	GraphInstance* GetGraphInstance() { return myGraphInstance; };

	__forceinline std::string GetNodeName() const { return myNodeType->GetInstanceName(this); }
	__forceinline std::vector<GraphNodePin>& GetPins() { return myPins; }
	__forceinline const GraphNodePin* GetPin(unsigned int aPinIndex) const { return &myPins[aPinIndex]; }
	__forceinline const std::vector<GraphNodeEdge>& GetLinks() const { return myLinks; }
};

