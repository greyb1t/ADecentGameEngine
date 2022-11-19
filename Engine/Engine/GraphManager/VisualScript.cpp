#include "pch.h"
#include "VisualScript.h"

#include "GraphInstance.h"
#include "GraphNodeRegistry.h"
#include "Nodes/Base/GraphVariable.h"
#include "rapidjson/document.h"
#include "Nodes/Base/UID.h"
#include <Engine/GameObject/GameObject.h>

Engine::VisualScript::VisualScript()
{
}

Engine::VisualScript::~VisualScript()
{
}

bool Engine::VisualScript::Init(const Path& aPath)
{
	std::ifstream inputFile(aPath.ToWString());

	if (!inputFile.is_open())
	{
		return false;
	}

	std::stringstream jsonDocumentBuffer;
	std::string inputLine;
	while (std::getline(inputFile, inputLine))
	{
		jsonDocumentBuffer << inputLine << "\n";
	}

	myDocument.Parse(jsonDocumentBuffer.str().c_str());

	return true;
}

GraphInstance* Engine::VisualScript::CreateInstance(GameObject* aGameObject)
{
	GraphInstance* graphInstance = DBG_NEW GraphInstance(aGameObject);

	const auto GetNodeFromPinID = [&](unsigned int aID) -> GraphNodeInstance*
	{
		for (auto& nodeInstance : graphInstance->myNodeInstancesInGraph)
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
	};

	graphInstance->myLinks.clear();
	graphInstance->myNodeInstancesInGraph.clear();
	graphInstance->myGraphVariables.clear();

	// must to to avoid invaliding pointers when vector gets resized
	graphInstance->myGraphVariables.reserve(64);

	{

		rapidjson::Value& results = myDocument["Variables"];

		for (rapidjson::SizeType i = 0; i < results.Size(); i++)
		{
			rapidjson::Value& variable = results[i];
			int uid = variable["UID"].GetInt();
			std::string Name = variable["Name"].GetString();

			DataType dataType = DataType::Unknown;
			DataPtr dataPtr;

			rapidjson::Value& variableData = variable["DATA"];
			dataType = (DataType)variable["DATA_TYPE"].GetUint();

			const rapidjson::SizeType dataSize = variableData.Size();
			if (dataSize > 0)
			{
				dataPtr = DataPtr::Create(dataType, dataSize);
				std::vector<char> data(dataSize);
				for (rapidjson::SizeType d = 0; d < dataSize; d++)
				{
					data[d] = (char)variableData[d].GetInt();
				}

				memcpy(dataPtr.Ptr, &data.front(), data.size());
			}

			graphInstance->myGraphVariables.push_back(
				GraphVariable(Name, dataType, dataPtr, false));
			graphInstance->myGraphVariables[graphInstance->myGraphVariables.size() - 1].myUID = uid;
		}
	}

	{
		rapidjson::Value& uidmax = myDocument["UID_MAX"];

		int test = uidmax["Num"].GetInt();
		graphInstance->myMaxUID = test;

		rapidjson::Value& results = myDocument["NodeInstances"];

		for (rapidjson::SizeType i = 0; i < results.Size(); i++)
		{
			rapidjson::Value& nodeInstance = results[i];
			GraphNodeInstance* object = DBG_NEW GraphNodeInstance(graphInstance, false);
			size_t nodeType = nodeInstance["NodeType"].GetUint64();
			int uid = nodeInstance["UID"].GetInt();

			object->myUID = uid;
			object->myNodeType = GraphNodeRegistry::GetNodeTypeFromID(nodeType);
			object->myEditorPos[0] = static_cast<float>(nodeInstance["Position"]["X"].GetInt());
			object->myEditorPos[1] = static_cast<float>(nodeInstance["Position"]["Y"].GetInt());

			object->ConstructUniquePins();

			object->myVariable = nullptr;
			int VUID = nodeInstance["VUID"].GetInt();
			if (VUID >= 0)
			{
				for (int v = 0; v < graphInstance->myGraphVariables.size(); v++)
				{
					if (graphInstance->myGraphVariables[v].myUID.ToInt() == VUID)
					{
						object->myVariable = &graphInstance->myGraphVariables[v];
						object->ChangePinTypes(object->myVariable->GetType());
						break;
					}
				}
			}

			for (unsigned int z = 0; z < nodeInstance["Pins"].Size(); z++)
			{
				int index = nodeInstance["Pins"][z]["Index"].GetInt();

				if (index >= object->myPins.size())
				{
					LOG_ERROR(LogType::Engine) << "Missing graph node pin for: \"" << object->GetNodeName() <<
						"\". Have you removed or changed the order of reflected variables for the specified Graph Node?";

					return nullptr;
				}

				object->myPins[index].UID.SetUID(nodeInstance["Pins"][z]["UID"].GetInt());
				DataType pinDataType = (DataType)nodeInstance["Pins"][z]["DATA_TYPE"].GetUint();

				rapidjson::Value& someData = nodeInstance["Pins"][z]["DATA"];
				const rapidjson::SizeType dataSize = someData.Size();
				if (dataSize > 0)
				{
					object->myPins[index].Data = DataPtr::Create(pinDataType, dataSize);
					std::vector<char> data(dataSize);
					for (rapidjson::SizeType d = 0; d < dataSize; d++)
					{
						data[d] = (char)someData[d].GetInt();
					}

					memcpy(object->myPins[index].Data.Ptr, &data.front(), data.size());
				}

				if (object->myPins[index].DataType == DataType::Unknown)
				{
					object->ChangePinTypes(pinDataType);
				}
			}

			graphInstance->myNodeInstancesInGraph.push_back(object);
		}
	}
	{
		graphInstance->myNextLinkIdCounter = 1000;
		for (rapidjson::SizeType i = 0; i < myDocument["Links"].Size(); i++)
		{
			int id = myDocument["Links"][i]["ID"].GetInt();
			int inputID = myDocument["Links"][i]["Input"].GetInt();
			int Output = myDocument["Links"][i]["Output"].GetInt();

			GraphNodeInstance* firstNode = GetNodeFromPinID(inputID);
			GraphNodeInstance* secondNode = GetNodeFromPinID(Output);

			firstNode->AddLinkToVia(secondNode, inputID, Output, id);
			secondNode->AddLinkToVia(firstNode, Output, inputID, id);

			graphInstance->myLinks.push_back(
				{ ed::LinkId(id), ed::PinId(inputID), ed::PinId(Output) });
			if (graphInstance->myNextLinkIdCounter < id + 1)
			{
				graphInstance->myNextLinkIdCounter = id + 1;
			}
		}
	}

	return graphInstance;
}
