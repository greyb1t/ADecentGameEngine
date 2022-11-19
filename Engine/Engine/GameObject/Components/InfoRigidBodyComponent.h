#pragma once
#include "Component.h"
#include "RigidBodyComponent.h"

class InfoRigidBodyComponent :
	public Component
{
public:
	COMPONENT_COPYABLE(InfoRigidBodyComponent, "IRigidBodyComponent");

	InfoRigidBodyComponent() = default;
	InfoRigidBodyComponent(const InfoRigidBodyComponent&) = default;
	InfoRigidBodyComponent(GameObject* aGameObject, eRigidBodyType aType = eRigidBodyType::DEFAULT);
	~InfoRigidBodyComponent();

	void Start() override;

	void Reflect(Engine::Reflector& aReflector) override;

	void Attach(Shape aShape);
	void				SetMass(float aMass);

	void SetOffset(const CU::Vector3f& aOffset);
	const CU::Vector3f& GetOffset() const;
	void SetLocalRotation(const CU::Quaternion& aRotation);
	const CU::Quaternion& GetLocalRotation() const;
	void SetTransform(const CU::Quaternion& aQuaternion, const CU::Vector3f& aPosition);
	void DebugDrawShape(const Shape& aShape, const CU::Vector3f& aPosition, const CU::Quaternion& aRotation, const CU::Vector4f& aColor);
private:
	std::vector<Shape>	myShapes;
	eRigidBodyType		myType = eRigidBodyType::DEFAULT;
	bool				myIsTrigger = false;

	float myMass;

	//Reflection Variables
	Vec3f mySize = Vec3f(1.f, 1.f, 1.f);
	Shape myShape;
	std::string myLayer;

	CU::Vector3f myOffset{ 0,0,0 };
	CU::Quaternion myLocalRotation;
	physx::PxTransform	myPxTransform;
};

