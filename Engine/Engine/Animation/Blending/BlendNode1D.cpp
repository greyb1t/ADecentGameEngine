#include "pch.h"
#include "BlendNode1D.h"
#include "Engine\Animation\NormalAnimation.h"
#include "Engine\Shortcuts.h"
#include "Engine\ResourceManagement\ResourceManager.h"
#include "Engine\Animation\State\AnimationMachine.h"

Engine::BlendNode1D::BlendNode1D(
	const float aTimePosition,
	Owned<Playable> aPlayable)
	: myThreshold(aTimePosition),
	myPlayable(std::move(aPlayable))
{

}

nlohmann::json Engine::BlendNode1D::ToJson() const
{
	nlohmann::json j;

	j["Threshold"] = myThreshold;

	if (myPlayable != nullptr)
	{
		j["Playable"] = myPlayable->ToJson();
	}

	return j;
}

void Engine::BlendNode1D::InitFromJson(const nlohmann::json& aJson, AnimationMachine& aMachine)
{
	myThreshold = aJson["Threshold"];

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

float Engine::BlendNode1D::GetThreshold() const
{
	return myThreshold;
}

Engine::Playable* Engine::BlendNode1D::GetPlayable()
{
	return myPlayable.get();
}

const Engine::Playable* Engine::BlendNode1D::GetPlayable() const
{
	return myPlayable.get();
}
