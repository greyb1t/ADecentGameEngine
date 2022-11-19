#include "pch.h"
#include "UnrealProject.h"

#include <Engine/GameObject/Components/RigidBodyComponent.h>

#include "PhysicsUtilities.h"
#include "Engine/AudioManager.h"
#include "Engine/Engine.h"
#include "Engine/ResourceReferences.h"
#include "Engine/Shortcuts.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/InfoRigidBodyComponent.h"
#include "Engine/GameObject/Components/InfoRigidStaticComponent.h"
#include "Engine/GameObject/Components/LandscapeComponent.h"
#include "Engine/GameObject/Components/ModelComponent.h"
#include "Engine/GameObject/Components/MultiRigidStatic.h"
#include "Engine/GameObject/Components/PointLightComponent.h"
#include "Engine/GameObject/Components/RigidStaticComponent.h"
#include "Engine/GameObject/Components/SpotLightComponent.h"
#include "Engine/Renderer/GraphicsEngine.h"
#include "Engine/Renderer/Material/MaterialFactory.h"
#include "Engine/Renderer/Model/ModelFactory.h"
#include "Engine/ResourceManagement/ResourceManager.h"
#include "Engine/ResourceManagement/Resources/ModelResource.h"
#include "Engine/Scene/Scene.h"
#include "Physics/CookingManager.h"
#include "Physics/Physics.h"
#include "Physics/RigidStatic.h"
#include "Engine/GameObject/Components/FolliageCuller.h"
#include "Engine/GameObject/Components/VFXComponent.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"
#include "Engine/GameObject/Components/VFXSpawnerComponent.h"
#include "Game/Components/VFXSpawner.h"

using namespace nlohmann;

CU::Vector3f ReadVector3(const json& aJson)
{
	return {
		aJson["x"],
		aJson["y"],
		aJson["z"] };
}

CU::Vector4f ReadVector4(const json& aJson)
{
	return {
		aJson["x"],
		aJson["y"],
		aJson["z"],
		aJson["w"] };
}

CU::Quaternion ReadQuaternion(const json& aJson)
{
	const auto rotationQuatAsVector = ReadVector4(aJson);
	const auto quaternion = CU::Quaternion(rotationQuatAsVector.w,
		CU::Vector3f(rotationQuatAsVector.x, rotationQuatAsVector.y, rotationQuatAsVector.z));

	return quaternion;
}

CU::Vector3f ReadColor(const json& aJson)
{
	return {
		aJson["r"],
		aJson["g"],
		aJson["b"],
	};
}

UTransform ReadTransform(const json& aJson)
{
	const auto rotationQuatAsVector = ReadVector4(aJson["RotationQuat"]);
	const auto quaternion = CU::Quaternion(rotationQuatAsVector.w,
		CU::Vector3f(rotationQuatAsVector.x, rotationQuatAsVector.y, rotationQuatAsVector.z));

	const CU::Vector3f position = ReadVector3(aJson["Position"]);
	const CU::Vector3f rotationEuler = ReadVector3(aJson["RotationEuler"]);
	CU::Vector3f scale = ReadVector3(aJson["Scale"]);

	return { position, rotationEuler, quaternion, scale };
}

CU::Vector3f ComponentWiseMultiply(const CU::Vector3f& aFirst, const CU::Vector3f& aSecond)
{
	return CU::Vector3f(
		aFirst.x * aSecond.x,
		aFirst.y * aSecond.y,
		aFirst.z * aSecond.z);
}

bool LevelImporter::UnrealProject::Init(Engine::Scene& aScene, Engine::ResourceReferences& aResourceReferences)
{
	myScene = &aScene;
	myResourceReferences = &aResourceReferences;

	myOrangeFallingLeafVfxRef = GResourceManager->CreateRef<Engine::VFXResource>("Assets/VFX/FallingLeafOrange.vfx");
	myOrangeFallingLeafVfxRef->Load();

	myGreenFallingLeafVfxRef = GResourceManager->CreateRef<Engine::VFXResource>("Assets/VFX/FallingLeafGreen.vfx");
	myGreenFallingLeafVfxRef->Load();

	return true;
}

bool LevelImporter::UnrealProject::LoadWorld(const std::string aJsonPath)
{
	std::ifstream file(aJsonPath);

	if (!file.is_open())
	{
		return false;
	}

	nlohmann::json j = nlohmann::json::parse(file, nullptr, false);
	nlohmann::json sceneJson = j["Scene"];

	auto& actors = sceneJson["Actors"];

	const UTransform zeroTransform = {};

	for (auto& actor : actors)
	{
		if (actor.is_null())
		{
			continue;
		}

		CreateActorEntity(actor, zeroTransform, nullptr);
	}

	return true;
}

void LevelImporter::UnrealProject::CreateActorEntity(
	const json& aActorJson,
	const UTransform& aParentActorTransform,
	GameObject* aParent)
{
	const std::string name = aActorJson["ActorName"];
	bool isFolliage = false;

	if (aActorJson.contains("IsFolliage"))
		isFolliage = aActorJson["IsFolliage"];

	if (name == "NewBlueprint_C_7")
	{
		int test = 0;
	}


	const auto& actorComponentsJsonArray = aActorJson["ActorComponents"];

	//auto entityActor = myScene->CreateEntity(name);
	const auto gameObjectActor = myScene->AddGameObject<GameObject>();

	gameObjectActor->SetIsFromUnreal(true);

	gameObjectActor->SetName(name);

	if (isFolliage)
	{
		gameObjectActor->AddComponent<Engine::FolliageCuller>();
	}

	// auto mainCamera = AddGameObject<GameObject>();
	// mainCamera->AddComponent<Engine::CameraComponentNew>();
	// mainCamera->AddComponent<Engine::BasicMovementComponentNew>();
	// SetMainCamera(mainCamera);

	UTransform actorTransform = {};

	// Manually create the transform component for the actor
	// Why? Because each component in the actor, has a TransformComponent
	// meaning, we would overwrite the actor transform with the last component
	// transform due to only having 1 entity which can have 1 TransformComponent

	json transformComponentJson = aActorJson["Transform"];

	CreateActorComponents(
		*gameObjectActor,
		aParentActorTransform,
		actorTransform,
		transformComponentJson,
		aParent);

	for (const auto& actorComponentJson : actorComponentsJsonArray)
	{
		CreateActorComponents(
			*gameObjectActor,
			aParentActorTransform,
			actorTransform,
			actorComponentJson,
			aParent);
	}

	/*
	const auto& childrenActorsJsonArray = aActorJson["ChildrenActors"];

	for (const auto& childActorJson : childrenActorsJsonArray)
	{
		CreateActorEntity(childActorJson, actorTransform, gameObjectActor);
	}
	*/

	// Assets/Landscapes/<landscapename>/<landscapename>.landscape
	if (aActorJson.contains("temp"))
	{
		std::string landscapeName = aActorJson["ActorName"];
		LoadLandscape(landscapeName, *gameObjectActor);
	}
}

void LevelImporter::UnrealProject::CreateActorComponents(
	GameObject& aActorGameObject,
	const UTransform& aParentActorTransform,
	UTransform& aActorTransform,
	const nlohmann::json& aComponent,
	GameObject* aParent)
{
	// We do not recognize the component, it is unknown type
	if (!aComponent.contains("Type"))
	{
		return;
	}

	const std::string type = aComponent["Type"];

	if (type == "TransformComponent")
	{
		// auto& transformComponent = aActorGameObject->GetComponent<Engine::TransformComponent>();

		const auto transform = ReadTransform(aComponent);

		if (aParent)
		{
			// aActorGameObject.GetTransform().SetParent(&aParent->GetTransform());
		}

		auto& gameobjectTransform = aActorGameObject.GetTransform();
		gameobjectTransform.SetPositionLocal(transform.myPosition);

		// Thank you Björn for helping me fix the rotation issues :)
		Quatf rotation(transform.myRotationQuat);
		gameobjectTransform.SetRotationLocal(rotation);

		gameobjectTransform.SetScaleLocal(transform.myScale);

		// We changed to parent system, so that internally handles
		// the conversions to world pos/rot/scale
		// We however still need this to because other stuff in
		// this function relies on the parent transform
		aActorTransform.myPosition = gameobjectTransform.GetPosition();
		aActorTransform.myRotationQuat = gameobjectTransform.GetRotation();
		aActorTransform.myScale = gameobjectTransform.GetScale();

		/*
		if (aParent)
		{
			aActorGameObject.GetTransform().SetParent(&aParent->GetTransform());

			auto& gameobjectTransform = aActorGameObject.GetTransform();
			gameobjectTransform.SetLocalPosition(transform.myPosition);

			// Thank you Björn for helping me fix the rotation issues :)
			Quatf rotation(transform.myRotationQuat);
			gameobjectTransform.SetLocalRotation(rotation);

			gameobjectTransform.SetScale(transform.myScale);
		}
		else
		{

			const auto scaledPosition = ComponentWiseMultiply(transform.myPosition, aParentActorTransform.myScale);

			const CU::Vector4f delta = CU::Vector4f(
				scaledPosition.x,
				scaledPosition.y,
				scaledPosition.z,
				1.f);

			const auto rotatedDelta = CU::Vector3f(delta.x, delta.y, delta.z) * aParentActorTransform.myRotationQuat;

			const auto deltaVec3 = CU::Vector3f(rotatedDelta.x, rotatedDelta.y, rotatedDelta.z);

			auto& gameobjectTransform = aActorGameObject.GetTransform();

			gameobjectTransform.SetPosition(deltaVec3 + aParentActorTransform.myPosition);

			// Thank you Björn for helping me fix the rotation issues :)
			CU::Quaternion rotation(transform.myRotationQuat);

			gameobjectTransform.SetRotation(aParentActorTransform.myRotationQuat * rotation);

			gameobjectTransform.SetScale(ComponentWiseMultiply(aParentActorTransform.myScale, transform.myScale));

			aActorTransform.myPosition = gameobjectTransform.GetPosition();
			aActorTransform.myRotationQuat = gameobjectTransform.GetRotation();
			aActorTransform.myScale = gameobjectTransform.GetScale();
		}
		*/
	}
	else if (type == "StaticMeshComponent")
	{
		std::string materialName;
		bool isTransparent = false;
		bool isMovable = false;
		bool useMeshCollider = false;
		bool ShouldRender = false;
		bool castShadows = false;

		if (aComponent.contains("CastShadows"))
		{
			castShadows = aComponent["CastShadows"];
		}
		if (aComponent.contains("Movable"))
		{
			isMovable = aComponent["Movable"];
		}
		if (aComponent.contains("UseMeshCollider"))
		{
			useMeshCollider = aComponent["UseMeshCollider"];
		}
		if (aComponent.contains("ShouldRender"))
		{
			ShouldRender = aComponent["ShouldRender"];
		}

		std::string meshString = aComponent["FbxPath"];
		{
			const auto lol = meshString.find_last_of("/");
			if (lol != std::string::npos)
			{
				meshString = meshString.substr(lol + 1, meshString.size() - lol - 1);
			}

			const auto lastDot = meshString.find_last_of(".");

			// Replace after the last . with .fbx
			if (lastDot != std::string::npos)
			{
				meshString = meshString.substr(0, lastDot);
			}
		}

		std::string* modelP = GetEngine().GetGraphicsEngine().GetModelFactory().ConvertModelFileNameToPath(meshString);

		if (ShouldRender)
		{
			Engine::ModelComponent* modelComponent = aActorGameObject.AddComponent<Engine::ModelComponent>(
				meshString.c_str(),
				true);
			if (modelP)
			{
				myResourceReferences->AddModel(*modelP);


				//modelComponent->SetMaterial(mat);

				modelComponent->SetCastShadowsAllMeshes(castShadows);
			}
			else
			{
				LOG_ERROR(LogType::Engine) << "Cannot convert: " << meshString << " to path";
			}

			if (meshString == "SM_Tree_A" || meshString == "SM_Tree_C")
			{
				auto abc = aActorGameObject.AddComponent<VFXSpawner>();
				abc->SetVfx(myOrangeFallingLeafVfxRef);
			}

			if (meshString == "SM_Tree_B" || meshString == "SM_Tree_D")
			{
				auto abc = aActorGameObject.AddComponent<VFXSpawner>();
				abc->SetVfx(myGreenFallingLeafVfxRef);
			}

			// NOTE: Not needed anymore, since we bake the material into the model!
			//if (aComponent.contains("Materials"))
			//{
			//	int matCount = 0;
			//	const json materialsJsonArray = aComponent["Materials"];
			//	// TODO: Currently only supporting ONE material
			//	for (const auto& materialsJson : materialsJsonArray)
			//	{
			//		materialName = materialsJson["Name"];

			//		if (materialsJson.contains("IsTransparent"))
			//		{
			//			isTransparent = materialsJson["IsTransparent"];
			//		}

			//		// const std::string blendMode = materialsJson["BlendMode"];

			//		// TODO: Attach correct shaders here?
			//		// vertexShaderName = ;

			//		auto& defaultDeferredMat = GetEngine().GetGraphicsEngine().GetMaterialFactory().GetDefaultMeshMaterialDeferred();
			//		auto& defaultForwardMat = GetEngine().GetGraphicsEngine().GetMaterialFactory().GetDefaultMeshMaterialForward();

			//		auto mat = GetEngine().GetGraphicsEngine().GetMaterialFactory().GetMeshMaterial(materialName);

			//		// If no material was found, it will return default deferred
			//		// We want to use the default forward if its transparent in unreal
			//		if (mat == defaultDeferredMat)
			//		{
			//			if (isTransparent)
			//			{
			//				mat = defaultForwardMat;
			//			}
			//		}

			//		//if (modelComponent->GetMeshCount() - 1 == matCount)
			//		//{
			//		//	// modelComponent->SetMaterialForMesh(mat, matCount);
			//		//}
			//		//else
			//		//{
			//		//	LOG_ERROR(LogType::Jonathan) << "Missmatched material count:" << modelComponent->GetModelPath();
			//		//}

			//		//modelComponent->SetMaterial(materialName);
			//		matCount++;
			//	}
			//}

		}

		bool hasMeshCollider = false;
		if (aComponent.contains("Colliders"))
		{
			const json collidersJsonArray = aComponent["Colliders"];

			std::vector<Shape> shapesVector;

			for (const auto& colliderJson : collidersJsonArray)
			{
				Common::Vector3f center;
				if (colliderJson["Type"] != "ConvexCollider")
				{
					//Center shared by all Colliders
					center = ReadVector3(colliderJson["Center"]);
				}

				const auto scaledPosition = ComponentWiseMultiply(center, aActorTransform.myScale);

				const CU::Vector4f delta = CU::Vector4f(
					scaledPosition.x,
					scaledPosition.y,
					scaledPosition.z,
					1.f);

				const auto rotatedDelta = CU::Vector3f(delta.x, delta.y, delta.z) * aActorTransform.myRotationQuat;

				const auto deltaVec3 = CU::Vector3f(rotatedDelta.x, rotatedDelta.y, rotatedDelta.z);

				center = deltaVec3;

				Common::Quaternion Rotation;
				Shape shape;

				auto tempScale = aActorTransform.myScale;
				tempScale.x = abs(tempScale.x);
				tempScale.y = abs(tempScale.y);
				tempScale.z = abs(tempScale.z);

				if (colliderJson["Type"] == "BoxCollider")
				{
					// Needed because if any of X,Y,Z is zero shape becomes invalid
					int weirdOffset = 1;

					Common::Vector3f size(
						(colliderJson["X"] + weirdOffset) * static_cast<float>(tempScale.x),
						(colliderJson["Y"] + weirdOffset) * static_cast<float>(tempScale.y),
						(colliderJson["Z"] + weirdOffset) * static_cast<float>(tempScale.z));

					//size.x = abs(size.x);
					//size.y = abs(size.y);
					//size.z = abs(size.z);

					Rotation = ReadQuaternion(colliderJson["Rotation"]);

					shape = Shape::Box(size);
				}
				else if (colliderJson["Type"] == "SphereCollider")
				{
					float radius = colliderJson["Radius"];

					shape = Shape::Sphere(radius);
				}
				else if (colliderJson["Type"] == "SphylCollider")
				{
					//Capsule does work

					float radius = colliderJson["Radius"];
					float Length = colliderJson["Length"];

					if (tempScale.x > tempScale.z)
					{
						radius *= tempScale.x;
					}
					else
					{
						radius *= tempScale.z;
					}

					Length *= tempScale.y;

					Rotation = ReadQuaternion(colliderJson["Rotation"]);
					constexpr double PI = 355.f / 113.f;
					constexpr float Rad2Deg = 180.f / PI;
					Rotation = Rotation * CU::Quaternion(CU::Vector3f(0, 0, 0.5f * PI));

					shape = Shape::Capsule(Length * 0.5, radius);
				}
				else if (colliderJson["Type"] == "ConvexCollider")
				{
					std::vector<CU::Vector3f> vertexPoints;
					auto& VertexDataJsonArray = colliderJson["Collider"]["VertexData"];
					for (auto vertexData : VertexDataJsonArray)
					{
						CU::Vector3f vector;

						float x = vertexData["x"];
						float y = vertexData["y"];
						float z = vertexData["z"];

						vector.x = x * static_cast<float>(tempScale.x);
						vector.y = y * static_cast<float>(tempScale.y);
						vector.z = z * static_cast<float>(tempScale.z);

						vertexPoints.push_back(vector);
					}
#ifdef _DEBUG
					//CookingManager::Instance().CreateConvex(meshString, vertexPoints);
#endif
					shape = Shape::Convex(vertexPoints);
					if (!shape.IsValid())
					{
						LOG_ERROR(LogType::Physics) << "Cannot create Convex for: " << meshString;
					}
				}


				assert(shape.IsValid());

				Common::Quaternion fullRot;
				fullRot = aActorTransform.myRotationQuat * Rotation;
				shape.SetLocalPosition(center);
				shape.SetLocalQuaternion(fullRot);

				shapesVector.push_back(shape);
			}


			if (useMeshCollider)
			{
				if (modelP != nullptr)
				{
					auto* triangeStatic = aActorGameObject.AddComponent<InfoRigidStaticComponent>();
					aActorGameObject.SetLayers(eLayer::DEFAULT, eLayer::DEFAULT, eLayer::DEFAULT);
					auto meshPointsData = PhysicsUtilities::GetMeshes(*modelP);
					auto tempScale = aActorTransform.myScale;
					for (auto& pointsData : meshPointsData)
					{
						Shape shape;

						for (auto && vertex : pointsData.vertices)
						{
							vertex.x *= static_cast<float>(tempScale.x);
							vertex.y *= static_cast<float>(tempScale.y);
							vertex.z *= static_cast<float>(tempScale.z);
						}

						shape = Shape::TriangleMesh(pointsData.vertices, pointsData.indices);

						if (shape.IsValid())
						{
							triangeStatic->Attach(shape);
						}
						else
						{
							LOG_ERROR(LogType::Physics) << "Cannot create Triangle mesh for: " << meshString;
						}
					}
				}
			}
			else
			{
				if (isMovable)
				{
					auto* rb = aActorGameObject.AddComponent<InfoRigidBodyComponent>();
					//rb->SetTransform(fullRot, center + aActorTransform.myPosition);

					float mass = 1;
					if (aComponent.contains("Mass"))
					{
						mass = aComponent["Mass"];
					}
					rb->SetMass(mass);

					for (const auto& shape : shapesVector)
					{
						rb->Attach(shape);
					}
					aActorGameObject.SetLayers(eLayer::MOVABLE, eLayer::DEFAULT | eLayer::ENVIRONMENT | eLayer::MOVABLE | eLayer::PLAYER | eLayer::ENEMY, eLayer::NONE);

					// NOTE(filip): removed because caused issue because KnockbackComponent is in Game project
					// auto knockback = aActorGameObject.AddComponent<KnockbackComponent>();
				}
				else
				{
					auto* multiStatic = aActorGameObject.AddComponent<InfoRigidStaticComponent>();
					for (const auto& shape : shapesVector)
					{
						multiStatic->Attach(shape);
					}

					////For some reason these lines are needed for static colliders to exsist in Release
					Common::Quaternion cringeQuaternion;
					//multiStatic->SetTransform(cringeQuaternion, aActorTransform.myPosition);
					//multiStatic->SetTransform(cringeQuaternion, aActorTransform.myPosition);

					aActorGameObject.SetLayers(eLayer::DEFAULT, eLayer::DEFAULT, eLayer::NONE);
				}
			}
		}
	}


	// auto materialComponent = aActorGameObject.AddComponent<Engine::MaterialComponentNew>(*modelComponent);
	// materialComponent->SetName(materialName);

	// auto& realComp = aActorEntity.AddComponent<Engine::RealComponent>();
	// realComp.myComponent = new Engine::RotateMeComponent();

	else if (type == "DirectionalLightComponent")
	{
		Engine::EnvironmentLight* environmentLight = nullptr;

		float intensity = aComponent["Intensity"];
		CU::Vector3f color = ReadColor(aComponent["Color"]);

		//environmentLight->SetColor(color);
		//environmentLight->SetColorIntensity(intensity);

		// IT IS NOW DONE FROM A JSON INSTEAD
		// myScene->GetEnvironmentLight().SetColor(color);
		// myScene->GetEnvironmentLight().SetColorIntensity(intensity);
	}
	else if (type == "PointLightComponent")
	{
		Engine::PointLightComponent* pointLightComponent = nullptr;

		float intensity = aComponent["Intensity"];
		float range = aComponent["Range"];
		CU::Vector3f color = ReadColor(aComponent["Color"]);

		bool castShadows = false;
		if (aComponent.contains("CastShadow"))
		{
			castShadows = aComponent["CastShadow"];
		}

		pointLightComponent = aActorGameObject.AddComponent<Engine::PointLightComponent>(castShadows);
		pointLightComponent->SetIntensity(intensity * 16000.f);
		pointLightComponent->SetRange(range + 0.1);
		pointLightComponent->SetColor(color);
		//pointLightComponent->SetActive(true);
	}
	else if (type == "SpotLightComponent")
	{
		Engine::SpotLightComponent* spotLightComponent = nullptr;

		float intensity = aComponent["Intensity"];
		float range = aComponent["Range"];
		float innerRadius = aComponent["InnerRadius"];
		float outerRadius = aComponent["OuterRadius"];
		CU::Vector3f color = ReadColor(aComponent["Color"]);

		bool castShadows = false;
		if (aComponent.contains("CastShadow"))
		{
			castShadows = aComponent["CastShadow"];
		}

		spotLightComponent = aActorGameObject.AddComponent<Engine::SpotLightComponent>(castShadows);
		spotLightComponent->SetIntensity(intensity * 16000.f);
		spotLightComponent->SetRange(range + 0.01);
		spotLightComponent->SetColor(color);
		spotLightComponent->SetInnerAngle(innerRadius + 0.1f);
		spotLightComponent->SetOuterAngle(outerRadius);
		////pointLightComponent->SetActive(true);
	}
	else if (type == "EnemySpawnerComponent")
	{
		int enemyType = aComponent["EnemyType"];
		int EnemyClass = aComponent["Class"];
		auto position = aActorTransform.myPosition;
		auto& rotation = aActorTransform.myRotationQuat;

		//myScene->CreateEnemyBuffer(enemyType, EnemyClass, position, rotation);
	}
	else if (type == "PlayerSpawnComponent")
	{
		auto position = aActorTransform.myPosition;
		auto& rotation = aActorTransform.myRotationQuat;

		//myScene->CreatePlayerBuffer(position, rotation);
	}
	else if (type == "ScriptedEventComponent")
	{
		//myScene->AddScriptedEventObjects(&aActorGameObject);
	}
	else if (type == "SoundComponent")
	{
		std::string soundName = aComponent["SoundName"];
		auto position = aActorTransform.myPosition;

		AudioManager::GetInstance()->PlayEvent3D(soundName, 0, position, { 1, 1, 1 }, { 0, 1, 0 });

		//myScene->CreateSoundBuffer(soundName, position);
	}
	else if (type == "MovableComponent")
	{
		bool movable = aComponent["Movable"];
	}

	// NOTE: We do not support children components and its hierarchy because of ENTT limitations
#if 0
		// Handle the children components
	if (aComponent.contains("ChildrenComponents"))
	{
		const auto& childrenComponentsJsonArray = aComponent["ChildrenComponents"];

		for (const auto& childComponentJson : childrenComponentsJsonArray)
		{
			CreateActorComponents(
				aActorEntity,
				aParentActorTransform,
				aActorTransform,
				childComponentJson);
		}
#endif
}

	void LevelImporter::UnrealProject::LoadLandscape(std::string aLandscapeName, GameObject & aGameObjectActor)
	{
		std::string landscapeName = aLandscapeName;

		std::string path = "Assets/Landscapes/" + landscapeName + "/" + landscapeName + ".landscape";

		Engine::LandscapeComponent* landscapeComponent = aGameObjectActor.AddComponent<Engine::LandscapeComponent>();
		if (std::filesystem::exists(path))
		{
			landscapeComponent->SetLandscape(path);
		}

		aGameObjectActor.GetTransform().SetPosition({ 0,0,0 });
		aGameObjectActor.GetTransform().SetScale({ 1,1,1 });
	}
