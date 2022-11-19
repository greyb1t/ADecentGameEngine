#pragma once
#include <cassert>

/**
 * Simple Unique ID for Graph Nodes
 */
class UID
{
	friend class GraphManager;
	friend class Engine::VisualScript;

	// Instead of having a UID manager we just keep track of all of them in here.
	//static std::vector<unsigned int> myAllUIDs;
	//// As well as the next valid ID.
	static unsigned int myGlobalUID;

	// My ID
	unsigned int myID;

public:
	UID(bool aCreateNewUID = true)
	{
		if (aCreateNewUID)
		{
			myGlobalUID++;
			myID = myGlobalUID;
			//while (std::find(myAllUIDs.begin(), myAllUIDs.end(), myID) != myAllUIDs.end())
			//{
			//	// Increment, the ID is already in use.
			//	myGlobalUID++;
			//	myID = myGlobalUID;
			//}
			//myAllUIDs.push_back(myID);
		}
	}

	operator int() const { return myID; }
	operator unsigned int() const { return myID; }

	int ToInt() const { return myID; }
	unsigned int ToUInt() const { return myID; }

	UID& operator=(const UID& other)
	{
//  Don't allow assignment of UIDs tha don't exist.
//#ifdef _DEBUG
//		if (std::find(myAllUIDs.begin(), myAllUIDs.end(), myID) != myAllUIDs.end())
//		{
//			assert(0);
//		}
//#endif
		myID = other.myID;
		return *this;
	}
	UID& operator=(const int other)
	{
// Don't allow assignment of UIDs tha don't exist.
//#ifdef _DEBUG
//		if (std::find(myAllUIDs.begin(), myAllUIDs.end(), myID) != myAllUIDs.end())
//		{
//			assert(0);
//		}
//#endif
		myID = other;
		return *this;
	}

	void SetUID(unsigned int aID)
	{
		myID = aID;
	}
};
