#pragma once

#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h"
#include "NodeEventStructs.h"

class GraphNode_OnInput : public GraphNodeBase
{
public:
	GraphNode_OnInput(const std::string& aEventName, const std::vector<DynamicPin>& aDynamicPins);

	int OnExec(class GraphNodeInstance* aNodeInstance) override;

	virtual std::string GetNodeName() const override { return "On Input"; }

	std::string GetNodeTypeCategory() const override { return "Events"; }

private:
	std::string myEventName;
	std::vector<DynamicPin> myDynamicPins;

	std::map<int, std::string> myKeys{
		{0x01, "Left mouse button"},
		{0x02, "Right mouse button"},
		{0x09, "TAB key"},
		{0x0D, "ENTER key"},
		{0x10, "SHIFT"},
		{0x11, "CTRL"},
		{0x12, "ALT"},
		{0x1B, "ESC"},
		{0x20, "SPACEBAR"},
		{0x25, "LEFT"},
		{0x26, "UP"},
		{0x27, "RIGHT"},
		{0x28, "DOWN"},
		{0x30, "0"},
		{0x31, "1"},
		{0x32, "2"},
		{0x33, "3"},
		{0x35, "4"},
		{0x35, "5"},
		{0x36, "6"},
		{0x37, "7"},
		{0x38, "8"},
		{0x39, "9"},
		{0x41, "A"},
		{0x42, "B"},
		{0x43, "C"},
		{0x44, "D"},
		{0x45, "E"},
		{0x46, "F"},
		{0x47, "G"},
		{0x48, "H"},
		{0x49, "I"},
		{0x4A, "J"},
		{0x4B, "K"},
		{0x4C, "L"},
		{0x4D, "M"},
		{0x4E, "N"},
		{0x4F, "O"},
		{0x50, "P"},
		{0x51, "Q"},
		{0x52, "R"},
		{0x53, "S"},
		{0x54, "T"},
		{0x55, "U"},
		{0x56, "V"},
		{0x57, "W"},
		{0x58, "X"},
		{0x59, "Y"},
		{0x5A, "Z"},

	};
	int myCurrentKey = 0x01;
};