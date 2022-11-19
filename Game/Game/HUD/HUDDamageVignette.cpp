#include "pch.h"
#include "HUDDamageVignette.h"
#include "Engine/GameObject/Components/SpriteComponent.h"
#include "Components\HealthComponent.h"
#include "Engine\Reflection\Reflector.h"
#include "Engine\GameObject\GameObject.h"
#include "Engine\ResourceManagement\Resources\AnimationCurveResource.h"

void HUDDamageVignette::Start()
{
	myVignette = GetTransform().GetChildByGameObjectName("Vignette")->GetComponent<Engine::SpriteComponent>();
	myStayVignette = GetTransform().GetChildByGameObjectName("StayVignette")->GetComponent<Engine::SpriteComponent>();
	
	myHealthComponent = Main::GetPlayer()->GetComponent<HealthComponent>();
	myHealthComponent->ObserveDamage([&](float/*aDamage*/) { OnDamageObserve(); });

	if (mySizeCurveRef && mySizeCurveRef->IsValid())
		mySizeCurve = mySizeCurveRef->Get();

	if (myStayCurveRef && myStayCurveRef->IsValid())
		myStayCurve = myStayCurveRef->Get();

	myResizeTimer = myResizeTime;
}

void HUDDamageVignette::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(mySizeCurveRef, "Size Curve");
	aReflector.Reflect(myStayCurveRef, "Stay Curve");
}

void HUDDamageVignette::Execute(Engine::eEngineOrder aOrder)
{
	myResizeTimer += Time::DeltaTime;

	float healthPercent = myHealthComponent->GetHealth() / myHealthComponent->GetMaxHealth();

	float scale = C::Lerp(myVignetteMinScale, myVignetteMaxScale, healthPercent);
	myVignette->GetTransform().SetScale(scale);

	float percent = myResizeTimer / myResizeTime;
	float curveVal = mySizeCurve.Evaluate(percent);
	myVignette->SetAlpha(curveVal);

	float stayScale = myStayCurve.Evaluate(1.0f - healthPercent);
	myStayVignette->GetTransform().SetScale(stayScale);
	//myStayVignette->SetAlpha(1.0f - curveVal);
}

void HUDDamageVignette::OnDamageObserve()
{
	myResizeTimer = 0.0f;
}
