#pragma once
#include "Engine/ResourceManagement/ResourceRef.h"
#include "Physics/Shape.h"

namespace Engine
{
	class Scene;
	class Entity;
	class ResourceReferences;
}

class GameObject;

struct UTransform
{
	CU::Vector3f myPosition;
	CU::Vector3f myRotationEuler;
	CU::Quaternion myRotationQuat;
	CU::Vector3f myScale = { 1.f, 1.f, 1.f };
};

struct ColliderStruct
{
	Shape shape;
	Common::Vector3f center;
	Common::Quaternion rotation;
};

namespace LevelImporter
{
	class UnrealProject
	{
	public:
		bool Init(Engine::Scene& aScene, Engine::ResourceReferences& aResourceReferences);

		bool LoadWorld(const std::string aJsonPath);

	private:
		void CreateActorEntity(const nlohmann::json& aActor, const UTransform& aParentActorTransform, GameObject* aParent);

		void CreateActorComponents(
			GameObject& aActorEntity, 
			const UTransform& aParentActorTransform,
			UTransform& aActorTransform,
			const nlohmann::json& aComponent,
			GameObject* aParent);

	private:
		Engine::Scene* myScene = nullptr;
		Engine::ResourceReferences* myResourceReferences = nullptr;

		VFXRef myGreenFallingLeafVfxRef;
		VFXRef myOrangeFallingLeafVfxRef;

		void LoadLandscape(std::string aLandscapeName, GameObject& aGameObjectActor);
	};
}
