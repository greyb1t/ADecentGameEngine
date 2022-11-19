#pragma once

//class GraphUIDManager
//{
//public:
//	static unsigned int CreateNewID()
//	{
//		unsigned int id = -1;
//
//		myGlobalUID++;
//		id = myGlobalUID;
//		while (std::find(myAllUIDs.begin(), myAllUIDs.end(), id) != myAllUIDs.end())
//		{
//			// Increment, the ID is already in use.
//			myGlobalUID++;
//			id = myGlobalUID;
//		}
//		myAllUIDs.push_back(id);
//
//		return id;
//	}
//
//	static bool IdExists(const unsigned int aId)
//	{
//		if (std::find(myAllUIDs.begin(), myAllUIDs.end(), aId) != myAllUIDs.end())
//		{
//			return true;
//		}
//
//		return false;
//	}
//
//private:
//	// Instead of having a UID manager we just keep track of all of them in here.
//	inline static std::vector<unsigned int> myAllUIDs;
//	// As well as the next valid ID.
//	inline static unsigned int myGlobalUID;
//};