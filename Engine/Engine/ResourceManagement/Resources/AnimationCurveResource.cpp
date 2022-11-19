#include "pch.h"
#include "AnimationCurveResource.h"
#include "../ResourceThreadContext.h"
#include "Engine/GraphManager/VisualScript.h"
#include "../ResourceObserver.h"
#include "Engine/AnimationCurve/Curve.h"

Engine::AnimationCurveResource::AnimationCurveResource(
	ResourceManager& aResourceManager,
	const std::string& aPath,
	const ResourceId& aId)
	: ResourceBase(aResourceManager, aPath, aId)
{
}

Engine::AnimationCurveResource::~AnimationCurveResource()
{
}

Engine::AnimationCurveResource::AnimationCurveResource(const AnimationCurveResource& aOther) :
	ResourceBase(*aOther.myResourceManager, aOther.myPath, aOther.myIdentifier)
{
	assert(aOther.myCurve == nullptr && "why is this not nullptr? it is "
		"supposed to be because we only ever copy this in ResourceManager "
		"before it has been fully created");
}

Engine::Curve& Engine::AnimationCurveResource::Get() const
{
	return *myCurve;
}

bool Engine::AnimationCurveResource::Create(ResourceThreadData& aThreadData)
{
	std::ifstream file(myPath);

	if (!file.is_open())
	{
		LOG_ERROR(LogType::Resource) << "Failed to open " << myPath;
		return false;
	}

	const bool ignoreComments = true;
	const bool allowExceptions = false;
	auto j = nlohmann::json::parse(file, nullptr, allowExceptions, ignoreComments);

	// If failed to parse json file
	if (j.is_discarded())
	{
		LOG_ERROR(LogType::Resource) << "Failed to parse " << myPath;
		return false;
	}

	auto curve = MakeOwned<Curve>();

	if (!curve->InitFromJson(j["Keys"]))
	{
		return false;
	}

	myCurve = std::move(curve);

	return true;
}

void Engine::AnimationCurveResource::Swap(ResourceBase* aResourceBase)
{
	// We know for a fact that aResourceBase input argument is of the same class
	// otherwise something went wrong in the ResourceManager
	auto other = reinterpret_cast<AnimationCurveResource*>(aResourceBase);

	myCurve.swap(other->myCurve);
}

void Engine::AnimationCurveResource::PostCreation(ResourceThreadData& aResourceThreadData)
{
	myResourceManager->GetFileWatcher()->WatchFile(myPath,
		[this, &aResourceThreadData](const std::string& aPath)
		{
			Create(aResourceThreadData);
		});
}

void Engine::AnimationCurveResource::RequestMe() const
{
	// Sadly this is required to avoid having the ResourceBase::RequestLoading as a NON-template function
	// Why do we want that as a non-template function? To avoid circular include with templated shit..
	// ABSOLUTE DOG
	myResourceManager->RequestLoading<AnimationCurveResource>(myIdentifier);
}

void Engine::AnimationCurveResource::LoadMe() const
{
	myResourceManager->Load<AnimationCurveResource>(myIdentifier);
}

void Engine::AnimationCurveResource::UnloadMe()
{
	myCurve = nullptr;
}
