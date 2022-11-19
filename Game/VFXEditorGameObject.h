#pragma once
#include "Engine\GameObject\GameObject.h"

class EditorEvent;

enum class eEmitterType
{
	PARTICLES,
    PHYSICPARTICLES,
    OBJECTS
};

enum eVFXEditorSpace
{
	LocalSpace,
    WorldSpace
};

class PropertyWindow
{
public:
    enum ePropertyType
    {
        None,
	    ParticleSpawn,
        ParticleUpdate,
    };
    void SetProperty(void* aVariableReference, unsigned int aVariableType, EditorEvent* aEvent, ePropertyType aPropType);
    void ClearProperty();

    void Draw();

    void DrawParticle();

    ePropertyType   myPropertyType;
    EditorEvent*    myEvent = nullptr;
    unsigned int    myVariableType;
    void*           myVariable = nullptr;
};

class EditorEvent
{
public:
    EditorEvent();
	virtual bool StartDraw();
    virtual void EndDraw();

    std::string myTitle = "Event";
    char myId[8];
};

class EditorEmitter : public EditorEvent
{
public:
    virtual bool StartDraw() override;
    virtual void EndDraw() override;

    std::string space;
    std::string geometry;
    bool burst = false;
    int emitAmount = 10;
};

class EditorParticles : public EditorEmitter
{
public:
    // Todo: Split this into a single struct and a map for which are which type (Spawn/Update)

    struct Force
    {
        float force = 100.f;
        bool isRandom = false;
    };

    struct Color
    {
        float r, g, b, a;
    };

    struct Variables
    {
        Color color;
        Force force;
        eVFXEditorSpace space;
    } myVariables;

    struct SpawnVariables
    {
        const static inline char* titles[] = {
            "Color",
            "Force",
            "Space",
        };

        enum Enum
	    {
	        Color = (1 << 0),
	        Force = (1 << 1),
	        Space = (1 << 2),
	    };
    };
    struct UpdateVariables
    {
        const static inline char* titles[] = {
            "Color",
            "Force",
            "Space",
        };

	    enum Enum
	    {
	        Color =     (1 << 0),
	        AddForce =  (1 << 1),
	        Space =     (1 << 2),
	    };
    };

    virtual bool StartDraw() override;
    virtual void EndDraw() override;

    void DrawAddVariableSpawn();
    void DrawAddVariableUpdate();
    void DrawSpawnVariables();
    void DrawUpdateVariables();

    void AddVariable(SpawnVariables::Enum aVariable);
    void AddVariable(UpdateVariables::Enum aVariable);

    void DrawVariable(SpawnVariables::Enum aVariable);
    void DrawVariable(UpdateVariables::Enum aVariable);

	std::string texture;

    unsigned int myStartVariables = 0;
    unsigned int myUpdateVariables = 0;
};

class VFXEditorGameObject :
    public GameObject
{
public:
    void AddEvent(EditorEvent* aEvent);

    void Update(float aDeltaTime) override;

    static PropertyWindow& GetPropertyWindow();
private:
    void RenderEvents();
    void DrawAddEventPopup();


    std::vector<EditorEvent*> myEvents;
};

