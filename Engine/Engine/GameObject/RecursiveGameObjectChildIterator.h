#pragma once

class GameObject;
class Transform;

namespace Engine
{
	class GameObjectChildIterator
	{
	public:
		GameObjectChildIterator(GameObject& aGameObject);

		GameObjectChildIterator() = default;

		GameObjectChildIterator& operator++(int);

		bool operator!=(Transform* aOther) const
		{
			return myCurrentTransform != aOther;
		}

		GameObject* DerefAsRaw() const;
		Shared<GameObject> DerefAsShared() const;

	private:
		Transform* myCurrentTransform = nullptr;

		std::queue<Transform*> myTransformStack;
	};
}