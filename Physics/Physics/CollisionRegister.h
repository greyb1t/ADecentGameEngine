#pragma once
#include "Types.h"
#include "CollisionData.h"

class CollisionRegister
{
public:
	void			UpdateStates();

	void			RegisterUUID(PUUID uuid, bool listener = false);
	void			UnregisterUUID(PUUID uuid);
	
	void			RegisterCollision(PUUID, CollisionData data);

	const std::vector<CollisionData>& GetData(PUUID uuid);
private:
	struct CollisionUpdateData 
	{
		CollisionUpdateData(PUUID uuid, PUUID collisionUUID) 
		{ this->uuid = uuid; this->collisionUUID = collisionUUID; }
		PUUID uuid; PUUID collisionUUID; 
	};
	std::vector<CollisionUpdateData>						myCollisionsToUpdate;
	
	std::unordered_map<PUUID, bool>							myCollisionDataListeners;
	std::unordered_map<PUUID, std::vector<CollisionData>>	myCollisionData;

};

