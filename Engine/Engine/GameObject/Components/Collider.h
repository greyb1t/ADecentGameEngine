#pragma once
#include "Component.h"
#include "Engine/Reflection/Enum.h"
#include "Physics/Shape.h"

struct EnumGeometry : public Engine::Enum
{
public:
	std::string EnumToString(int aValue);
	int GetCount();
};
struct EnumRigidBodyType : public Engine::Enum
{
public:
	std::string EnumToString(int aValue);
	int GetCount();
};

namespace Engine
{
	class Collider :
	    public Component
	{
	public:
		Collider();
		Collider(GameObject*);
		Collider(const Collider&) = default;

		Component* Clone() const override { return DBG_NEW Collider(*this); }
		void Reflect(Reflector& aReflector) override;

		void Awake() override;
		void Start() override;
		void EditorExecute() override;

		// WARNING: Will only work if the collider is a SphereCollider. Don't call this on other objects.
		void SetRadius(const float aRadius);

		const Vec3f& GetOffsetPosition() const;

	private:
		EnumGeometry myGeometry;
		EnumRigidBodyType myRigidBodyType;
		Vec3f myOffsetPosition = {};

		union Properties
		{
			Properties()
			{
				box.halfSize = { 100,100,100 };
			}

			struct
			{
				Vec3f halfSize = { 100, 100, 100 };
			} box;

			struct
			{
				float radius = 100;
			} sphere;

			struct
			{
				float radius = 10;
				float halfHeight = 40;
			} capsule;
		} properties;
	};
}