#pragma once

#include "Engine/LiteTransform.h"

class GameObject;

namespace Engine
{
	class Selection
	{
	public:
		void Update();

		void SelectGameObject(Weak<GameObject> aGameObject);
		void DeselectGameObject(GameObject* aGameObject);
		void Clear();

		int GetCount() const;

		GameObject* GetAt(const int aIndex);
		Weak<GameObject> GetAtWeak(const int aIndex);

		bool Exists(const GameObject* aGameObject) const;

		LiteTransform CalculateCenterTransform() const;

		bool WasChanged() const;

		bool operator==(const Selection& aOther) const
		{
			if (myGameObjects.size() != aOther.myGameObjects.size())
			{
				return false;
			}

			for (size_t i = 0; i < myGameObjects.size(); ++i)
			{
				if (myGameObjects[i].lock() != aOther.myGameObjects[i].lock())
				{
					return false;
				}
			}

			return true;
		}

	private:
		std::vector<Weak<GameObject>> myGameObjects;

		bool myChanged = false;
	};
}