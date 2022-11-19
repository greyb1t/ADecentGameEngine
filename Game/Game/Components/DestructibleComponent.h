#pragma once
#include "Engine/GameObject/Components/RigidBodyComponent.h"
#include "Engine\GameObject\Components\Component.h"
#include "Engine/Renderer/MeshPointsData.h"

namespace Destructible
{
	enum class eDamageType
	{
	    OnlyAffectHits,
	    ShatterOnDamage,
	    RigidOnDamage,
	};
}

// Considering doing some kind of fracture system where it can impact closeby fractures. Not sure \0/

struct DestructibleShape
{
    Shape shape;
    float mass;
    int childIndex = -1;
};

class DestructibleComponent :
    public Component
{
public:
    COMPONENT(DestructibleComponent, "Destructible Component");

    DestructibleComponent() = default;
    DestructibleComponent(GameObject* aGameObject);

    void Start() override;
    void Execute(Engine::eEngineOrder aOrder) override;

    void Attach(Shape aShape, float aMass, const std::string& aModelPath);
    void SetKinematic(bool isKinematic);
    void SetDamageResponse(Destructible::eDamageType);

    /**
     * \brief Applies damage to a specific shape.
     * \param aLocalPosition This is relative to the gameobject position, it will automatically convert it into the shapes local position space
     */
    void ApplyDamage(Shape aShape, const Vec3f& aForce, const Vec3f& aLocalPosition = { 0, 0, 0 });

    /**
     * \brief Simulate an explosion at a position with a force and range, 
     * \param aPosition Source position
     * \param aForce A maximum force
     * \param aRange The range of explosion
     * \param aDir A optional direction for force calculation. This will modify which direction fractures will be projected towards.
     * \param aDirPercentage Modifies the percentage of the direction is based on the dir parameter, 1.f = full dir, 0.f = No impact on direction and fully from position - fracture position 
     */
    void ApplyExplosion(const Vec3f& aPosition, float aForce, float aRange, const Vec3f& aDir = { 0.f, 0.f, 0.f }, float aDirPercentage = 0.f);


    void Reflect(Engine::Reflector& aReflector) override;
private:
    std::vector<MeshPointsData> GetMeshes();

    void Attach(Shape aShape, float aMass, GameObject* aGameObject, int aIndex);
    Engine::RigidBodyComponent* DetachShape(DestructibleShape& aShape);

    void ApplyResponse();

    void AddDestructableShape(DestructibleShape aShape);
    DestructibleShape RemoveDestructableShape(int aIndex);
private:
    Engine::RigidBodyComponent* myRb = nullptr;

    std::string myModelPath;

    bool myIsKinematic = true;
    Destructible::eDamageType myResponse = Destructible::eDamageType::OnlyAffectHits;
    int myResponseTypeID = 0;
    std::vector<DestructibleShape> myShapes;
    float myMass = 1.f;
};

