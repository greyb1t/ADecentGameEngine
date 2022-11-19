#pragma once
#include "Engine\GameObject\Components\Component.h"
#include "Player/DamageInfo.h"

struct RayCastHit;

class PistolProjectile :
    public Component
{
public:
    PistolProjectile();
    PistolProjectile(GameObject*);
    PistolProjectile(const PistolProjectile&) = default;

    void Setup(const Vec3f& aDir, const DamageInfo& aDamageInfo, float aSpeed, float aRange, float aProjectileLength, VFXRef aHitVFX);
    void Start() override;

    Component* Clone() const override { return DBG_NEW PistolProjectile(*this); }

    void Execute(Engine::eEngineOrder aOrder) override;
    void OnHit(RayCastHit& hit);
    void HitScan();
private:
    Vec3f myDirection;
    float mySpeed = 100;
    float myProjectileLength = 100;
    DamageInfo myDamageInfo;
    float myRange = 5000;
    float myTarget = 5000;
    float myDistanceTraveled = 0;
    Weak<GameObject> myVFXObj;
    Engine::VFXComponent* myVFX = nullptr;

    VFXRef myHitVFX;
};

