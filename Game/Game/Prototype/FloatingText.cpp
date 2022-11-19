#include "pch.h"
#include "FloatingText.h"

#include "Engine/Shortcuts.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/TextComponent.h"

FloatingText::FloatingText(GameObject* aObj, std::string aText)
	: Component(aObj), myText(aText)
{
}

void FloatingText::Start()
{
	myTextComp = myGameObject->AddComponent<Engine::TextComponent>(myText, 0);
	myTextComp->SetColor(myTextColor);
	myGameObject->SetName("FloatingText");
	GetTransform().SetScale({ .5f, .5f, .5f });
}

void FloatingText::Execute(Engine::eEngineOrder aOrder)
{
	myTime += Time::DeltaTime;
	if (myTime >= myLifetime)
	{
		myGameObject->Destroy();
	}

	const float percentage = 1.f - (myTime / myLifetime);

	float dist = myRiseSpeed * Time::DeltaTime * CU::Max(powf(percentage, 2), 0.f);
	GetTransform().Move(Vec3f(0,dist,0));
	const Vec3f dir = myGameObject->GetScene()->GetMainCamera().GetTransform().GetPosition() - GetTransform().GetPosition();
	GetTransform().SetRotation(Quatf(Vec3f(0, (Math::AngleOfVector({ dir.x, dir.z }) + 90.f) * Math::Deg2Rad, 0)));
	
}

