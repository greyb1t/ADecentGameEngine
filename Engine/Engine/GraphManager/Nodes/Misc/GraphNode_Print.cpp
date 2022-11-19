#include "pch.h"
#include "GraphNode_Print.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"
#include <iostream>

GraphNode_Print::GraphNode_Print()
{
	CreatePin("Exec", PinDirection::PinDirection_IN);
	CreatePin("Text", PinDirection::PinDirection_IN, DataType::String);
	CreatePin("Out", PinDirection::PinDirection_OUT);
}

int GraphNode_Print::OnExec(class GraphNodeInstance* aNodeInstance)
{
	// Generic method of getting data from a pin which provides the type, size and raw data
	// pointer of the information stored in the pin.
	DataType outType; 
	DataPtr outPinData;
	size_t outDataSize = 0;

	// Get data on pin index 1, this index is relative to what you push in the constructor
	// This means that Pin 1 is the Text pin. If we have a property connected here then we
	// should read the data from that incoming node instead of the Text field.
	GetPinData(aNodeInstance, 1, outType, outPinData, outDataSize);
	
	// If we got data from GetPinData we need to react to what type it is.
	// Valid types are float, bool int and string.
	if (outDataSize > 0)
	{
		if (outType == DataType::Int)
		{
			// Yay an integer! Get the data and print it
			const int i = outPinData.Get<int>();
			std::cout << "Data: " << i << std::endl;
		}
		else if (outType == DataType::String)
		{
			// Yay a string! Get the data and print it
			const std::string string = outPinData.Get<std::string>();
			std::cout << "Data: " << string << std::endl;
		}
		else if (outType == DataType::Bool)
		{
			// Yay a bool! Get the data and print it
			const bool b = outPinData.Get<bool>();
			std::cout << "Data: " << b << std::endl;
		}
		else if (outType == DataType::Float)
		{
			// Yay a float! Get the data and print it
			const float f = outPinData.Get<float>();
			std::cout << "Data: " << f << std::endl;
		}
	}

	// Since this node is a pass-through, that is it has an outgoing Exec pin we want to
	// send the execution flow onwards to the next node connected on our OUT pin.
	// We know from our ctor that the Exec pin output is Pin idx 2.
	return 2;
}
