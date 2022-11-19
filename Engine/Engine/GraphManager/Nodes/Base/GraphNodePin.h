#pragma once
#include "NodeTypes.h"
#include "UID.h"

struct GraphNodePin
{
	GraphNodePin(std::string aText, PinDirection aType = PinDirection::PinDirection_IN, DataType aVarType = DataType::Exec, bool canConstructInPlace = true)
		:Text(aText), IsReadOnly(false)
	{
		DataType = aVarType;
		Direction = aType;
		AllowConstructInPlace = canConstructInPlace;
	}

	GraphNodePin(const GraphNodePin& p2)
	{
		Text = p2.Text;
		DataType = p2.DataType;
		Direction = p2.Direction;
		Data = p2.Data;
		IsReadOnly = p2.IsReadOnly;
		AllowConstructInPlace = p2.AllowConstructInPlace;
		Parent = p2.Parent;
		//myUID.SetUID(p2.myUID.AsInt()); // Cant do this here, copy constructor should create new UID and it will if we dont set it here
	}

	GraphNodePin& operator=(const GraphNodePin& p2)
	{
		Text = p2.Text;
		DataType = p2.DataType;
		Direction = p2.Direction;
		Data = p2.Data;
		IsReadOnly = p2.IsReadOnly;
		AllowConstructInPlace = p2.AllowConstructInPlace;
		Parent = p2.Parent;
		UID.SetUID(p2.UID); // = operator should use the UID the last pin had, no new UID here, only in copy constructor
		return *this;
	}

	~GraphNodePin()
	{
		// Don't destruct parent when we get destroyed.
		Parent = nullptr;
	}

	std::string Text;
	UID UID;
	DataType DataType = DataType::Exec;
	DataPtr Data;
	PinDirection Direction;

	// The Instanced parent node of this pin.
	class GraphNodeInstance* Parent;

	// If this pin can have something connected to it or not.
	// Used in Make nodes, for example.
	bool IsReadOnly;

	// If this pin should draw its edit widget if it's an input
	// pin or not. If True, they will be drawn, if False it'll
	// just appear as a pin connector.
	bool AllowConstructInPlace;
};
