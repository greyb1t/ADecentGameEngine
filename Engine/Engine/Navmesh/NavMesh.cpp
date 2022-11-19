#include "pch.h"
#include "NavMesh.h"
#include "Engine\DebugManager\DebugDrawer.h"
#include <Engine\Engine.h>

#include "Engine/Scene/Scene.h"
#include "Engine/Scene/SceneManager.h"

NavMesh::NavMesh::NavMesh()
{
}

NavMesh::NavMesh::~NavMesh()
{
}

void NavMesh::NavMesh::HeapTest()
{
	std::vector<TriangleInfo> triangleInfo;

	for (size_t i = 0; i < 100; i++)
	{
		triangleInfo.push_back(TriangleInfo());
		triangleInfo.back().myCurrentDistance = static_cast<float>(i);
	}

	CU::Heap<TriangleInfo> heap;

	for (auto& info : triangleInfo)
	{
		heap.Enqueue(info);
	}

	float lastDistance = -1.0f;
	for (size_t i = 0; i < heap.GetSize(); i++)
	{
		const int distance = heap.Dequeue().myCurrentDistance;

		assert(lastDistance < distance);

		lastDistance = distance;
	}
}

std::vector<CU::Vector3f> NavMesh::NavMesh::Pathfind(const CU::Vector3f& aStartPos, const CU::Vector3f& aEndPos)
{
    //Triangle* startTirangle = IsInside(aStartPos);
	//Triangle* endTirangle = IsInside(aEndPos);
	Triangle* startTirangle = GetClosestTriangle(aStartPos);
	Triangle* endTirangle = GetClosestTriangle(aEndPos);

	if (!startTirangle || !endTirangle)
		return std::vector<CU::Vector3f>();

	std::vector<CU::Vector3f> path;

	if (startTirangle == endTirangle) //Ifall man är samma triangel så behövs inte a*
	{
		path.push_back(aEndPos);
		return path;
	}

	path = AStar(*startTirangle, *endTirangle);

	if (!path.empty())
		path.insert(path.begin(), aEndPos); //Ifall a* hitta en path så lägg till EndPos som slutdestination

	StringPull(path);

	if (path.size() > 1)
		path.pop_back(); //Behöver inte gå mot triangeln man står på

	return path;
}

bool NavMesh::NavMesh::IsPositionOnNavMesh(const CU::Vector3f& aPos)
{
	Triangle* startTirangle = IsInside(aPos);

	if (!startTirangle)
		return false;

	return true;
}

bool NavMesh::NavMesh::ArePositionsOnNavMesh(const CU::Vector3f& aStartPos, const CU::Vector3f& aEndPos)
{
	if (!IsPositionOnNavMesh(aStartPos))
		return false;
	if (!IsPositionOnNavMesh(aEndPos))
		return false;

	return true;
}

bool NavMesh::NavMesh::ArePositionsSameTriangle(const CU::Vector3f& aFirstPos, const CU::Vector3f& aSecondPos)
{
	Triangle* firstTirangle = IsInside(aFirstPos);
	Triangle* secondTirangle = IsInside(aSecondPos);

	if (!firstTirangle || !secondTirangle)
		return false;

	if (firstTirangle == secondTirangle)
		return true;

	return false;
}

void NavMesh::NavMesh::GetClosestTrianglePos(const CU::Vector3f& aPos, CU::Vector3f& aOutPos)
{
	//if (myTriangles.empty())
	//	return;

	//float closestDistance = FLT_MAX;
	//CU::Vector3f closestPos;

	//for (auto& triangle : myTriangles)
	//{
	//	float distance = (aPos - triangle.myMiddlePos).LengthSqr();

	//	if (distance < closestDistance)
	//	{
	//		closestDistance = distance;
	//		closestPos = triangle.myMiddlePos;
	//	}
	//}

	//aOutPos = closestPos;

	Triangle* result = GetClosestTriangle(aPos);

	if (!result)
		return;

	aOutPos = result->myMiddlePos;
}

NavMesh::Triangle* NavMesh::NavMesh::GetClosestTriangle(const CU::Vector3f& aPos)
{
	if (myTriangles.empty())
		return nullptr;


	float closestDistance = FLT_MAX;
	Triangle* closestTriangle = nullptr;

	for (auto& triangle : myTriangles)
	{
		const float distance = (aPos - triangle.myMiddlePos).LengthSqr();

		if (distance < closestDistance)
		{
			closestDistance = distance;
			closestTriangle = &triangle;
		}
	}

	return closestTriangle;
}

void NavMesh::NavMesh::DrawMesh()
{
	const CU::Vector4f colour = { 0.6f, 0.2f, 0.2f, 1.0f };

	auto& debugDrawer = GetEngine().GetSceneManager().GetSceneBeingUpdatedNow()->GetRendererScene().GetDebugDrawer();

	for (Triangle& triangle : myTriangles)
	{
		debugDrawer.DrawLine3D(DebugDrawFlags::Navmesh, triangle.myVetrices[0].myPosition, triangle.myVetrices[1].myPosition, 0.0f, colour);
		debugDrawer.DrawLine3D(DebugDrawFlags::Navmesh, triangle.myVetrices[1].myPosition, triangle.myVetrices[2].myPosition, 0.0f, colour);
		debugDrawer.DrawLine3D(DebugDrawFlags::Navmesh, triangle.myVetrices[2].myPosition, triangle.myVetrices[0].myPosition, 0.0f, colour);


		debugDrawer.DrawLine3D(DebugDrawFlags::Navmesh, (triangle.myMiddlePos - CU::Vector3f({ 1.1f, 0.0f, 0.0f })),
							   (triangle.myMiddlePos + CU::Vector3f({ 1.1f, 0.0f, 0.0f })), 0.0f, colour);

		debugDrawer.DrawLine3D(DebugDrawFlags::Navmesh, (triangle.myMiddlePos - CU::Vector3f({ 0.0f, 0.0f, 1.1f })),
							   (triangle.myMiddlePos + CU::Vector3f({ 0.0f, 0.0f, 1.1f })), 0.0f, colour);
	}
}

const std::vector<NavMesh::Vertex>& NavMesh::NavMesh::GetVertices() const
{
	return myVertices;
}

const std::vector<NavMesh::Triangle>& NavMesh::NavMesh::GetTriangles() const
{
	return myTriangles;
}

void NavMesh::NavMesh::Init(const std::vector<Triangle>& aTriangleList)
{
	myTriangles = aTriangleList;
}

NavMesh::Triangle* NavMesh::NavMesh::IsInside(const CU::Vector3f& aPosition) //this calculation doesnt account for slopes
{
	std::vector<Triangle*> trianglesInside;

	for (Triangle& triangle : myTriangles)
	{
		const float triangleArea = GetTriangleArea(triangle.myVetrices[0].myPosition, triangle.myVetrices[1].myPosition, triangle.myVetrices[2].myPosition);

		const float area1 = GetTriangleArea(aPosition, triangle.myVetrices[1].myPosition, triangle.myVetrices[2].myPosition);
		const float area2 = GetTriangleArea(triangle.myVetrices[0].myPosition, aPosition, triangle.myVetrices[2].myPosition);
		const float area3 = GetTriangleArea(triangle.myVetrices[0].myPosition, triangle.myVetrices[1].myPosition, aPosition);

		const float totalArea = area1 + area2 + area3;

		const float wiggleRoom = triangleArea * 0.005f;
		if (totalArea >= triangleArea - wiggleRoom && totalArea <= triangleArea + wiggleRoom)
		{
			trianglesInside.push_back(&triangle);
		}
	}

	if (trianglesInside.size() == 1)
	{
		return trianglesInside.back();
	}

	Triangle* closesTriangle = nullptr;
	float closestDistance = FLT_MAX;
	for (size_t i = 0; i < trianglesInside.size(); i++)
	{
		auto triangle = trianglesInside[i];

		float distance = abs(aPosition.y - triangle->myMiddlePos.y); //Kör bara y distance för ifall man hade kört från middlePos så kan storlek på triangel påverka resultat

		if (distance < closestDistance)
		{
			closesTriangle = triangle;
			closestDistance = distance;
		}
	}

	return closesTriangle;
}

CU::Vector3f NavMesh::NavMesh::GetRandomPosition()
{
	int index = Random::RandomInt(0, myNumberOfTriangles - 1);
	CU::Vector3f position = myTriangles[index].myMiddlePos;
	//TODO: Randomize more exact positions?
	return position;
}

float NavMesh::NavMesh::GetTriangleArea(const CU::Vector3f& aFirstPos, const CU::Vector3f& aSecondPos, const CU::Vector3f& aThirdPos) const //Herons formula
{
	const float side1 = (CU::Vector2f({ aFirstPos.x, aFirstPos.z }) - CU::Vector2f({ aSecondPos.x, aSecondPos.z })).Length();
	const float side2 = (CU::Vector2f({ aFirstPos.x, aFirstPos.z }) - CU::Vector2f({ aThirdPos.x, aThirdPos.z })).Length();
	const float side3 = (CU::Vector2f({ aSecondPos.x, aSecondPos.z }) - CU::Vector2f({ aThirdPos.x, aThirdPos.z })).Length();

	const float s = (side1 + side2 + side3) / 2.0f;

	const float area = s * (s - side1) * (s - side2) * (s - side3);
	const float sqrtArea = sqrtf(area);

	return sqrtArea;
}

std::vector<CU::Vector3f> NavMesh::NavMesh::AStar(const Triangle& aStartTriangle, const Triangle& aEndTriangle)
{
	if (aStartTriangle.myMiddlePos == aEndTriangle.myMiddlePos)
	{
		return std::vector<CU::Vector3f>();
	}

	int startIndex = 0;
	int endIndex = 0;

	std::vector<TriangleInfo> triangleInfo;
	triangleInfo.reserve(myTriangles.size());

	for (size_t i = 0; i < myTriangles.size(); i++)
	{
		triangleInfo.push_back(TriangleInfo());
		triangleInfo.back().myTriangle = &myTriangles[i];
		triangleInfo.back().myCurrentDistance = FLT_MAX;
		triangleInfo.back().myIndex = i;

		if (myTriangles[i].myMiddlePos == aStartTriangle.myMiddlePos)
		{
			startIndex = i;
		}
		else if (myTriangles[i].myMiddlePos == aEndTriangle.myMiddlePos)
		{
			endIndex = i;
		}

		myTriangles[i].myInfo = &triangleInfo.back();
	}

	triangleInfo[startIndex].myCurrentDistance = 0.0f;

	CU::Heap<TriangleInfo> toBeChecked;
	toBeChecked.Enqueue(triangleInfo[startIndex]);

	while (toBeChecked.GetSize() != 0)
	{
		TriangleInfo triangleToCheck = toBeChecked.Dequeue();

		const CU::Vector3f& triangleToCheckPos = triangleToCheck.myTriangle->myMiddlePos;
		const CU::Vector3f& endPos = myTriangles[endIndex].myMiddlePos;

		if (triangleToCheckPos == endPos)
		{
			std::vector<CU::Vector3f> result;

			TriangleInfo* currentTriangle = &triangleToCheck;
			while (currentTriangle)
			{
				result.push_back(currentTriangle->myTriangle->myMiddlePos);
				currentTriangle = triangleInfo[currentTriangle->myIndex].myPredecessor;
			}

			//std::reverse(result.begin(), result.end()); //Vill kuuna köra pop_back() på movementListan. därmed så reversar vi inte den

			return result;
		}

		std::vector<TriangleInfo*> currentNeighbours;

		for (Triangle* neighbour : triangleToCheck.myTriangle->myNeighbours)
		{
			currentNeighbours.push_back(neighbour->myInfo);
		}

		for (size_t i = 0; i < currentNeighbours.size(); i++)
		{
			const CU::Vector3f& neighbourPos = currentNeighbours[i]->myTriangle->myMiddlePos;

			const float hValue = HeuristicCalculation(neighbourPos, endPos);

			const float gValue = (triangleToCheck.myCurrentDistance + (triangleToCheckPos - neighbourPos).LengthSqr());

			const float fValue = gValue + hValue;

			if (fValue < currentNeighbours[i]->myCurrentDistance)
			{
				TriangleInfo& newToBeChecked = triangleInfo[currentNeighbours[i]->myIndex];

				newToBeChecked.myCurrentDistance = gValue;
				newToBeChecked.myPredecessor = &triangleInfo[triangleToCheck.myIndex];

				toBeChecked.Enqueue(newToBeChecked);
			}
		}
	}

	return std::vector<CU::Vector3f>();
}

float NavMesh::NavMesh::HeuristicCalculation(const CU::Vector3f& aStartPos, const CU::Vector3f& aEndPos) const
{
	const float distance = (aStartPos - aEndPos).LengthSqr();

	return distance;
}

void NavMesh::NavMesh::StringPull(std::vector<CU::Vector3f>& aOutList)
{
	bool changed = true;
	while (changed)
	{
		changed = false;

		for (int i = aOutList.size() - 1; i >= 2; i--)
		{
			if (CanStringPull(aOutList[i], aOutList[i - 2]))
			{
				changed = true;

				aOutList.erase(aOutList.begin() + (i - 1));
			}
		}
	}
}

bool NavMesh::NavMesh::CanStringPull(const CU::Vector3f& aFirstPos, const CU::Vector3f& aSecondPos)
{
	constexpr  float checkDistance = 5.f;

	const CU::Vector3f dir = (aSecondPos - aFirstPos).GetNormalized();
	const float distance = (aSecondPos - aFirstPos).Length();

	const int ammountOfChecks = static_cast<int>(distance / checkDistance);

	bool isValidPath = true;
	for (int i = 1; i <= ammountOfChecks; i++)
	{
		const auto triangle = IsInside(aFirstPos + dir * (checkDistance * i));

		if (triangle == nullptr) //TODO: STRING PULLISH IS KINDA SHITT, NEED BETTER CHECK
		{
			isValidPath = false;
		}
		else if (abs(aFirstPos.y - aSecondPos.y) < 100.0f && abs(triangle->myMiddlePos.y - aFirstPos.y) > 100.f)
		{
			isValidPath = false;
		}

	}

	return isValidPath;
}