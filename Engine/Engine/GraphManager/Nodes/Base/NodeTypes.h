#pragma once
#include <cassert>
#include "imgui.h"

namespace Engine
{
	class VisualScript;
}

/**
 * Defines for all DataTypes that can exist in the Graph.
 * This also specifies the type of a Pin on a Node.
 */
enum class DataType
{
	Bool,
	Int,
	Float,
	String,
	UUID,

	// When adding new types do it BEFORE this line
	// and DO NOT change their order.
	Variable = 253,
	Exec = 254,
	Unknown = 255,
};

/**
 * Pins should have different colors so it's easy for the user to identify
 * what goes where. When adding new types it's a good idea to give them a
 * unique color as well. They go in this struct.
 */
struct DataTypeColor
{
	static ImColor Get(DataType aType)
	{
		switch (aType)
		{
			case DataType::Bool:		return ImColor(220, 48, 48);
			case DataType::Int:			return ImColor(68, 201, 156);
			case DataType::Float:		return ImColor(147, 226, 74);
			case DataType::String:		return ImColor(124, 21, 153);
			// Add your type above with some fancy color if you don't want
			// it to just appear as white.
			case DataType::Exec:		return ImColor(255, 255, 255);
			case DataType::Unknown:		return ImColor(255, 0, 0);
			default:					return ImColor(100, 100, 100);
		}
	}
};

/**
 * If a pin is an Input or Output pin.
 * Not a bool for readability.
 */
enum class PinDirection
{
	PinDirection_IN,
	PinDirection_OUT
};

/**
 * A data pointer. Responsible for containing data held by node pins and graph variables.
 */
struct DataPtr
{
	/**
	 * Creates a new data pointer of the specified type and the specified size.
	 * @param aType The DataType of this container.
	 * @param someSize The size of the data. This only has an effect when creating
	 * strings or raw pointers.
	 * @returns A new Data container of the specified type and size.
	 */
	static DataPtr Create(DataType aType, size_t someSize = 1)
	{
		DataPtr result;

		switch(aType)
		{
		case DataType::Bool:
			result.Ptr = malloc(sizeof(bool));
			::new (result.Ptr) bool();
			result.Size = sizeof(bool);
			break;
		case DataType::Int:
			result.Ptr = malloc(sizeof(int));
			::new (result.Ptr) int();
			result.Size = sizeof(int);
			break;
		case DataType::Float:
			result.Ptr = malloc(sizeof(float));
			::new (result.Ptr) float();
			result.Size = sizeof(float);
			break;
		case DataType::String:
			result.Ptr = malloc(sizeof(char) * 128);
			::new (result.Ptr) char[sizeof(char) * 128]();
			result.Size = sizeof(char) * 128;
			break;

		// Add your own types above this line.
		case DataType::Exec:
		case DataType::Unknown:
		case DataType::Variable:
			assert(false && "You cannot create data pointers of non-quantifiable types!");
			break;
		default:
			assert(false && "Unknown data type! Do you need to update NodeTypes.h?");
			break;
		}

		result.Type = aType;
		//memset(result.Ptr, 0, result.Size);

		return result;
	}

	void Destroy()
	{
		if(Ptr)
		{
			free(Ptr);
			Ptr = nullptr;
		}

		Size = 0;
		Type = DataType::Unknown;
	}

	/*****************************************************************************
	 *
	 * You shouldn't need to modify anything below this line.
	 *
	 ****************************************************************************/

	// Use the DataPtr::Create method to populate.
	DataPtr() = default;
	~DataPtr() { Destroy(); }

	DataPtr(const DataPtr& aPtr)
	{
		Size = aPtr.Size;
		Type = aPtr.Type;
		if(aPtr.GetType() < DataType::Variable)
		{			
			assert(Size > 0);			
			Ptr = malloc(Size);
			memset(Ptr, 0, Size);
			memcpy(Ptr, aPtr.Ptr, aPtr.Size);
		}

	}

	DataPtr& operator=(const DataPtr& aPtr)
	{
		Size = aPtr.Size;
		Type = aPtr.Type;

		if(aPtr.GetType() < DataType::Variable)
		{
			if(this != &aPtr)
			{
				if(Ptr)
				{
					free(Ptr);
				}

				Ptr = malloc(Size);
				memset(Ptr, 0, Size);
				memcpy(Ptr, aPtr.Ptr, aPtr.Size);
			}
		}

		return *this;
	}

	// This is more efficient than having two functions.
	// https://docs.microsoft.com/en-us/cpp/cpp/move-constructors-and-move-assignment-operators-cpp
	DataPtr(DataPtr&& aPtr) noexcept
	{
		*this = std::move(aPtr);
	}

	DataPtr& operator=(DataPtr&& aPtr) noexcept
	{
		if(this == &aPtr)
			return *this;

		std::swap(this->Ptr, aPtr.Ptr);
		std::swap(this->Size, aPtr.Size);
		std::swap(this->Type, aPtr.Type);
		aPtr.Destroy();

		return *this;
	}
	
	__forceinline size_t GetSize() const { return Size; }
	__forceinline DataType GetType() const { return Type; }

	template<class T>
	__forceinline T Get()
	{
#if _DEBUG
		assert(Ptr);
#endif
		if (!Ptr)
		{
			return T();
		}
		return *static_cast<T*>(Ptr);
	}

	template<>
	__forceinline std::string Get()
	{
#if _DEBUG
		assert(Ptr);
#endif
		if(!Ptr)
		{
			return std::string();
		}
		return std::string(static_cast<char*>(Ptr));
	}

	template<typename T>
	__forceinline const T* GetPtr() const
	{
		return (T*)Ptr;
	}

	bool operator!() const
	{
		return !Ptr;
	}

	operator bool() const
	{
		return Ptr;
	}

	bool operator== (const DataPtr& anOther) const
	{
		return Ptr == anOther.Ptr && Size == anOther.Size;
	}

	bool operator== (const std::nullptr_t) const
	{
		return Ptr == nullptr;
	}

	bool operator!=(const std::nullptr_t) const
	{
		return Ptr != nullptr;
	}

	bool operator!=(const DataPtr& anOther) const
	{
		return Ptr != anOther.Ptr || Size != anOther.Size;
	}

	// Only internal classes may mess with the pointer.
	// We do NOT want to allow external things to modify
	// it in any fashion.
	friend class GraphNodeBase;
	friend class GraphManager;
	friend class GraphVariable;
	friend struct Payload;
	friend class GraphInstance;
	friend class Engine::VisualScript;

private:
	void* Ptr = nullptr;
	size_t Size = 0;
	DataType Type = DataType::Unknown;
};

/**
 * Used to provide input data for a Start node that requires it.
 * Example will appear in F3 :).
 */
struct Payload
{
private:
	std::vector<DataPtr> myData;

public:
	Payload() = default;

	template<DataType D, typename T>
	void AddData(T someData)
	{
		myData.push_back(DataPtr::Create(D));
		memcpy(myData[myData.size() - 1].Ptr, &someData, sizeof(someData));
	}

	template<>
	void AddData<DataType::String>(std::string& someData)
	{
		myData.push_back(DataPtr::Create(DataType::String, someData.size()));
		memcpy(myData[myData.size() - 1].Ptr, someData.c_str(), someData.length());
	}

	size_t Size() const { return myData.size(); }

	template<typename T>
	T GetEntry(unsigned int anIndex) const
	{
		assert(myData.size() > anIndex);
		return *static_cast<const T*>(myData[anIndex].Ptr);
	}

	~Payload()
	{
		for(auto& data : myData)
		{
			data.Destroy();
		}
	}
};