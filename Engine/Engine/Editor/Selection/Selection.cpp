#include "pch.h"
#include "Selection.h"

#include "Engine/GameObject/GameObject.h"

void Engine::Selection::Update()
{
	myChanged = false;
}

void Engine::Selection::SelectGameObject(Weak<GameObject> aGameObject)
{
	myGameObjects.push_back(aGameObject);
	myChanged = true;
}

void Engine::Selection::DeselectGameObject(GameObject* aGameObject)
{
	const auto findResult = std::find_if(
		myGameObjects.begin(),
		myGameObjects.end(),
		[aGameObject](const Weak<GameObject>& aG)
		{
			return aG.lock().get() == aGameObject;
		});

	if (findResult != myGameObjects.end())
	{
		myGameObjects.erase(findResult);
		myChanged = true;
	}
}

void Engine::Selection::Clear()
{
	myGameObjects.clear();
}

int Engine::Selection::GetCount() const
{
	return myGameObjects.size();
}

GameObject* Engine::Selection::GetAt(const int aIndex)
{
	return myGameObjects[aIndex].lock().get();
}

Weak<GameObject> Engine::Selection::GetAtWeak(const int aIndex)
{
	return myGameObjects[aIndex];
}

bool Engine::Selection::Exists(const GameObject* aGameObject) const
{
	//return std::find(
	//	myGameObjects.begin(),
	//	myGameObjects.end(),
	//	aGameObject) != myGameObjects.end();

	const auto findResult = std::find_if(
		myGameObjects.begin(),
		myGameObjects.end(),
		[aGameObject](const Weak<GameObject>& aG)
		{
			return aG.lock().get() == aGameObject;
		});

	return findResult != myGameObjects.end();
}

Engine::LiteTransform Engine::Selection::CalculateCenterTransform() const
{
	Vec3f totalPosition;
	Vec3f totalRotation;
	Vec3f totalScale;
	//Quatf totalRotation;

	for (const auto g : myGameObjects)
	{
		if (auto g2 = g.lock())
		{
			totalPosition += g2->GetTransform().GetPosition();
			totalRotation += g2->GetTransform().GetRotation().EulerAngles();
			totalScale += g2->GetTransform().GetScale();
		}
	}

	totalPosition = totalPosition.Scale(Vec3f::One * (1.f / static_cast<float>(myGameObjects.size())));
	totalRotation = totalRotation.Scale(Vec3f::One * (1.f / static_cast<float>(myGameObjects.size())));
	totalScale = totalScale.Scale(Vec3f::One * (1.f / static_cast<float>(myGameObjects.size())));

	//totalPosition = myGameObjects.front()->GetTransform().GetPositionWorld();
	//totalRotation = myGameObjects.front()->GetTransform().GetRotationWorld();

	LiteTransform result;

	result.SetPositionLocal(totalPosition);
	result.SetRotationLocal(totalRotation);
	//result.SetScaleLocal(totalScale);

	// LOG_INFO(LogType::Filip) << "Pos: " << totalPosition.x << ", " << totalPosition.y << ", " << totalPosition.z << "... Rot: " << totalRotation.x << ", " << totalRotation.y << ", " << totalRotation.z;

	return result;
}

bool Engine::Selection::WasChanged() const
{
	return myChanged;
}
