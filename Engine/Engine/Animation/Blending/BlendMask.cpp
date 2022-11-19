#include "pch.h"
#include "BlendMask.h"

#include "Engine/Renderer/Animation/AnimationClip.h"
#include "Engine/ResourceManagement/Resources/AnimationClipResource.h"

void Engine::BlendMask::Init(const int aBoneCount)
{
	myMask.clear();
	myMask.resize(aBoneCount, true);
}

void Engine::BlendMask::Clear()
{
	myMask.clear();
}

bool Engine::BlendMask::InitFromJson(const nlohmann::json& aJson)
{
	myMask.resize(aJson["Values"].size());

	for (int i = 0; i < aJson["Values"].size(); ++i)
	{
		myMask[i] = aJson["Values"][i];
	}

	// if (aJson.contains("ClipForSkeleton"))
	// {
	// 	aClipForSkeleton = GResourceManager->CreateRef<AnimationClipResource>(aJson["ClipForSkeleton"]);
	// }

	if (aJson.contains("MaskEnabled"))
	{
		myIsEnabled = aJson["MaskEnabled"];
	}

	return true;
}

nlohmann::json Engine::BlendMask::ToJson() const
{
	nlohmann::json j = nlohmann::json::array();

	for (const auto b : myMask)
	{
		j.push_back(b);
	}

	nlohmann::json jj;

	jj["Values"] = j;

	// if (myClipForSkeleton && myClipForSkeleton->IsValid())
	// {
	// 	jj["ClipForSkeleton"] = myClipForSkeleton->GetPath();
	// }

	jj["MaskEnabled"] = myIsEnabled;

	return jj;
}

bool Engine::BlendMask::IsMaskEnabled() const
{
	return myIsEnabled;
}

bool Engine::BlendMask::IsEnabled(const int aBoneIndex) const
{
	if (aBoneIndex < myMask.size())
	{
		return myMask[aBoneIndex];
	}

	return true;
}

void Engine::BlendMask::SetIsEnabled(const int aBoneIndex, const bool aValue)
{
	myMask[aBoneIndex] = aValue;
}

bool Engine::BlendMask::IsEmpty() const
{
	return myMask.empty();
}
