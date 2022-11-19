#pragma once
#include <typeinfo>

#include "Nodes/Base/GraphNodeInstance.h"
#include "Nodes/Base/GraphNodeBase.h"
#include "Nodes/Event/NodeEventStructs.h"

class GraphNodeBase;

class GraphNodeRegistry
{
public:
	// Only called ONCE
	static void PopulateTypes();

	static void RegisterComponentNodes();

	static GraphNodeBase* GetNodeTypeFromID(size_t aTypeId);

	template <class T>
	static size_t GetIdFromType()
	{
		for (auto& type : myTypes)
		{
			if (typeid(type) == typeid(T))
				return type->myID;
		}

		return -1;
	}

	template <class T>
	static GraphNodeBase* GetNodeTypePtrFromType()
	{
		size_t typeID = -1;

		const std::type_info& AType = typeid(T);

		for (auto& [id, type] : myTypes)
		{
			const std::type_info& BType = typeid(*type);
			if (AType == BType)
			{
				typeID = type->myID;
				break;
			}
		}

		if (typeID != -1)
		{
			return GetNodeTypeFromID(typeID);
		}
		return nullptr;
	}

	static std::unordered_map<size_t, GraphNodeBase*>& GetAllNodeTypes();

	static size_t GetNodeTypeCount();

	template <class T, typename ...Args>
	static void RegisterType(const std::string& aName, Args&&... aArgs)
	{
		std::hash<std::string> hasher;

		const size_t hash = hasher(aName);

		myTypes[hash] = DBG_NEW T(std::forward<Args>(aArgs)...);
		myTypes[hash]->myID = hash;
	}

	static void RegisterEvent(const std::string& aEventName, std::vector<DynamicPin>& aPins);

	static void Destroy();

private:
	inline static std::unordered_map<size_t, GraphNodeBase*> myTypes;
};
