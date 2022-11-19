#include "pch.h"
#include "AnimationStateMachine.h"
#include "Engine\ResourceManagement\Resources\AnimationClipResource.h"
#include "Engine\ResourceManagement/Resources/AnimationStateMachineResource.h"
#include "Engine\ResourceManagement\Resources\ModelResource.h"

bool Engine::AnimationStateMachine::Init(const Path& aPath, AnimationStateMachineResource& aAnimationStateMachine)
{
	std::ifstream file(aPath.ToString());

	if (!file.is_open())
	{
		LOG_ERROR(LogType::Resource) << "Failed to open " << aPath;
		return false;
	}

	const bool ignoreComments = true;
	const bool allowExceptions = false;
	myJson = nlohmann::json::parse(file, nullptr, allowExceptions, ignoreComments);

	// If failed to parse json file
	if (myJson.is_discarded())
	{
		LOG_ERROR(LogType::Resource) << "Failed to parse " << aPath;
		return false;
	}

	// Read the animations used by the machine to add them as child to the AnimationMachineResource
	// Request to make us load the children as well
	if (myJson.contains("AnimationClips"))
	{
		for (const std::string animClipPath : myJson["AnimationClips"])
		{
			auto clip = GResourceManager->CreateRef<AnimationClipResource>(animClipPath);
			clip->RequestLoading();

			aAnimationStateMachine.AddChild(clip);
		}
	}

	if (myJson.contains("Model"))
	{
		auto model = GResourceManager->CreateRef<ModelResource>(myJson["Model"]);
		model->RequestLoading();

		aAnimationStateMachine.AddChild(model);
	}

	return true;
}
