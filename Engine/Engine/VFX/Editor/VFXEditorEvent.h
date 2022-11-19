#pragma once
#include "Common/Enums.hpp"
#include "Engine/VFX/Time/Timeline.h"

enum class eEditorEventType
{
	ParticleEmitter,
    PhysicsParticleEmitter,
    PhysicsObjectEmitter,
};

class VFXEditorEvent
{
public:
    virtual bool StartDraw();
    virtual void EndDraw();

    virtual void DrawEventHeader();

    void SetKey(Timeline::Key aKey)
    {
        myKey = aKey;
    }
    const Timeline::Key& GetKey() const
    {
        return myKey;
    }

    eEditorEventType GetType() const;

    bool GetDelete() const;
protected:
    void Setup();

    Timeline::Key myKey;
    std::string myTitle = "Event";
    char myId[8] = "none";
    eEditorEventType myType;
    bool myIsDeleted = false;
};
