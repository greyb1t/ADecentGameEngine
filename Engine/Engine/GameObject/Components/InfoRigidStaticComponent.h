#pragma once
#include "Component.h"
#include "Physics/Shape.h"

class InfoRigidStaticComponent :
    public Component
{
public:
	COMPONENT_COPYABLE(InfoRigidStaticComponent, "IRigidStaticComponent");

	InfoRigidStaticComponent() = default;
	InfoRigidStaticComponent(const InfoRigidStaticComponent&) = default;
	InfoRigidStaticComponent(GameObject* aGameObject);
	~InfoRigidStaticComponent();

	void Start() override;
	void Reflect(Engine::Reflector& aReflector) override;
	void SetTransform(const CU::Quaternion& anQuaternion, const CU::Vector3f& aPosition);
	int GetLayerFromString(const std::string& aLayerName);

	void Attach(Shape aShape);

	void SetTrigger(bool aValue) { myIsTrigger = aValue; }
private:
	std::vector<Shape>	myShapes;
	bool				myIsTrigger = false;
	physx::PxTransform	myPxTransform;
	//eRigidBodyType		myType = eRigidBodyType::DEFAULT;

	Vec3f myPos;
	Quatf myQuat;

	Vec3f mySize;
	Shape myShape;
	std::string myLayer;

	CU::Vector3f myOffset{ 0,0,0 };
};

