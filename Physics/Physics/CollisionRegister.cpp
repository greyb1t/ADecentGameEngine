#include "pch.h"
#include "CollisionRegister.h"

void CollisionRegister::UpdateStates()
{
    for (auto& d : myCollisionsToUpdate) 
    {
        auto& list = myCollisionData[d.uuid];
    
        auto it = std::find_if(list.begin(), list.end(), [d](CollisionData& val) { return val.other == d.collisionUUID; });
    
        if (it == list.end())
            continue;

        if (it->state == eCollisionState::OnEnter) 
        {
            it->state = eCollisionState::OnStay;
            continue;
        }
        if (it->state == eCollisionState::OnExit || it->state == eCollisionState::OnCollision)
        {
            list.erase(it);
            continue;
        }
    }
}

void CollisionRegister::RegisterUUID(PUUID uuid, bool listener)
{
    myCollisionDataListeners[uuid] = listener;
}

void CollisionRegister::UnregisterUUID(PUUID uuid)
{
    myCollisionDataListeners.erase(uuid);
    myCollisionData.erase(uuid);
}

void CollisionRegister::RegisterCollision(PUUID uuid, CollisionData data)
{
    if (!myCollisionDataListeners[uuid])
        return;

    PUUID other = data.a.uuid == uuid ? data.b.uuid : data.a.uuid;
    data.other = other;

    auto& collisions = myCollisionData[uuid];
    for (auto& c : collisions) 
    {
        if (c.other == other) 
        {
            c.state = eCollisionState::OnExit;
            myCollisionsToUpdate.emplace_back(CollisionUpdateData{ uuid, other });
            return;
        }
    }

    data.state = data.type == eCollisionType::DEFAULT ? eCollisionState::OnCollision : eCollisionState::OnEnter;
    collisions.emplace_back(data);
    myCollisionsToUpdate.emplace_back(CollisionUpdateData{ uuid, other });
}

const std::vector<CollisionData>& CollisionRegister::GetData(PUUID uuid)
{
    return myCollisionData[uuid];
}
