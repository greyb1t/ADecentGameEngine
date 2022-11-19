#pragma once
#include "Engine\GameObject\Components\Component.h"
#include "Player/DamageInfo.h"
struct RayCastHit;

class BouncingProjectile :
    public Component
{
public:
    BouncingProjectile(GameObject*);

    void Start() override;
    void Execute(Engine::eEngineOrder aOrder) override;

    void Setup(Vec3f aDir, const DamageInfo& aDamageInfo, int aBounces, float aBounceRange, float aSpeed, float aBounceSpeed, float aRange, VFXRef aProjectileVFX, VFXRef aHitVFX);
private:
    void Init();
    
    void NormalMove();
    void HomingMove();
    void Hit(GameObject* aObj);

    void KillInstance();

    void Bounce(GameObject* aObj);
    GameObject* SearchNextTarget();
private:
    Vec3f myDirection;
    DamageInfo myDamageInfo;
    float mySpeed = 100;
    float myRange = 5000;
    float myDistanceTraveled = 0;

    int myMaxBounces = 5;
    int myBouncesLeft = 0;
    float myBounceRange = 800;
    float myBounceSpeed = 100;

    bool myBouncing = false;

    Weak<GameObject> myTargetObject;

    Weak<GameObject> myVFXObj;
    Engine::VFXComponent* myVFX = nullptr;
    VFXRef myProjectileVFX;
    VFXRef myHitVFX;

    Engine::AudioComponent* myAudioComponent = nullptr;
};

