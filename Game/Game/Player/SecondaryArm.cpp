#include "pch.h"
#include "SecondaryArm.h"
#include "Player/Player.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"
#include "Engine/GameObject/Components/VFXComponent.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"

void SecondaryArm::Init(Player* aPlayer) 
{
    myPlayer = aPlayer;

	myForearm = aPlayer->GetGameObject()->GetScene()->AddGameObject<GameObject>();
	myForearm->SetName("Player Left Arm");

    myArmBoneIndex = myPlayer->GetAnimatorComponent().GetController().GetBoneIndexFromName("LeftForeArm");
    myHandBoneIndex = myPlayer->GetAnimatorComponent().GetController().GetBoneIndexFromName("LeftHand");
}

void SecondaryArm::Update()
{
    UpdatePosition();
    
    if (myIsActive)
        UpdateCharge();
}

void SecondaryArm::ActivateSecondary()
{
    VFXRef chargeupVFX = myPlayer->GetVFXHolder().secondary.chargeUp;
    if (chargeupVFX && chargeupVFX->IsValid())
    {
        auto obj = myPlayer->GetGameObject()->GetScene()->AddGameObject<GameObject>();
        obj->GetTransform().SetParent(&myForearm->GetTransform());
        obj->GetTransform().SetPositionLocal({0,0,0});

        mySecondaryChargeVFX = obj->AddComponent<Engine::VFXComponent>(chargeupVFX->Get());
        mySecondaryChargeVFX->Play();
        mySecondaryChargeVFX->AutoDestroy();
    }

    if (mySecondaryChargeVFX)
        mySecondaryChargeVFX->Play();
}

void SecondaryArm::DisableArm()
{
    if (!mySecondaryChargeVFX)
        return;

    mySecondaryChargeVFX->ForceStop();
    mySecondaryChargeVFX->GetGameObject()->Destroy();

    mySecondaryChargeVFX = nullptr;
}

GameObject* SecondaryArm::GetForearm() const
{
    return myForearm;
}

void SecondaryArm::UpdatePosition()
{
    Mat4f boneTrans = myPlayer->GetAnimatorComponent().GetController().GetBoneTransformWorld(myHandBoneIndex);
    Vec3f translation;
    Quatf rotation;
    Vec3f scale;
    boneTrans.Decompose(translation, rotation, scale);

    const Vec3f targetPos = translation;// +myPlayer->GetTransform().GetMovement();
    //const Vec3f lerpPos = C::Lerp(myForearm->GetTransform().GetPosition(), targetPos, Time::DeltaTime * 20);
    myForearm->GetTransform().SetRotation(rotation);
    const Vec3f offset = myForearm->GetTransform().Forward() * myPlayer->GetSettings().leftHand.offset.z
        + myForearm->GetTransform().Up() * myPlayer->GetSettings().leftHand.offset.y
        + myForearm->GetTransform().Right() * myPlayer->GetSettings().leftHand.offset.x;
    myForearm->GetTransform().SetPosition(targetPos + offset);
}

void SecondaryArm::UpdateCharge()
{

}
