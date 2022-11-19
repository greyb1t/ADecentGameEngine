#include "pch.h"
#include "BlendTree1D.h"
#include "BlendNode1D.h"
#include "BlendHelper.h"
#include "Engine\Animation\State\AnimationMachine.h"
#include "Engine\Animation\Visitor\PlayableVisitor.h"

void Engine::BlendTree1D::AddNode(BlendNode1D aNode)
{
	myNodes.push_back(std::move(aNode));
}

Engine::AnimationTransformResult Engine::BlendTree1D::CalculateInterpolatedTransforms()
{
	if (!CanPlay())
	{
		return {};
	}

	const auto [left, right] = DetermineNodesToBlend();

	if (left == nullptr && right == nullptr)
	{
		return {};
	}

	AnimationTransformResult blendedResult;

	// if we have 2 nodes to blend between
	if (right != nullptr)
	{
		const float blendValue = CalculateBlendWeight(*left, *right);

		// Interpolate each local animation
		AnimationTransformResult leftResult = left->GetPlayable()->CalculateInterpolatedTransforms();
		AnimationTransformResult rightResult = right->GetPlayable()->CalculateInterpolatedTransforms();

		// Blend the already interpolated stuff
		blendedResult = BlendHelper::Blend(leftResult, rightResult, blendValue);
	}
	else
	{
		// if only one node was found to blend with, means the blend value
		// was the exact same as one of the blend node thresholds
		blendedResult = left->GetPlayable()->CalculateInterpolatedTransforms();
	}

	{
		myRootMotionPrevious = myRootMotion;

		if (myRootMotion.myTranslation.Length() > 150.f)
		{
			int test = 0;
		}

		myRootMotion.myScale = blendedResult.myEntries[1].myScale;
		myRootMotion.myRotation = blendedResult.myEntries[1].myRotation;
		myRootMotion.myTranslation = blendedResult.myEntries[1].myTranslation;

		if (myRootMotion.myTranslation.Length() > 150.f)
		{
			int test = 0;

			// blendedResult = left->GetPlayable()->CalculateInterpolatedTransforms();
		}
	}

	return blendedResult;
}

Engine::AnimationTransform Engine::BlendTree1D::GetRootMotionDelta() const
{
	// TODO: root motion blending
	// https://takinginitiative.wordpress.com/2016/07/10/blending-animation-root-motion/

	AnimationTransform rootMotionDelta;
	{
		rootMotionDelta.myScale = myRootMotion.myScale - myRootMotionPrevious.myScale;

		// TODO: Is this Inverse correct? or should the other one be Inverse?
		rootMotionDelta.myRotation = myRootMotion.myRotation * myRootMotionPrevious.myRotation.GetInverse();

		rootMotionDelta.myTranslation = myRootMotion.myTranslation - myRootMotionPrevious.myTranslation;
	}

	return rootMotionDelta;
}

bool Engine::BlendTree1D::HasFinished() const
{
	if (myIsLooping)
	{
		return false;
	}

	return myNormalizedTime >= 1.f;
}

void Engine::BlendTree1D::Reset()
{
	myNormalizedTime = 0.f;

	if (myBlendFloatName == "DashBlend")
	{
		if (myRootMotion.myTranslation.Length() > 150.f)
		{
			int test2 = 0;
		}

		int test = 0;
	}

	for (auto& node : myNodes)
	{
		node.GetPlayable()->Reset();
	}

	if (myRootMotion.myTranslation.Length() > 0)
	{
		int test = 0;
	}

	if (myRootMotionPrevious.myTranslation.Length() > 0)
	{
		int test = 0;
	}

	// When we reset the animation, the prev root motion result has to be reset as well
	myRootMotion = {};
	myRootMotionPrevious = {};
}

float Engine::BlendTree1D::GetElapsedTimeNormalized() const
{
	return myNormalizedTime;
}

void Engine::BlendTree1D::Update(AnimationState& aState, const float aDeltaTime)
{
	ZoneScopedN("BlendTree1D::Update");

	const auto [left, right] = DetermineNodesToBlend();

	if (left == nullptr && right == nullptr)
	{
		LOG_WARNING(LogType::Animation) << "BlendTree1D missing both nodes, cannot play blendtree";
		return;
	}

	if (right != nullptr)
	{
		// we have both nodes to blend between

		const float blendWeight = CalculateBlendWeight(*left, *right);
		myDurationSeconds = Math::Lerp(
			left->GetPlayable()->GetDurationSeconds(),
			right->GetPlayable()->GetDurationSeconds(),
			blendWeight);

		left->GetPlayable()->Update(aState, 0.f);
		left->GetPlayable()->SetElapsedTimeNormalized(myNormalizedTime);

		right->GetPlayable()->Update(aState, 0.f);
		right->GetPlayable()->SetElapsedTimeNormalized(myNormalizedTime);
	}
	else
	{
		// only update left one because we only have that
		myDurationSeconds = left->GetPlayable()->GetDurationSeconds();
		left->GetPlayable()->Update(aState, 0.f);
		left->GetPlayable()->SetElapsedTimeNormalized(myNormalizedTime);
	}

	/*
	for (auto& node : myNodes)
	{
		// Update with delta time 0 since we want to control the
		// animation normalized time ourself from this function
		node.GetPlayable()->Update(aState, aDeltaTime);

		node.GetPlayable()->SetElapsedTimeNormalized(myNormalizedTime);

		// LOG_INFO(LogType::Filip) << myNormalizedTime;
	}
	*/

	// myNormalizedTime += aDeltaTime / myDurationSeconds;

	// COPY PASTED FROM NormalAnimation
	{
		if (myNormalizedTime >= 1.f)
		{
			if (myIsLooping)
			{
				Reset();
			}
			else
			{
				// If not looping, freeze the animation at the last frame
				myNormalizedTime = 1.f;
			}
		}

		if (myIsLooping)
		{
			// myNormalizedTime += aDeltaTime * myTimeScale * myClip->Get().GetTicksPerSecond();
			myNormalizedTime += aDeltaTime / myDurationSeconds;
		}
		else
		{
			if (!HasFinished())
			{
				myNormalizedTime += aDeltaTime / myDurationSeconds;
			}
		}

		// Clamp it to not spill over the max duration
		myNormalizedTime = std::min(myNormalizedTime, 1.f);
	}
}

float Engine::BlendTree1D::GetDurationSeconds() const
{
	return myDurationSeconds;
}

bool Engine::BlendTree1D::IsRootMotion() const
{
	// if any of the animations is a root motion, the blend tree uses root motion as well
	for (const auto& node : myNodes)
	{
		if (node.GetPlayable()->IsRootMotion())
		{
			return true;
		}
	}

	return false;
}

void Engine::BlendTree1D::RecalculateTree()
{
	// Calculate the blend tree duration
	myDurationSeconds = FLT_MAX;
	for (auto& node : myNodes)
	{
		if (node.GetPlayable() == nullptr)
		{
			continue;
		}

		myDurationSeconds = std::min(myDurationSeconds, node.GetPlayable()->GetDurationSeconds());
	}

	// Determine if looping
	// A blendtree is only looping if any of the animations are looping
	// if all animations are not looping, the blendtree is not looping either
	myIsLooping = true;
	for (const auto& node : myNodes)
	{
		if (!node.GetPlayable()->IsLooping())
		{
			myIsLooping = false;
			break;
		}
	}

	// Calculate min and max blend value
	myMinThreshold = FLT_MAX;
	myMaxThreshold = -FLT_MAX;
	for (const auto& node : myNodes)
	{
		myMinThreshold = std::min(myMinThreshold, node.GetThreshold());
		myMaxThreshold = std::max(myMaxThreshold, node.GetThreshold());
	}

	// Fixup the blend node order, they must be in order of threshold value small-to-big
	std::sort(myNodes.begin(), myNodes.end(),
		[](const BlendNode1D& aLeft, const BlendNode1D& aRight)
		{
			return aLeft.GetThreshold() < aRight.GetThreshold();
		});
}

std::pair<Engine::BlendNode1D*, Engine::BlendNode1D*> Engine::BlendTree1D::DetermineNodesToBlend()
{
	ZoneScopedN("BlendTree1D::DetermineNodesToBlend");

	BlendNode1D* left = nullptr;
	BlendNode1D* right = nullptr;

	// Find the two nodes that are located to 
	// the left and right of the current time

	if (myNodes.empty())
	{
		return {};
	}

	left = &myNodes.front();
	right = &myNodes.back();

	int indexToContinueAt = 0;

	// Find the left node
	for (int i = 0; i < myNodes.size(); ++i)
	{
		// if the either left or right node is the exact same value as the blend node
		// means, it should not blend between 2 of them, since only one of them is to be used
		if (myNodes[i].GetThreshold() == GetBlendValue())
		{
			left = &myNodes[i];

			// therefore return with right node being nullptr
			return std::make_pair(left, nullptr);
		}

		if (myNodes[i].GetThreshold() <= GetBlendValue())
		{
			left = &myNodes[i];
		}
		else
		{
			indexToContinueAt = i;
			break;
		}
	}

	// Find the right node
	for (int i = static_cast<int>(myNodes.size()) - 1; i >= indexToContinueAt; --i)
	{
		if (myNodes[i].GetThreshold() >= GetBlendValue())
		{
			right = &myNodes[i];
		}
		else
		{
			break;
		}
	}

	assert(left && right && "we might need to change the way we calculate the blend value");

	return std::make_pair(left, right);
}

float Engine::BlendTree1D::CalculateBlendWeight(const BlendNode1D& aLeft, const BlendNode1D& aRight) const
{
	const float blendValue =
		(GetBlendValue() - aLeft.GetThreshold()) /
		(aRight.GetThreshold() - aLeft.GetThreshold());

	return blendValue;
}

void Engine::BlendTree1D::RemoveFloatUsage(const std::string& aFloatName)
{
	if (myBlendFloatName == aFloatName)
	{
		myBlendFloatName = "";
		myBlendValue = nullptr;
	}
}

float Engine::BlendTree1D::GetBlendValue() const
{
	return Math::Clamp(*myBlendValue, myMinThreshold, myMaxThreshold);
}

void Engine::BlendTree1D::InitFromJson2(const nlohmann::json& aJson, AnimationMachine& aMachine)
{
	if (aJson.contains("MinThreshold"))
	{
		myMinThreshold = aJson["MinThreshold"];
	}

	if (aJson.contains("MaxThreshold"))
	{
		myMaxThreshold = aJson["MaxThreshold"];
	}

	if (aJson.contains("DurationSeconds"))
	{
		myDurationSeconds = aJson["DurationSeconds"];
	}

	if (aJson.contains("IsLooping"))
	{
		myIsLooping = aJson["IsLooping"];
	}

	for (const auto& node : aJson["Nodes"])
	{
		BlendNode1D blendNode;
		blendNode.InitFromJson(node, aMachine);

		myNodes.push_back(std::move(blendNode));
	}

	if (aJson.contains("BlendFloatName"))
	{
		myBlendFloatName = aJson["BlendFloatName"];

		if (aMachine.GetFloats().find(myBlendFloatName) != aMachine.GetFloats().end())
		{
			myBlendValue = aMachine.GetFloats().at(myBlendFloatName).get();
		}
	}
}

nlohmann::json Engine::BlendTree1D::ToJson() const
{
	nlohmann::json j;

	j["Type"] = "BlendTree1D";

	j["MinThreshold"] = myMinThreshold;
	j["MaxThreshold"] = myMaxThreshold;

	j["DurationSeconds"] = myDurationSeconds;
	j["IsLooping"] = myIsLooping;

	j["BlendFloatName"] = myBlendFloatName;

	nlohmann::json nodeArray = nlohmann::json::array();

	for (const auto& node : myNodes)
	{
		nodeArray.push_back(node.ToJson());
	}

	j["Nodes"] = nodeArray;

	return j;
}

void Engine::BlendTree1D::SetElapsedTimeNormalized(const float aTimeNormalized)
{
	myNormalizedTime = aTimeNormalized;
}

void Engine::BlendTree1D::SetIsRootMotion(const bool aIsRootMotion)
{
	throw std::logic_error("The method or operation is not implemented.");
}

void Engine::BlendTree1D::SetTimeScale(const float aTimeScale)
{
	throw std::logic_error("The method or operation is not implemented.");
}

float Engine::BlendTree1D::GetTimeScale() const
{
	throw std::logic_error("The method or operation is not implemented.");
}

const std::string& Engine::BlendTree1D::GetName() const
{
	return myName;
}

std::vector<const Engine::AnimationEvent*> Engine::BlendTree1D::GetEventsExecutedThisFrame()
{
	std::vector<const Engine::AnimationEvent*> events;

	for (auto& node : myNodes)
	{
		auto nodeEvents = node.GetPlayable()->GetEventsExecutedThisFrame();

		events.insert(events.end(), nodeEvents.begin(), nodeEvents.end());
	}

	return events;
}

bool Engine::BlendTree1D::IsLooping() const
{
	return myIsLooping;
}

void Engine::BlendTree1D::SetIsLooping(const bool aIsLooping)
{
	// This is not defined, since we can have both animations that does loop, and does not loop

	throw std::logic_error("The method or operation is not implemented.");
}

bool Engine::BlendTree1D::CanPlay() const
{
	if (myBlendValue == nullptr)
	{
		return false;
	}

	for (const auto& node : myNodes)
	{
		if (node.GetPlayable() == nullptr)
		{
			return false;
		}

		if (!node.GetPlayable()->CanPlay())
		{
			return false;
		}
	}

	return true;
}

void Engine::BlendTree1D::Accept(PlayableVisitor& aVisitor)
{
	aVisitor.Visit(*this);
}
