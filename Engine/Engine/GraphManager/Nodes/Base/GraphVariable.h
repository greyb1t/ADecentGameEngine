#pragma once
#include "NodeTypes.h"
#include "UID.h"

class GraphVariable
{
	friend class GraphManager;
	friend class GraphInstance;
	friend class Engine::VisualScript;

	UID myUID;
	std::string myName;
	// The currently held value, changed by the graphs flow.
	DataPtr myValue;
	// The value we have at the start of execution.
	DataPtr myStartValue;
	DataType myType;

	GraphVariable() = default;

	GraphVariable(std::string& aName, DataType aType, bool aCreateNewUID = true)
		: myUID(aCreateNewUID), myName(aName), myType(aType)
	{
		myStartValue = DataPtr::Create(myType);
		myValue = DataPtr::Create(myType);
	}

	GraphVariable(std::string& aName, DataType aType, DataPtr aDataPointer, bool aCreateNewUID = true)
		: GraphVariable(aName, aType, aCreateNewUID)
	{
		memcpy(myStartValue.Ptr, aDataPointer.Ptr, aDataPointer.Size);
	}

public:

	template<typename T>
	GraphVariable(std::string& aName, DataType aType, T someData, bool aCreateNewUID = true)
	{
		myUID = UID(aCreateNewUID);
		myName = aName;
		myType = aType;
		myStartValue = DataPtr::Create(myType);
		memcpy(myStartValue.Ptr, &someData, sizeof(T));
	}

	template<>
	GraphVariable(std::string& aName, DataType aType, std::string someData, bool aCreateNewUID)
	{
		myUID = UID(aCreateNewUID);
		myName = aName;
		myType = aType;
		myStartValue = DataPtr::Create(myType, someData.length() + 1);
		memcpy(myStartValue.Ptr, someData.c_str(), someData.length());
	}

	void Initialize()
	{
		if(!myValue)
		{
			myValue = DataPtr::Create(myType, myStartValue.GetSize());
		}

		memcpy(myValue.Ptr, myStartValue.Ptr, myStartValue.GetSize());
	}

	DataType GetType() const { return myType; }

	const UID GetUID() const { return myUID; }

	std::string GetName() const { return myName; }

	template<typename T>
	void Set(T someNewData)
	{
		memcpy(myValue.Ptr, &someNewData, sizeof(T));
	}

	template<>
	void Set(std::string someNewData)
	{
		assert(myValue.GetSize() <= someNewData.length());
		memcpy(myValue.Ptr, someNewData.c_str(), someNewData.length());
	}

	void Set(DataType someType, DataPtr aDataPtr);

	void SetRaw(const char* someData, const size_t someSize);

	template<typename T>
	T Get() const
	{
		return *((T*)myValue);
	}

	template<>
	std::string Get() const
	{
		return std::string(myValue.GetPtr<char>());
	}

	void GetRaw(DataType& outDataType, DataPtr& outDataPtr) const;

	template<typename Writer>
	void Serialize(Writer& writer) const
	{
		writer.StartObject();
		writer.Key("UID");
		writer.Int(myUID.ToUInt());
		writer.Key("Name");
		writer.String(myName.c_str());

		writer.Key("DATA");
		std::vector<char> data((char*)myStartValue.Ptr, (char*)myStartValue.Ptr + myStartValue.Size);
		writer.StartArray();
		for(auto& d : data)
		{
			writer.Int(d);
		}
		writer.EndArray();
		writer.Key("DATA_TYPE");
		writer.Uint((unsigned int)myType);

		writer.EndObject();
	}
};
