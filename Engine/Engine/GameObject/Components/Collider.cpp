#include "pch.h"
#include "Collider.h"

#include "RigidBodyComponent.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/Physics/PhysicsDebugDrawer.h"
#include "Engine/Reflection/Reflector.h"

std::string EnumGeometry::EnumToString(int aValue)
{
	switch (static_cast<Shape::eGeometry>(aValue))
	{
	case Shape::eBOX:
		return "Box";
	case Shape::eSPHERE:
		return "Sphere";
	case Shape::eCAPSULE:
		return "Capsule";
	case Shape::eCONVEX:
		return "Convex";
	case Shape::eTRIANGLE_MESH:
		return "Triangle Mesh";
	default: ;
	}
	return "Error";
}

int EnumGeometry::GetCount()
{
	return 3;
}

std::string EnumRigidBodyType::EnumToString(int aValue)
{
	switch (static_cast<eRigidBodyType>(aValue))
	{
	case eRigidBodyType::DEFAULT: 
		return "Default";
	case eRigidBodyType::KINEMATIC: 
		return "Kinematic";
	case eRigidBodyType::TRIGGER: 
		return "Trigger";
	default: break;
	}
	return "Error";
}

int EnumRigidBodyType::GetCount()
{
	return 3;
}

Engine::Collider::Collider()
	: Component(nullptr)
{
	properties.box = {};
}

Engine::Collider::Collider(GameObject* aGameObject)
	: Component(aGameObject)
{
	properties.box = {};
}

void Engine::Collider::Reflect(Reflector& aReflector)
{
	aReflector.Reflect(myRigidBodyType, "Collider Type");
	aReflector.Reflect(myGeometry, "Geometry");
	aReflector.Reflect(myOffsetPosition, "Offset Position", ReflectionFlags::ReflectionFlags_ReadOnly);

	switch (static_cast<Shape::eGeometry>(myGeometry.GetValue()))
	{
	case Shape::eBOX: 
		aReflector.Reflect(properties.box.halfSize, "Half Size");
		break;
	case Shape::eSPHERE: 
		aReflector.Reflect(properties.sphere.radius, "Radius");
		break;
	case Shape::eCAPSULE: 
		aReflector.Reflect(properties.capsule.halfHeight, "Half Height");
		aReflector.Reflect(properties.capsule.radius, "Radius");
		break;
	case Shape::eCONVEX: break;
	case Shape::eTRIANGLE_MESH: break;
	default: break;
	}
}

void Engine::Collider::Awake()
{
	eRigidBodyType type = static_cast<eRigidBodyType>(myRigidBodyType.GetValue());
	auto* rb = myGameObject->AddComponent<RigidBodyComponent>(type);

	switch (static_cast<Shape::eGeometry>(myGeometry.GetValue()))
	{
	case Shape::eBOX:
	{
		auto shape = Shape::Box(properties.box.halfSize);
		shape.SetLocalPosition(myOffsetPosition);
		rb->Attach(shape);
	}
	break;
	case Shape::eSPHERE:
	{
		auto shape = Shape::Sphere(properties.sphere.radius);
		shape.SetLocalPosition(myOffsetPosition);
		rb->Attach(shape);
	}
	break;
	case Shape::eCAPSULE:
	{
		auto shape = Shape::Capsule(properties.capsule.halfHeight, properties.capsule.radius);
		shape.SetLocalPosition(myOffsetPosition);
		rb->Attach(shape);
	}
	break;
	case Shape::eCONVEX: break;
	case Shape::eTRIANGLE_MESH: break;
	default:;
	}

	//myGameObject->RemoveComponent(this);
}
void Engine::Collider::Start()
{
	Destroy();
}

void Engine::Collider::EditorExecute()
{
	const auto pos = myGameObject->GetTransform().GetPosition() + myOffsetPosition;

	switch (static_cast<Shape::eGeometry>(myGeometry.GetValue()))
	{
	case Shape::eBOX:
	{
		GDebugDrawer->DrawCube3D(DebugDrawFlags::Always, pos, myGameObject->GetTransform().GetRotation().EulerAngles(), properties.box.halfSize * .5f);
	}
	break;
	case Shape::eSPHERE:
	{
		GDebugDrawer->DrawSphere3D(DebugDrawFlags::Always, pos, properties.sphere.radius);
	}
	break;
	case Shape::eCAPSULE:
	{
		const auto radius = properties.capsule.radius;
		const auto height = properties.capsule.halfHeight;
		auto color =  static_cast<eRigidBodyType>(myRigidBodyType.GetValue()) == eRigidBodyType::DEFAULT ? P_RIGID_COLOR :
			(static_cast<eRigidBodyType>(myRigidBodyType.GetValue()) == eRigidBodyType::KINEMATIC ? P_KINEMATIC_COLOR : P_TRIGGER_COLOR);

		GDebugDrawer->DrawLine3D(DebugDrawFlags::Always,
				pos + CU::Vector3f{ radius, height * .5f, 0 },
				pos - CU::Vector3f{ -radius, height * .5f, 0 },
				0,
				color);
		GDebugDrawer->DrawLine3D(DebugDrawFlags::Always,
				pos + CU::Vector3f{ 0, height * .5f, radius },
				pos - CU::Vector3f{ 0, height * .5f, -radius },
				0,
				color);
		GDebugDrawer->DrawLine3D(DebugDrawFlags::Always,
				pos + CU::Vector3f{ -radius, height * .5f, 0 },
				pos - CU::Vector3f{ radius, height * .5f, 0 },
				0,
				color);
		GDebugDrawer->DrawLine3D(DebugDrawFlags::Always,
			pos + CU::Vector3f{ 0, height * .5f, -radius },
			pos - CU::Vector3f{ 0, height * .5f, radius },
			0,
			color);

		GDebugDrawer->DrawSphere3D(DebugDrawFlags::Always,
			pos + CU::Vector3f{ 0, height * .5f, 0 },
			radius,
			0,
			color);
		GDebugDrawer->DrawSphere3D(DebugDrawFlags::Always,
			pos - CU::Vector3f{ 0, height * .5f, 0 },
			radius,
			0,
			color);
	}
	break;
	case Shape::eCONVEX: break;
	case Shape::eTRIANGLE_MESH: break;
	default:;
	}
}

const Vec3f& Engine::Collider::GetOffsetPosition() const
{
	return myOffsetPosition;
}

void Engine::Collider::SetRadius(const float aRadius)
{
	if (myGeometry.GetValue() != Shape::eSPHERE)
		return;

	properties.sphere.radius = aRadius;
}