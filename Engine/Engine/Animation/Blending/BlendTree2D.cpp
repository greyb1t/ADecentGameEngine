#include "pch.h"
#include "BlendTree2D.h"
#include "BlendNode1D.h"
#include "BlendHelper.h"
#include "Engine\Animation\State\AnimationMachine.h"
#include "Engine\Animation\Visitor\PlayableVisitor.h"

void Engine::BlendTree2D::AddNode(BlendNode2D aNode)
{
	myNodes.push_back(std::move(aNode));
}

void Engine::BlendTree2D::RemoveNode(const int aIndex)
{
	myNodes.erase(myNodes.begin() + aIndex);
}

Engine::AnimationTransformResult Engine::BlendTree2D::CalculateInterpolatedTransforms()
{
	ZoneNamedN(zone1, "BlendTree2D::CalculateINterpolatedTransforms", true);

	if (!CanPlay())
	{
		return {};
	}

	// const auto [left, right] = DetermineNodesToBlend();

	float u = 0.f, v = 0.f, w = 0.f;
	const Triangle* triangle = DetermineTriangleToBlendWith(u, v, w);

	if (triangle == nullptr)
	{
		LOG_WARNING(LogType::Animation) << "2D BlendTree does not play because cannot find triangle to blend with";
		return {};
	}

	AnimationTransformResult blendedResult;

	// Interpolate each local animation
	AnimationTransformResult nodeResult1 =
		myNodes[triangle->myNodeIndex1].GetPlayable()->CalculateInterpolatedTransforms();
	AnimationTransformResult nodeResult2 =
		myNodes[triangle->myNodeIndex2].GetPlayable()->CalculateInterpolatedTransforms();
	AnimationTransformResult nodeResult3 =
		myNodes[triangle->myNodeIndex3].GetPlayable()->CalculateInterpolatedTransforms();

	// Blend the already interpolated stuff
	//blendedResult = BlendHelper::Blend(nodeResult2, nodeResult1, u);
	blendedResult = BlendHelper::Blend(nodeResult1, nodeResult2, nodeResult3, myMachine->GetBindPose(), u, v, w);
	//blendedResult = BlendHelper::Blend(blendedResult, nodeResult3, w);

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

Engine::AnimationTransform Engine::BlendTree2D::GetRootMotionDelta() const
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

bool Engine::BlendTree2D::HasFinished() const
{
	if (myIsLooping)
	{
		return false;
	}

	return myNormalizedTime >= 1.f;
}

void Engine::BlendTree2D::Reset()
{
	myNormalizedTime = 0.f;

	for (auto& node : myNodes)
	{
		if (node.GetPlayable())
		{
			node.GetPlayable()->Reset();
		}
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

float Engine::BlendTree2D::GetElapsedTimeNormalized() const
{
	return myNormalizedTime;
}

void Engine::BlendTree2D::Update(AnimationState& aState, const float aDeltaTime)
{
	ZoneScopedN("BlendTree2D::Update");

	float u = 0.f, v = 0.f, w = 0.f;
	const Triangle* triangle = DetermineTriangleToBlendWith(u, v, w);

	if (triangle != nullptr)
	{
		// we have both nodes to blend between

		float dur1 = myNodes[triangle->myNodeIndex1].GetPlayable()->GetDurationSeconds() * u;
		float dur2 = myNodes[triangle->myNodeIndex2].GetPlayable()->GetDurationSeconds() * v;
		float dur3 = myNodes[triangle->myNodeIndex3].GetPlayable()->GetDurationSeconds() * w;

		// LOG_WARNING(LogType::Animation) << "Filip fixa detta med duration! den ska blenda korrekt";
		//myDurationSeconds = myNodes[triangle->myNodeIndex1].GetPlayable()->GetDurationSeconds();
		//myDurationSeconds = (dur1 + dur2 + dur3) / 3.f;
		myDurationSeconds = dur1 + dur2 + dur3;

		/*
		myDurationSeconds = Math::Lerp(
			left->GetPlayable()->GetDurationSeconds(),
			right->GetPlayable()->GetDurationSeconds(),
			blendWeight);
		*/

		// myNodes[triangle->myNodeIndex1].GetPlayable()->Update(aState, 0.f);
		// myNodes[triangle->myNodeIndex1].GetPlayable()->SetElapsedTimeNormalized(myNormalizedTime);
		// 
		// myNodes[triangle->myNodeIndex2].GetPlayable()->Update(aState, 0.f);
		// myNodes[triangle->myNodeIndex2].GetPlayable()->SetElapsedTimeNormalized(myNormalizedTime);
		// 
		// myNodes[triangle->myNodeIndex3].GetPlayable()->Update(aState, 0.f);
		// myNodes[triangle->myNodeIndex3].GetPlayable()->SetElapsedTimeNormalized(myNormalizedTime);

		for (auto& node : myNodes)
		{
			node.GetPlayable()->Update(aState, 0.f);
			node.GetPlayable()->SetElapsedTimeNormalized(myNormalizedTime);
		}
	}
	else
	{
		LOG_WARNING(LogType::Animation) << "Missing triangle to blend in 2d blend tree";
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

float Engine::BlendTree2D::GetDurationSeconds() const
{
	return myDurationSeconds;
}

bool Engine::BlendTree2D::IsRootMotion() const
{
	// if any of the animations is a root motion, the blend tree uses root motion as well
	for (const auto& node : myNodes)
	{
		if (node.GetPlayable() && node.GetPlayable()->IsRootMotion())
		{
			return true;
		}
	}

	return false;
}

void Engine::BlendTree2D::RecalculateTriangles()
{
	const auto triangles = TriangulateNodes();

	myTriangles.clear();

	for (int i = 0; i < triangles.triangles.size(); ++i)
	{
		const auto& triangle = triangles.triangles[i];

		Triangle newTriangle(triangle);
		newTriangle.myNodeIndex1 = GetNodeIndexFromPosition(Vec2f(triangle.p0.x, triangle.p0.y));
		newTriangle.myNodeIndex2 = GetNodeIndexFromPosition(Vec2f(triangle.p1.x, triangle.p1.y));
		newTriangle.myNodeIndex3 = GetNodeIndexFromPosition(Vec2f(triangle.p2.x, triangle.p2.y));

		myTriangles.push_back(newTriangle);
	}

	for (size_t i = 0; i < myTriangles.size(); ++i)
	{
		if (myTriangles[i].myNodeIndex1 == -1 ||
			myTriangles[i].myNodeIndex2 == -1 ||
			myTriangles[i].myNodeIndex3 == -1)
		{
			LOG_ERROR(LogType::Animation) << "2D BlendTree a triangle has a node index that is -1, tell filip!";
		}
	}

	// Determine if looping
	// A blendtree is only looping if any of the animations are looping
	// if all animations are not looping, the blendtree is not looping either
	myIsLooping = true;
	for (const auto& node : myNodes)
	{
		if (node.GetPlayable())
		{
			if (!node.GetPlayable()->IsLooping())
			{
				myIsLooping = false;
				break;
			}
		}
	}


	/*
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
	*/
}

Engine::BlendTree2D::Triangle* Engine::BlendTree2D::DetermineTriangleToBlendWith(
	float& aU,
	float& aV,
	float& aW)
{
	// Cache them as triangles
	// Loop through all triangles and check if intersects
	for (auto& triangle : myTriangles)
	{
		assert(triangle.myNodeIndex1 != -1);
		assert(triangle.myNodeIndex2 != -1);
		assert(triangle.myNodeIndex3 != -1);

		Barycentric(
			GetBlendValue(),
			myNodes[triangle.myNodeIndex1].GetPosition(),
			myNodes[triangle.myNodeIndex2].GetPosition(),
			myNodes[triangle.myNodeIndex3].GetPosition(),
			aU,
			aV,
			aW);

		if (aU >= 0.f && aU <= 1.f &&
			aV >= 0.f && aV <= 1.f &&
			aW >= 0.f && aW <= 1.f)
		{
			return &triangle;
		}
	}

	// Now we found what triangle to use
	// Get the 3 animations to blend between

	// If nothing intersected with the triangle, we must do another type of 
	// calculation according to pontus

	auto& triangle = FindClosestTriangle(GetBlendValue());

	Barycentric(
		GetBlendValue(),
		myNodes[triangle.myNodeIndex1].GetPosition(),
		myNodes[triangle.myNodeIndex2].GetPosition(),
		myNodes[triangle.myNodeIndex3].GetPosition(),
		aU,
		aV,
		aW);

	//assert(aV < 0.f && "assumes to be negative");

	if (aV < 0.f)
	{
		aU += aV / 2.f;
		aW += aV / 2.f;
	}
	else if (aU < 0.f)
	{
		aV += aU / 2.f;
		aW += aU / 2.f;
	}
	else if (aW < 0.f)
	{
		aV += aW / 2.f;
		aU += aW / 2.f;
	}
	else
	{
		assert(false);
	}

	aU = Math::Clamp(aU, 0.f, 1.f);
	aV = Math::Clamp(aV, 0.f, 1.f);
	aW = Math::Clamp(aW, 0.f, 1.f);

	return &triangle;

	//ZoneScopedN("BlendTree2D::DetermineNodesToBlend");

	//BlendNode2D* left = nullptr;
	//BlendNode2D* right = nullptr;

	//// Find the two nodes that are located to 
	//// the left and right of the current time

	//left = &myNodes.front();
	//right = &myNodes.back();

	//int indexToContinueAt = 0;

	//// Find the left node
	//for (int i = 0; i < myNodes.size(); ++i)
	//{
	//	// if the either left or right node is the exact same value as the blend node
	//	// means, it should not blend between 2 of them, since only one of them is to be used
	//	if (myNodes[i].GetThreshold() == GetBlendValue())
	//	{
	//		left = &myNodes[i];

	//		// therefore return with right node being nullptr
	//		return std::make_pair(left, nullptr);
	//	}

	//	if (myNodes[i].GetThreshold() <= GetBlendValue())
	//	{
	//		left = &myNodes[i];
	//	}
	//	else
	//	{
	//		indexToContinueAt = i;
	//		break;
	//	}
	//}

	//// Find the right node
	//for (int i = static_cast<int>(myNodes.size()) - 1; i >= indexToContinueAt; --i)
	//{
	//	if (myNodes[i].GetThreshold() >= GetBlendValue())
	//	{
	//		right = &myNodes[i];
	//	}
	//	else
	//	{
	//		break;
	//	}
	//}

	//assert(left && right && "we might need to change the way we calculate the blend value");

	//return std::make_pair(left, right);
}

void Engine::BlendTree2D::RemoveVec2Usage(const std::string& aFloatName)
{
	if (myBlendVec2Name == aFloatName)
	{
		myBlendVec2Name = "";
		myBlendValue = nullptr;
	}
}

const Vec2f& Engine::BlendTree2D::GetBlendValue() const
{
	return *myBlendValue;
}

int Engine::BlendTree2D::GetNodeIndexFromPosition(const Vec2f& aPosition) const
{
	for (int i = 0; i < myNodes.size(); ++i)
	{
		if (myNodes[i].GetPosition() == aPosition)
		{
			return i;
		}
	}

	return -1;
}

void Engine::BlendTree2D::Barycentric(const Vec2f& p, const Vec2f& a, const Vec2f& b, const Vec2f& c, float& u, float& v, float& w)
{
	// Filip says thank you to: https://gamedev.stackexchange.com/a/23745
	Vec2f v0 = b - a, v1 = c - a, v2 = p - a;
	float d00 = v0.Dot(v0);
	float d01 = v0.Dot(v1);
	float d11 = v1.Dot(v1);
	float d20 = v2.Dot(v0);
	float d21 = v2.Dot(v1);
	float denom = d00 * d11 - d01 * d01;
	v = (d11 * d20 - d01 * d21) / denom;
	w = (d00 * d21 - d01 * d20) / denom;
	u = 1.0f - v - w;
}

float Engine::BlendTree2D::DistanceToTriangle(
	const Vec2f& aPosition,
	const delaunay::Triangle<float>& aTriangle)
{
	float abDistance = DistanceToEdge(aPosition, aTriangle.e0);
	float bcDistance = DistanceToEdge(aPosition, aTriangle.e1);
	float caDistance = DistanceToEdge(aPosition, aTriangle.e2);

	return CU::Max(CU::Max(abDistance, bcDistance), caDistance);
}

float Engine::BlendTree2D::DistanceToEdge(const Vec2f& aPosition, const delaunay::Edge<float>& anEdge)
{
	Vec2f v = Vec2f(anEdge.p0.x, anEdge.p0.y);
	Vec2f w = Vec2f(anEdge.p1.x, anEdge.p1.y);

	const float l2 = (w - v).LengthSqr();
	if (l2 == 0.0f)
		return (aPosition - v).Length();

	const float t = CU::Max(0.0f, CU::Min(1.0f, (aPosition - v).Dot(Vec2f(w - v)) / l2));
	const Vec2f projection = v + t * (w - v);
	return (aPosition - projection).Length();
}

Engine::BlendTree2D::Triangle& Engine::BlendTree2D::FindClosestTriangle(const Vec2f& aPosition)
{
	float currentClosestDist = DistanceToTriangle(aPosition, myTriangles[0].myDelaTri);
	int closestTriangleIndex = 0;

	for (size_t i = 1; i < myTriangles.size(); ++i)
	{
		const float dist = DistanceToTriangle(aPosition, myTriangles[i].myDelaTri);

		if (dist < currentClosestDist)
		{
			currentClosestDist = dist;
			closestTriangleIndex = i;
		}
	}

	return myTriangles[closestTriangleIndex];
}

void Engine::BlendTree2D::InitFromJson2(const nlohmann::json& aJson, AnimationMachine& aMachine)
{
	myMachine = &aMachine;

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
		BlendNode2D blendNode;
		blendNode.InitFromJson(node, aMachine);

		myNodes.push_back(std::move(blendNode));
	}

	if (aJson.contains("Triangles"))
	{
		for (const auto& triangleJson : aJson["Triangles"])
		{
			Triangle triangle;
			triangle.myNodeIndex1 = triangleJson["Triangles"][0];
			triangle.myNodeIndex2 = triangleJson["Triangles"][1];
			triangle.myNodeIndex3 = triangleJson["Triangles"][2];
			myTriangles.push_back(triangle);
		}
	}

	// Disgucsting
	RecalculateTriangles();

	if (aJson.contains("BlendVec2Name"))
	{
		myBlendVec2Name = aJson["BlendVec2Name"];

		if (aMachine.GetVec2s().find(myBlendVec2Name) != aMachine.GetVec2s().end())
		{
			myBlendValue = aMachine.GetVec2s().at(myBlendVec2Name).get();
		}
	}
}

nlohmann::json Engine::BlendTree2D::ToJson() const
{
	nlohmann::json j;

	j["Type"] = "BlendTree2D";

	j["DurationSeconds"] = myDurationSeconds;
	j["IsLooping"] = myIsLooping;

	j["BlendVec2Name"] = myBlendVec2Name;

	nlohmann::json nodeArray = nlohmann::json::array();

	for (const auto& node : myNodes)
	{
		nodeArray.push_back(node.ToJson());
	}

	j["Nodes"] = nodeArray;

	nlohmann::json triangleArray = nlohmann::json::array();

	for (const auto& triangle : myTriangles)
	{
		nlohmann::json triangleJson;
		triangleJson["Triangles"][0] = triangle.myNodeIndex1;
		triangleJson["Triangles"][1] = triangle.myNodeIndex2;
		triangleJson["Triangles"][2] = triangle.myNodeIndex3;

		triangleArray.push_back(triangleJson);
	}

	j["Triangles"] = triangleArray;

	return j;
}

void Engine::BlendTree2D::SetElapsedTimeNormalized(const float aTimeNormalized)
{
	myNormalizedTime = aTimeNormalized;
}

void Engine::BlendTree2D::SetIsRootMotion(const bool aIsRootMotion)
{
	throw std::logic_error("The method or operation is not implemented.");
}

void Engine::BlendTree2D::SetTimeScale(const float aTimeScale)
{
	throw std::logic_error("The method or operation is not implemented.");
}

float Engine::BlendTree2D::GetTimeScale() const
{
	throw std::logic_error("The method or operation is not implemented.");
}

const std::string& Engine::BlendTree2D::GetName() const
{
	return myName;
}

std::vector<const Engine::AnimationEvent*> Engine::BlendTree2D::GetEventsExecutedThisFrame()
{
	std::vector<const Engine::AnimationEvent*> events;

	for (auto& node : myNodes)
	{
		auto nodeEvents = node.GetPlayable()->GetEventsExecutedThisFrame();

		events.insert(events.end(), nodeEvents.begin(), nodeEvents.end());
	}

	return events;
}

bool Engine::BlendTree2D::IsLooping() const
{
	return myIsLooping;
}

void Engine::BlendTree2D::SetIsLooping(const bool aIsLooping)
{
	// This is not defined, since we can have both animations that does loop, and does not loop

	throw std::logic_error("The method or operation is not implemented.");
}

bool Engine::BlendTree2D::CanPlay() const
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

void Engine::BlendTree2D::Accept(PlayableVisitor& aVisitor)
{
	aVisitor.Visit(*this);
}

delaunay::Delaunay<float> Engine::BlendTree2D::TriangulateNodes() const
{
	std::vector<delaunay::Point<float>> vertices;

	for (int i = 0; i < myNodes.size(); ++i)
	{
		auto& node = myNodes[i];
		delaunay::Point<float> point(node.GetPosition().x, node.GetPosition().y, 0.f);
		vertices.push_back(point);
	}

	return delaunay::triangulate<float>(vertices);
}
