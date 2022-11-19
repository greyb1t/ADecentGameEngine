#include "pch.h"
#include "BlendNode2D.h"
#include "Engine/Animation/NormalAnimation.h"

Engine::BlendNode2D::BlendNode2D(const Vec2f aPosition, Owned<Playable> aPlayable)
	: myPosition(aPosition),
	myPlayable(std::move(aPlayable))
{
}

nlohmann::json Engine::BlendNode2D::ToJson() const
{
	nlohmann::json j;

	j["Position"]["x"] = myPosition.x;
	j["Position"]["y"] = myPosition.y;

	if (myPlayable != nullptr)
	{
		j["Playable"] = myPlayable->ToJson();
	}

	return j;
}

void Engine::BlendNode2D::InitFromJson(const nlohmann::json& aJson, AnimationMachine& aMachine)
{
	myPosition.x = aJson["Position"]["x"];
	myPosition.y = aJson["Position"]["y"];

	if (aJson.contains("Playable"))
	{
		// TODO: turn into an enum
		const std::string playableType = aJson["Playable"]["Type"];

		if (playableType == "Animation")
		{
			myPlayable = MakeOwned<NormalAnimation>();
			myPlayable->InitFromJson2(aJson["Playable"], aMachine);
		}
		else
		{
			assert(false);
		}
	}
}

void Engine::BlendNode2D::SetPosition(const Vec2f& aPosition)
{
	myPosition = aPosition;
}

const Vec2f& Engine::BlendNode2D::GetPosition() const
{
	return myPosition;
}

Engine::Playable* Engine::BlendNode2D::GetPlayable()
{
	return myPlayable.get();
}

float Engine::BlendNode2D::GetRotationRadians() const
{
	return myRotationRadians;
}

void Engine::BlendNode2D::RecalculateRotation()
{
	// Assumes 0,0 is the center reference point
	myRotationRadians = atan2(myPosition.y, myPosition.x);
}

const Engine::Playable* Engine::BlendNode2D::GetPlayable() const
{
	return myPlayable.get();
}
