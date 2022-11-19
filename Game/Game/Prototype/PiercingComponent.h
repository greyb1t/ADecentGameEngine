#pragma once
#include "Engine/GameObject/Components/Component.h"
#include "Player/DamageInfo.h"
#include <Player/SkillStats.h>
struct RayCastHit;

class PiercingComponent :
    public Component
{
public:
    PiercingComponent();
    PiercingComponent(GameObject*);
    PiercingComponent(const PiercingComponent&) = default;
    Component* Clone() const override { return DBG_NEW PiercingComponent(*this); }

    void Start() override;
    void Setup(Vec3f aDir, const DamageInfo& aDamageInfo, float aDmgRadius, float aSpeed, float aRange, float aExplosionRange, float aExplosionDamageMultiplier, VFXRef aProjectileVFX, VFXRef aHitVFX, VFXRef aExplosionVFX, GameObjectPrefabRef aExplosionPrefab, AnimationCurveRef aExplosionSizeCurve, float aExplosionTime);
    void Execute(Engine::eEngineOrder aOrder) override;
    void OnHit(RayCastHit& hit);
private:
    void Init();

    void ScanDamage(float aDistance);
    void Explode();
    void KillInstance();
private:
    std::vector<int> myHits;

    Weak<GameObject> myVFXObj;
    Engine::VFXComponent* myVFX = nullptr;
    float mySpeed = 100;
    float myDamageRadius = 25;
    float myRange = 5000;
    float myExplosionRange = 200;
    float myExplosionDamageMultiplier = 1;
    float myExplosionKnockback = 1;
    float myDistanceTraveled = 0;
    float myExplosionTime;
    VFXRef myProjectileVFX;
    VFXRef myHitVFX;
    VFXRef myExplosionVFX;
    GameObjectPrefabRef myExplosionPrefab;
    AnimationCurveRef myExplosionSizeCurve;
    Vec3f myDirection;
    DamageInfo myDamageInfo;
};

