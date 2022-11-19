#include "pch.h"
#include "UI_Fade.h"

#include "Engine/GameObject/Components/TextComponent.h"
#include "Engine/GameObject/Components/SpriteComponent.h"

void UI_Fade::Awake()
{
	myText = myGameObject->GetComponent<Engine::TextComponent>();
	mySprite = myGameObject->GetComponent<Engine::SpriteComponent>();
}

void UI_Fade::Start()
{
	if (myDoScale)
	{
		myScaleSine.myTotalTime = Random::RandomFloat(-60.0f, 60.0f);
	}

	if (myDoAlpha)
	{
		myAlphaSine.myTotalTime = Random::RandomFloat(-60.0f, 60.0f);
	}
}

void UI_Fade::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myDoScale, "Scale");
	if (myDoScale)
	{
		aReflector.Reflect(myScaleSine, "Scale Sine Values");
	}

	aReflector.Reflect(myDoAlpha, "Alpha");
	if (myDoAlpha)
	{
		aReflector.Reflect(myAlphaSine, "Alpha Sine Values");
	}
}

void UI_Fade::Execute(Engine::eEngineOrder aOrder)
{
	if (myDoScale)
	{
		myScaleSine.myTotalTime += Time::DeltaTimeUnscaled * myScaleSine.mySpeed;

		const float middle = (myScaleSine.myHighestValue + myScaleSine.myLowestValue) * 0.5f;
		float sine = std::sinf(myScaleSine.myTotalTime) * myScaleSine.myMultiplier;

		const float diff = myScaleSine.myHighestValue - myScaleSine.myLowestValue;
		float value = middle + (diff * 0.5f) * sine;

		GetTransform().SetScale(value);
	}

	if (myDoAlpha)
	{
		myAlphaSine.myTotalTime += Time::DeltaTimeUnscaled * myAlphaSine.mySpeed;

		const float middle = (myAlphaSine.myHighestValue + myAlphaSine.myLowestValue) * 0.5f;
		float sine = std::sinf(myAlphaSine.myTotalTime) * myAlphaSine.myMultiplier;

		const float diff = myAlphaSine.myHighestValue - myAlphaSine.myLowestValue;
		float value = middle + (diff * 0.5f) * sine;

		if (myText)
		{
			myText->SetAlpha(value);
		}

		if (mySprite)
		{
			mySprite->SetAlpha(value);
		}
	}
}

void SineWave::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(mySpeed, "Speed");
	aReflector.Reflect(myLowestValue, "Lowest Value");
	aReflector.Reflect(myHighestValue, "Highest Value");
	aReflector.Reflect(myMultiplier, "Multiplier");
}
