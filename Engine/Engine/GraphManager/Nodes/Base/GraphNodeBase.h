#pragma once
#include "NodeTypes.h"
#include "UID.h"
#include "GraphNodePin.h"

namespace Engine 
{
	class GraphNodeReflector;
	class SetPinDataReflector;
	class SetComponentReflector;
}

/**
 * Base class for all graph nodes regardless of type.
 */
class GraphNodeBase
{
	friend class GraphNodeRegistry;
	friend class GraphNodeInstance;
	friend class Engine::GraphNodeReflector;
	friend class Engine::SetPinDataReflector;
	friend class Engine::SetComponentReflector;

	size_t myID = static_cast<size_t>(-1);
	std::vector<GraphNodePin> myPins;

public:

	virtual ~GraphNodeBase();

	FORCEINLINE virtual std::string GetNodeName() const { return "N/A"; }
	FORCEINLINE virtual std::string GetInstanceName(const GraphNodeInstance* anInstance) const { anInstance;  return GetNodeName(); }
	FORCEINLINE virtual bool IsStartNode() const { return false; }
	FORCEINLINE virtual std::string GetNodeTypeCategory() const { return ""; }

	std::vector<GraphNodePin> GetPins() { return myPins; }

	/**
	 * Node Entry point. This is when we get activated.
	 * @param prevNode The Node that called this function.
	 * @returns Pin ID to continue execution flow on, or -1 if we have no such pin to use.
	 */
	int Exec(class GraphNodeInstance* prevNode);

	int ExecWithPayload(class GraphNodeInstance* prevNode, Payload& aPayload);
	
	bool IsFlowNode();
	
	// Draw debug? Draw text? Use it for something handy.
	virtual void DebugUpdate(class GraphNodeInstance*) {}

	__forceinline size_t GetId() const { return myID; }
	__forceinline bool IsHiddenFromMenu() const { return myHiddenFromMenu; }
	
protected:
	bool myHiddenFromMenu = false;

	/**
	 * Event triggered when the node is entered. This is what should be overriden to do Node work.
	 */
	virtual int OnExec(class GraphNodeInstance* aNodeInstance);

	/**
	 * Specific event used by Start nodes that can have input data.
	 */
	virtual int OnExecWithPayload(class GraphNodeInstance* aNodeInstance, Payload& aPayload);

	/**
	 * Creates a new pin of the specified type.
	 * @param aLabel The label of this pin, if applicable.
	 * @param aDirection If this pin is an In or Out pin.
	 * @param aPinType Type of this pin, also controls its data type.
	 * @param isReadOnly If True this pin can only be used to construct a type, not be connected to anything.
	 * @param allowConstructInPlace If True this pin will show input controls if it's not connected.
	 */
	void CreatePin(std::string aLabel, PinDirection aDirection, DataType aPinType = DataType::Exec,
	               bool isReadOnly = false, bool allowConstructInPlace = true);

	template<typename T>
	T GetPinData(class GraphNodeInstance* aNode, unsigned int aPinIndex)
	{
		DataPtr ptr;
		aNode->FetchData(aPinIndex, ptr);
		return *((T*)ptr.Ptr);
	}

	/**
	 * Overload to handle strings since ImGui doesn't :P
	 */
	template<>
	std::string GetPinData(GraphNodeInstance* aNode, unsigned int aPinIndex)
	{
		DataType outType;
		DataPtr outPtr;
		size_t outSize;

		GetPinData(aNode, aPinIndex, outType, outPtr, outSize);

		std::string str = std::string((char*)outPtr.Ptr);
		return str;
	}

	void GetPinData(GraphNodeInstance* aNode, unsigned int aPinIndex, DataType& outType, DataPtr& someData, size_t& outSize) const;

	/**
	 * Sets the provided data on the specified pin.
	 * Creates the data object on the pin if it's not already present.
	 * @param aNodeInstance The Node Instance to set the data on
	 * @param aPinIndex The index of the pin on the Node Instance to set the data.
	 * @param someData The data itself.
	 */
	template<typename T>
	void SetPinData(GraphNodeInstance* aNodeInstance, unsigned int aPinIndex, T someData)
	{		
		GraphNodePin& pin = aNodeInstance->myPins[aPinIndex];
		if(!pin.Data)
		{
			pin.Data = DataPtr::Create(pin.DataType);
		}
		memcpy(pin.Data.Ptr, &someData, sizeof(T));
	}

	/**
	 * Overload to handle strings since ImGui doesn't without an addon :P
	 */
	template<>
	void SetPinData(GraphNodeInstance* aNodeInstance, unsigned int aPinIndex, std::string someData)
	{
		char* terminatedData = DBG_NEW char[someData.length() + 1]();
		memcpy(terminatedData, someData.c_str(), someData.length());

		SetPinDataRaw(aNodeInstance, aPinIndex, terminatedData, someData.length() + 1);
	}

	/**
	 * Allows setting of raw data on nodes, useful for i.e. std::string can so on (called by specific SetPinData overload)
	 * @param aNodeInstance The Node Instance to set the data on
	 * @param aPinIndex The index of the pin on the Node Instance to set the data.
	 * @param someRawData The data itself.
	 * @param someDataSize The length of the data ptr.
	 */
	void SetPinDataRaw(GraphNodeInstance* aNodeInstance, unsigned int aPinIndex, const char* someRawData, size_t someDataSize);
};