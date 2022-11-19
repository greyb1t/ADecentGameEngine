#include "pch.h"
#include "Transform.h"
#include "GameObject.h"
#include <Engine\GameObject\RecursiveGameObjectChildIterator.h>

Transform::Transform(const Shared<GameObject>& aGameObject)
	: myGameObject(aGameObject)
{
}

Transform::~Transform()
{
	ReleaseTransform();
}

GameObject* Transform::GetGameObject()
{
	if (auto p = myGameObject.lock())
	{
		return p.get();
	}

	return nullptr;
}

const GameObject* Transform::GetGameObject() const
{
	if (auto p = myGameObject.lock())
	{
		return p.get();
	}

	return nullptr;
}

Shared<GameObject> Transform::GetGameObjectShared() const
{
	if (auto p = myGameObject.lock())
	{
		return p;
	}

	return nullptr;
}

void Transform::SetParent(Transform* aParentTransform)
{
	if (myParent)
	{
		RemoveParent();
	}

	myPosition = myPosition - aParentTransform->GetPosition();

	myParent = aParentTransform;
	myParent->AddChild(this);
}

void Transform::SetParent2(Transform* aParentTransform)
{
	if (myParent)
	{
		RemoveParent2();
	}

	myParent = aParentTransform;
	myParent->AddChild(this);
}

void Transform::SetPosition(const Vec3f& aPos)
{
	if (myParent != nullptr)
	{
		//myPosition = aPos - myParent->GetPosition();
		myPosition = (aPos.ToVec4(1.f) * myParent->GetMatrix().Inverse()).ToVec3();
	}
	else
	{
		myPosition = aPos;
	}
}

void Transform::Move(const Vec3f& aMovement)
{
	if (myIsPhysicalBody)
	{
		myMovement += aMovement;
		return;
	}

	myPosition += aMovement;
}

void Transform::SetRotation(const Quatf& aQuaternion)
{
	if (myParent != nullptr)
	{
		// NOTE(filip): this may be wrong, I have not tested
		myRotation = aQuaternion * myParent->GetRotation().GetInverse();
	}
	else
	{
		myRotation = aQuaternion;
	}
}

void Transform::Rotate(const Quatf& aQuaternion)
{
	if (myIsPhysicalBody)
	{
		myRotated = aQuaternion * myRotated;
		return;
	}

	myRotation = aQuaternion * myRotation;
}

void Transform::SetScale(const Vec3f& aScale)
{
	//myScale = aScale;
	if (myParent != nullptr)
	{
		myScale = aScale * (1.f / myParent->GetScale());
	}
	else
	{
		myScale = aScale;
	}
}

void Transform::SetScale(const float aScale)
{
	SetScale(Vec3f(aScale, aScale, aScale));
}

void Transform::SetScaleX(const float aScale)
{
	SetScale(Vec3f(aScale, myScale.y, myScale.z));
}

Vec3f Transform::GetScale() const
{
	if (myParent != nullptr)
	{
		return myScale.Scale(myParent->GetScale());
	}
	else
	{
		return myScale;
	}
}

Transform* Transform::GetParent() const
{
	return myParent;
}

bool Transform::HasParent() const
{
	return myParent != nullptr;
}

Quatf Transform::GetRotation() const
{
	if (myParent != nullptr)
	{
		return myRotation * myParent->GetRotation();
	}
	else
	{
		return myRotation;
	}
}

const Vec3f Transform::GetPosition() const
{
	if (myParent != nullptr)
	{
		const Vec3f scaledLocalPos = myPosition.Scale(myParent->myScale);

		const Vec3f toLocalPosFromParent = CU::Vector3f(
			scaledLocalPos.x,
			scaledLocalPos.y,
			scaledLocalPos.z);

		const auto rotatedLocalPos = toLocalPosFromParent * myParent->GetRotation();

		return myParent->GetPosition() + rotatedLocalPos;
	}
	else
	{
		return myPosition;
	}
}

void Transform::SetPhysical(bool aValue)
{
	myIsPhysicalBody = aValue;
}

bool Transform::IsPhysical() const
{
	return myIsPhysicalBody;
}

void Transform::ResetMovement()
{
	myMovement = Vec3f();
	myRotated = Quatf();
}

void Transform::ResetRotate()
{
	myRotated = Quatf();
}

const Vec3f& Transform::GetMovement() const
{
	return myMovement;
}

const Quatf& Transform::GetRotated() const
{
	return myRotated;
}

Transform* Transform::GetChildByIndex(const int aIndex)
{
	return myChildren[aIndex];
}

const std::vector<Transform*>& Transform::GetChildren() const
{
	return myChildren;
}

GameObject* Transform::GetChildByGameObjectName(const std::string& aName, const bool aRecursively)
{
	auto g = myGameObject.lock();

	if (g == nullptr)
		return nullptr;

	if (aRecursively)
	{
		for (auto it = Engine::GameObjectChildIterator(*g); it != nullptr; it++)
		{
			auto child = it.DerefAsRaw();

			if (!child)
				continue;

			if (child->GetName() == aName)
			{
				return child;
			}
		}
	}
	else
	{
		for (auto& childTransform : g->GetTransform().myChildren)
		{
			auto child = childTransform->GetGameObject();
			if (!child)
				continue;

			if (child->GetName() == aName)
			{
				return child;
			}
		}
	}

	return nullptr;
}

bool Transform::HasChildren() const
{
	return !myChildren.empty();
}

Vec3f Transform::Forward() const
{
	return GetMatrix().GetForward();
}

Vec3f Transform::Right() const
{
	return GetMatrix().GetRight();
}

Vec3f Transform::Up() const
{
	return GetMatrix().GetUp();
}

Mat4f Transform::GetMatrix() const
{
	return Mat4f::CreateScale(GetScale())
		* GetRotation().ToMatrix()
		* Mat4f::CreateTranslation(GetPosition());
}

Mat4f Transform::GetMatrixLocal() const
{
	return Mat4f::CreateScale(GetScaleLocal())
		* GetRotationLocal().ToMatrix()
		* Mat4f::CreateTranslation(GetPositionLocal());
}

void Transform::LookAt(const Vec3f& aTargetPosition, const Vec3f& aUp)
{
	auto result = Mat4f::LookAt(GetPosition(), aTargetPosition, aUp);

	Vec3f position;
	Quatf rotation;
	Vec3f scale;
	result.Decompose(position, rotation, scale);

	SetPosition(position);
	SetRotation(rotation);
}

Quatf Transform::GetLookAtResult(const Vec3f& aTargetPosition, const Vec3f& aUp)
{
	auto result = Mat4f::LookAt(GetPosition(), aTargetPosition, aUp);

	Vec3f position;
	Quatf rotation;
	Vec3f scale;
	result.Decompose(position, rotation, scale);

	return rotation;
}

Transform& Transform::operator=(const Transform& aRight)
{
	myPosition = aRight.myPosition;
	myRotation = aRight.myRotation;
	myScale = aRight.myScale;

	return *this;
}

void Transform::AddChild(Transform* aTransform)
{
	myChildren.emplace_back(aTransform);
}

void Transform::RemoveChild(Transform* aTransform)
{
	for (int i = 0; i < myChildren.size(); i++)
	{
		if (myChildren[i] == aTransform)
		{
			myChildren.erase(myChildren.begin() + i);
			return;
		}
	}
}

void Transform::ReleaseTransform()
{
	// NOTE(filip): I don't think we need this
	// Note(Viktor): We REALLY, REALLY need this.
	for (int i = myChildren.size() - 1; i >= 0; i--)
	{
		myChildren[i]->RemoveParent2();
	}

	if (myParent != nullptr)
	{
		myParent->RemoveChild(this);
	}
}

void Transform::RemoveParent()
{
	if (!myParent)
	{
		return;
	}
	Vec3f position = GetPosition();
	Quatf rotation = GetRotation();
	myParent->RemoveChild(this);
	myParent = nullptr;

	myPosition = position;
	myRotation = rotation;
}

Transform* Transform::GetTopMostParent()
{
	if (myParent == nullptr)
	{
		return this;
	}

	Transform* result = myParent;

	for (; result->HasParent(); result = result->GetParent())
	{
	}

	return result;
}

void Transform::RemoveParent2()
{
	if (!myParent)
	{
		return;
	}

	myParent->RemoveChild(this);
	myParent = nullptr;
}

bool operator!=(const Transform& aLeft, const Transform& aRight)
{
	return aLeft.myPosition != aRight.myPosition ||
		aLeft.myRotation != aRight.myRotation ||
		aLeft.myScale != aRight.myScale;
}
