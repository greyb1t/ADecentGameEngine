#include "pch.h"
#include "HUDItemDescription.h"

#include "Engine/Engine.h"

#include "Engine/GameObject/Components/SpriteComponent.h"
#include "Engine/GameObject/Components/TextComponent.h"

#include "Engine\Reflection\Reflector.h"
#include "Engine\GameObject\GameObject.h"

HUDItemDescription::HUDItemDescription(GameObject* aGameObject) : Component(aGameObject)
{
}

HUDItemDescription::~HUDItemDescription()
{

}

void HUDItemDescription::Awake()
{
	myShortDescBox = GetTransform().GetChildByGameObjectName("NameShortDesc");
	myNameDescBox = GetTransform().GetChildByGameObjectName("BothNameDesc");

	if (myShortDescBox)
	{
		myShortNameText = myShortDescBox->GetTransform().GetChildByGameObjectName("Item Name")->GetComponent<Engine::TextComponent>();
		myShortDescText = myShortDescBox->GetTransform().GetChildByGameObjectName("Item Description")->GetComponent<Engine::TextComponent>();
	}

	if (myNameDescBox)
	{
		myNameText = myNameDescBox->GetTransform().GetChildByGameObjectName("Item Name")->GetComponent<Engine::TextComponent>();
		myDescText = myNameDescBox->GetTransform().GetChildByGameObjectName("Item Description")->GetComponent<Engine::TextComponent>();
		myBackground = myNameDescBox->GetTransform().GetChildByGameObjectName("Background Box")->GetComponent<Engine::SpriteComponent>();
	}
}

void HUDItemDescription::Start()
{
}

void HUDItemDescription::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myShouldFadeOut, "Fade Out Desc");

	std::string name = "";
	if (myNameText)
		name = myNameText->GetText();

	std::string desc = "";
	if (myDescText)
		desc = myDescText->GetText();

	aReflector.Reflect(name, "Name", Engine::ReflectionFlags_ReadOnly);
	aReflector.Reflect(desc, "Desc", Engine::ReflectionFlags_ReadOnly);
}

void HUDItemDescription::Execute(Engine::eEngineOrder aOrder)
{
	if (!myShouldFadeOut)
		return;

	if (!myNameDescBox)
		return;

	switch (myDescState)
	{
	case HUDItemDescription::DescState::Active:
	{
		myShowTimer += Time::DeltaTime;

		if (myShowTimer > myShowDescTime)
		{
			myFadeTimer = 0.0f;
			myDescState = DescState::Fading;
		}
	}
	break;

	case HUDItemDescription::DescState::Fading:
	{
		myFadeTimer += Time::DeltaTime;

		float timePercent = std::min(1.0f, myFadeTimer / myFadeTime);
		float alpha = C::Lerp(1.0f, 0.0f, timePercent);

		myNameText->SetAlpha(alpha);
		myDescText->SetAlpha(alpha);
		myBackground->SetAlpha(alpha);

		if (myFadeTimer > myFadeTime)
		{
			myNameDescBox->SetActive(false);
			myDescState = DescState::Disabled;
		}
	}
	break;

	default:
		break;
	}
}

void HUDItemDescription::SetName(const std::string& aName)
{
	myNameText->SetText(aName);

	if(myShortNameText)
		myShortNameText->SetText(aName);
}

void HUDItemDescription::SetDesc(const std::string& aDesc)
{
	myDescText->SetText(aDesc);
}

void HUDItemDescription::SetShortDesc(const std::string& aDesc)
{
	myShortDescText->SetText(aDesc);
}

void HUDItemDescription::ShowShortDesc(bool aShouldShow)
{
	if (myShortDescBox)
		myShortDescBox->SetActive(aShouldShow);

	if (aShouldShow && myNameDescBox)
		myNameDescBox->SetActive(!aShouldShow);
}

void HUDItemDescription::ShowDesc(bool aShouldShow)
{
	if (myNameDescBox)
		myNameDescBox->SetActive(aShouldShow);

	if (aShouldShow && myShortDescBox)
		myShortDescBox->SetActive(!aShouldShow);
}

void HUDItemDescription::PickupItem()
{
	myShowTimer = 0.0f;

	myNameText->SetAlpha(1.0f);
	myDescText->SetAlpha(1.0f);
	myBackground->SetAlpha(1.0f);

	myDescState = DescState::Active;

	if (myNameDescBox)
		myNameDescBox->SetActive(true);
}
