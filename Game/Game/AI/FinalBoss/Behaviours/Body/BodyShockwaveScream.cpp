#include "pch.h"
#include "BodyShockwaveScream.h"
#include "AI/FinalBoss/Constants.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "Engine/GameObject/GameObject.h"
#include "AI/FinalBoss/FinalBossBody.h"
#include "Engine/ResourceManagement/Resources/GameObjectPrefabResource.h"
#include "Engine/GameObject/Prefab/GameObjectPrefab.h"
#include "AI/FinalBoss/FinalBoss.h"
#include "AI/FinalBoss/EmittedShockwave.h"
#include "Engine/GameObject/Components/CameraShakeComponent.h"
#include "Engine/GameObject/Components/AudioComponent.h"

void FB::BodyShockwaveScream::UpdateStart()
{
}

void FB::BodyShockwaveScream::UpdateLoop()
{
	const auto& desc = myBody.GetShockwaveDesc();

	myLoopTimer.Tick(Time::DeltaTime);

	if (myLoopTimer.IsFinished())
	{
		if (myEmitCounter >= desc.myTotalShockwaves)
		{
			myState = FinalBossShockwaveState::End;
			myBody.GetAnimator().GetController().SetBool("ShockwaveFinishedLoop", true);
			return;
		}
		else
		{
			myEmitCounter++;

			EmitShockwave();

			// cam shake
			{ 
				auto mainCam = myBody.GetGameObject()->GetScene()->GetMainCameraGameObject();
				auto shakeComponent = mainCam->GetComponent<Engine::CameraShakeComponent>();
				if (shakeComponent)
				{
					// shakeComponent->AddPerlinShake("FinalBossEmitShockwave");
					const auto dir = (myBody.GetFinalBoss().GetPlayerPos() - myBody.GetFinalBoss().GetTransform().GetPosition()).GetNormalized();
					shakeComponent->AddKickShake("FinalBossEmitShockwave", dir);
				}
			}

			myLoopTimer = TickTimer::FromSeconds(desc.myShockwaveDelaySec);
		}
	}
}

void FB::BodyShockwaveScream::UpdateEnd()
{
}

void FB::BodyShockwaveScream::ShockwaveSound(const bool aEnable) const
{
	if (aEnable)
	{
		myBody.GetAudio()->PlayEvent("ShockwaveScream");
	}
	else
	{
		myBody.GetAudio()->StopEvent("ShockwaveScream");
	}
}

FB::BodyShockwaveScream::BodyShockwaveScream(FinalBossBody& aBody)
	: myBody(aBody)
{
	myBody.GetAnimator().GetController().AddStateOnExitCallback("Base", "Shockwave Start",
		[this]()
		{
			myState = FinalBossShockwaveState::Loop;
		});

	myBody.GetAnimator().GetController().AddStateOnExitCallback("Base", "Shockwave End",
		[this]()
		{
			myIsFinished = true;
		});

	myBody.GetAnimator().GetController().AddEventCallback(
		"EmitShockwave",
		[this]()
		{
			// if (myEmitCounter % 2 == 0)
			// {
			// 	EmitShockwave();
			// }
			// 
			// myEmitCounter++;
		});
}

void FB::BodyShockwaveScream::Update()
{
	switch (myState)
	{
	case FinalBossShockwaveState::Start:
		UpdateStart();
		break;
	case FinalBossShockwaveState::Loop:
		UpdateLoop();
		break;
	case FinalBossShockwaveState::End:
		UpdateEnd();
		break;
	default:
		break;
	}
}

bool FB::BodyShockwaveScream::IsFinished() const
{
	return myIsFinished;
}

void FB::BodyShockwaveScream::OnEnter()
{
	myBody.GetAnimator().GetController().Trigger("Shockwave");

	const auto& desc = myBody.GetShockwaveDesc();

	myState = FinalBossShockwaveState::Start;
	myIsFinished = false;
	myLoopTimer = TickTimer::FromSeconds(0.f);
	myBody.GetAnimator().GetController().SetBool("ShockwaveFinishedLoop", false);
	myEmitCounter = 0;

	ShockwaveSound(true);
}

void FB::BodyShockwaveScream::OnExit()
{
	ShockwaveSound(false);
}

void FB::BodyShockwaveScream::Reflect(Engine::Reflector& aReflector)
{
	aReflector.ReflectEnum(myState, "State");
}

void FB::BodyShockwaveScream::EmitShockwave()
{
	const auto& desc = myBody.GetShockwaveDesc();

	if (desc.myShockwavePrefab && desc.myShockwavePrefab->IsValid())
	{
		auto& shockwave = desc.myShockwavePrefab->Get().Instantiate(*myBody.GetGameObject()->GetScene());
		const auto& ground = myBody.GetFinalBoss().GetGroundPosition();
		shockwave.GetTransform().SetPosition(ground + Vec3f(0.f, 1.f, 0.f));

		if (auto emittedShockwave = shockwave.GetComponent<EmittedShockwave>())
		{
			emittedShockwave->SetDamage(desc.myDamage.GetDamage());
			emittedShockwave->SetSpeed(desc.mySpeed);
			emittedShockwave->SetKnockbarStrength(desc.myKnockbackStrength);
			emittedShockwave->SphereThiccness(desc.mySphereThiccness);
			emittedShockwave->SetShockwaveHeight(desc.myShockwaveHeight);
		}
	}
}
