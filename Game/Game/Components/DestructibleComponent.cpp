#include "pch.h"
#include "DestructibleComponent.h"

#include "FractureComponent.h"
#include "PhysicsUtilities.h"
#include "Common/BinaryReader.h"

#include "Engine/GameObject/GameObject.h"
#include "Engine/Shortcuts.h"
#include "Engine/GameObject/Components/MeshComponent.h"
#include "Engine/GameObject/Components/ModelComponent.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Reflection/Reflector.h"
#include "Engine/Renderer/Model/Model.h"
#include "Engine/Renderer/ModelLoader/Binary/model_generated.h"
#include "Physics/CookingManager.h"

DestructibleComponent::DestructibleComponent(GameObject* aGameObject)
	: Component(aGameObject)
{
}

void DestructibleComponent::Start()
{
	myGameObject->SetLayer(myGameObject->GetLayer() | eLayer::DESTRUCTABLE);
	myGameObject->SetPhysicalLayer(eLayer::DEFAULT | eLayer::ALL);

	myResponse = static_cast<Destructible::eDamageType>(myResponseTypeID);

	myRb = myGameObject->AddComponent<Engine::RigidBodyComponent>(myIsKinematic ? eRigidBodyType::KINEMATIC : eRigidBodyType::DEFAULT);

	const auto children = myGameObject->GetTransform().GetChildren()[0]->GetChildren();

	auto meshes = GetMeshes();

	//for (int i = 0; i < meshes.size(); i++)
	//{
	//	std::string path = "EN_P_Staty_01_Shattered" + std::to_string(i);
	//	//std::string path = "EN_P_CubicleDesk_01_Shattered" + std::to_string(i);
	//	//path.erase(0, 14);
	//	//path.erase(path.size() - 6, 6);
	//	LOG_INFO(LogType::Viktor) << "Meshes " << i;
	//	if (!CookingManager::Instance().CreateTriangleMesh(path, meshes[i].vertices, meshes[i].indices))
	//	{
	//		assert(false && "Failed to create this");
	//	}
	//}

	for (int i = 0; i < children.size(); i++)
	{
		auto child = children[i];
		auto* obj = child->GetGameObject();
		auto* mesh = obj->GetComponent<Engine::MeshComponent>();
		if (mesh)
		{
#ifdef  _DEBUG
			CookingManager::Instance().CreateConvex(myModelPath + std::to_string(i), meshes[i].vertices);
#endif
			auto shape = Shape::Convex(myModelPath + std::to_string(i));//(meshes[i].vertices);

			shape.SetLocalPosition(obj->GetTransform().GetPositionLocal());
			shape.SetData(DBG_NEW int(i));

			Attach(shape, 10.f, obj, i);
		}
		else
		{
			assert(false && "Deprecated");
			while (child->GetChildren().size() > 0) 
			{
				child = child->GetChildren()[0];
			}
			obj = child->GetGameObject();

			auto* cmesh = obj->GetComponent<Engine::MeshComponent>();
			if (cmesh)
			{
				// ADD SHAPE FROM MODEL
				//auto shape = Shape::Sphere(50);
				//shape.SetLocalPosition(obj->GetTransform().GetPositionLocal());

				std::string path = myModelPath;
				std::replace(path.begin(), path.end(), '/', '.');
				path.erase(path.size() - 6, 6);

				path += std::to_string(i);
				CookingManager::Instance().CreateTriangleMesh(path, meshes[i].vertices, meshes[i].indices);

				auto shape = Shape::TriangleMesh(path);//(meshes[i].vertices, meshes[i].indices);
				shape.SetLocalPosition(obj->GetTransform().GetPositionLocal());
				shape.SetData(DBG_NEW int(i));

				Attach(shape, 30.f, obj, i);
			}
		}
	}

	myRb->SetMass(myMass);
}

void DestructibleComponent::Execute(Engine::eEngineOrder aOrder)
{

}
void DestructibleComponent::Attach(Shape aShape, float aMass, const std::string& aModelPath)
{
	const auto name = std::to_string(myShapes.size());
	if (!aModelPath.empty())
	{
		auto* obj = myGameObject->GetScene()->AddGameObject<GameObject>();

		auto* model = obj->AddComponent<Engine::ModelComponent>(aModelPath);
		obj->SetName(name);

		obj->GetTransform().SetParent(&myGameObject->GetTransform());
		obj->GetTransform().SetPositionLocal(aShape.GetLocalPosition());
	}
	aShape.SetData(DBG_NEW int(myShapes.size()));

	DestructibleShape shape { aShape, aMass };

	myShapes.emplace_back(shape);
	myMass += aMass;
}

void DestructibleComponent::SetKinematic(bool isKinematic)
{
	myIsKinematic = isKinematic;
	if (myRb)
	{
		myRb->SetType(myIsKinematic ? eRigidBodyType::KINEMATIC : eRigidBodyType::DEFAULT);
	}
}

void DestructibleComponent::SetDamageResponse(Destructible::eDamageType aResponse)
{
	myResponse = aResponse;
}

void DestructibleComponent::ApplyDamage(Shape aShape, const Vec3f& aForce, const Vec3f& aLocalPosition)
{
	const auto sPos = aShape.GetLocalPosition() * myGameObject->GetTransform().GetRotation();
	auto localPosition = sPos - aLocalPosition;

	Engine::RigidBodyComponent* rb = nullptr;

	int data = -1;
	if (aShape.GetData())
	{
		data = *static_cast<int*>(aShape.GetData());
	}

	for (auto it = myShapes.begin(); it != myShapes.end(); it++)
	{
		auto& shape = *it;

		int shapeData = -1;
		if (void* d = shape.shape.GetData())
		{
			shapeData = *static_cast<int*>(d);
		}
		if (shapeData == data)
		{
			rb = DetachShape(shape);
			assert(rb && "This shouldnt happen :( and means that something went wrong, talk with Viktor");
			myShapes.erase(it);
			break;
		}
	}

	rb->AddForceAtLocalPosition(aForce, localPosition, eForceMode::IMPULSE);

	ApplyResponse();
}

void DestructibleComponent::ApplyExplosion(const Vec3f& aPosition, float aForce, float aRange, const Vec3f& aDir,
                                           float aDirPercentage)
{
	for (auto it = myShapes.begin(); it != myShapes.end();)
	{
		auto& shape = *it;
		const auto pos = myGameObject->GetTransform().GetPosition() + shape.shape.GetLocalPosition() * myGameObject->GetTransform().GetRotation();

		auto dir = pos - aPosition;
		const auto dist = dir.Length();
		if (dist > aRange)
		{
			++it;
			continue;
		}

		dir.Normalize();
		dir = aDir * aDirPercentage + dir * (1.f - aDirPercentage);

		auto force = dir * aForce * ((aRange - dist) / aRange);

		auto* rb = DetachShape(shape); // Your big boii back xoxo <3333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333

		rb->AddForce(force, eForceMode::IMPULSE);

		it = myShapes.erase(it);

		//GDebugDrawer->DrawLine3D(DebugDrawFlags::Physics, pos, pos + dir * 100.f, 10.f, { 1,0,0, 1 });
		//GDebugDrawer->DrawLine3D(DebugDrawFlags::Physics, pos, aPosition, 10.f, { 0,0,1, 1});
	}
	ApplyResponse();
}

std::vector<MeshPointsData> DestructibleComponent::GetMeshes()
{
	return PhysicsUtilities::GetMeshes(myModelPath);

	//std::string aModelPath = myModelPath;
	//std::vector<MeshPointsData> meshes;

	//BinaryReader reader;

	//if (!reader.InitWithFile(aModelPath))
	//{
	//	assert(false && "BNruh");
	//}

	//reader.UncompressGzip();

	//const auto mdl = MyGame::Sample::GetModel(reader.GetStart());

	//std::vector<Renderer::MeshData> resultMeshDatas;

	//for (int meshIndex = 0; meshIndex < mdl->meshes()->size(); ++meshIndex)
	//{
	//	MeshPointsData meshPointData;

	//	const auto& loaderMesh = mdl->meshes()->Get(meshIndex);

	//	Renderer::MeshData meshData = { };
	//	{
	//		meshData.myVertexCount = loaderMesh->vertices_count();
	//		meshData.myIndicesCount = loaderMesh->indices_count();

	//		meshData.myBaseVertexLocation = loaderMesh->vertices_start_index();
	//		meshData.myStartIndexLocation = loaderMesh->indices_start_index();

	//		//if (loaderMesh->local_position())
	//		//	meshData.myTransform.SetPositionLocal(
	//		//		*reinterpret_cast<const Vec3f*>(loaderMesh->local_position()));

	//		//if (loaderMesh->local_rotation())
	//		//{
	//		//	meshData.myTransform.SetRotationLocal(
	//		//		Quatf(loaderMesh->local_rotation()->w(),
	//		//			Vec3f(
	//		//				loaderMesh->local_rotation()->x(),
	//		//				loaderMesh->local_rotation()->y(),
	//		//				loaderMesh->local_rotation()->z())));
	//		//}

	//		//if (loaderMesh->local_scale())
	//		//{
	//		//	meshData.myTransform.SetScaleLocal(
	//		//		*reinterpret_cast<const Vec3f*>(loaderMesh->local_scale()));
	//		//}
	//	}

	//	resultMeshDatas.push_back(meshData);
	//	meshes.emplace_back(meshPointData);
	//}
	//const MyGame::Sample::DefaultVertex* vertices = mdl->vertices()->Get(0);
	//const unsigned int* indices = mdl->indices()->data();
	//const int verticesSize = mdl->vertex_count();
	//const int indicesSize = mdl->indices_count();

	//for (int i = 0; i < resultMeshDatas.size(); i++)
	//{
	//	const auto& md = resultMeshDatas[i];
	//	auto& mesh = meshes[i];

	//	const auto& node = mdl->nodes()->Get(i);
	//	const auto& nodeData = node->data();

	//	mesh.localPosition = Vec3f(
	//		nodeData->position().x(),
	//		nodeData->position().y(),
	//		nodeData->position().z());

	//	mesh.localRotation = (Quatf(
	//		nodeData->rotation().w(),
	//		Vec3f(nodeData->rotation().x(),
	//			nodeData->rotation().y(),
	//			nodeData->rotation().z())));

	//	mesh.indices.resize(md.myIndicesCount);
	//	int index = 0;
	//	for (int j = md.myStartIndexLocation; j < md.myStartIndexLocation + md.myIndicesCount; j++)
	//	{
	//		mesh.indices[index] = indices[j];
	//		++index;
	//	}

	//	mesh.vertices.resize(md.myVertexCount);
	//	index = 0;
	//	for (int j = md.myBaseVertexLocation; j < md.myBaseVertexLocation + md.myVertexCount; j++)
	//	{
	//		Vec3f pos = { vertices[j].position().x(), vertices[j].position().y(), vertices[j].position().z() };
	//		mesh.vertices[index] = pos;
	//		++index;
	//	}
	//}

	//return meshes;
}

void DestructibleComponent::Reflect(Engine::Reflector& aReflector)
{
	Component::Reflect(aReflector);
	aReflector.Reflect(myModelPath, "Model Path");
	aReflector.Reflect(myMass, "Total Mass");
	aReflector.Reflect(myResponseTypeID, "Response Mode");
}

void DestructibleComponent::Attach(Shape aShape, float aMass, GameObject* aGameObject, int aIndex)
{
	const auto name = std::to_string(aIndex);
	aShape.SetData(DBG_NEW int(aIndex));

	DestructibleShape shape{ aShape, aMass, aIndex };

	myShapes.emplace_back(shape);
	myRb->Attach(aShape);
	myMass += aMass;
}

Engine::RigidBodyComponent* DestructibleComponent::DetachShape(DestructibleShape& aShape)
{
	int data = -1;
	if (aShape.shape.GetData())
	{
		data = *static_cast<int*>(aShape.shape.GetData());
	};

	GameObject* obj = nullptr;

	const auto& children =	myGameObject->GetTransform().GetChildren()[0]->GetChildren();

	auto& child = children[aShape.childIndex];
	obj = child->GetGameObject();
	obj->GetTransform().RemoveParent();

	//for (const auto& child : children)
	//{
	//	if (child->GetGameObject()->GetName() == name)
	//	{	
	//		obj = child->GetGameObject();
	//		obj->GetTransform().RemoveParent();
	//		break;
	//	}
	//}

	for (auto& destructibleShape : myShapes)
	{
		if (destructibleShape.childIndex >= aShape.childIndex)
		{
			destructibleShape.childIndex -= 1;
		}
	}

	myRb->Detach(aShape.shape);
	myMass -= aShape.mass;

	if (!obj)
		obj = myGameObject->GetScene()->AddGameObject<GameObject>();
	obj->SetLayers(eLayer::FRACTURE, myGameObject->GetPhysicalLayerMask());

	auto pos = myGameObject->GetTransform().GetPosition() + aShape.shape.GetLocalPosition() * myGameObject->GetTransform().GetRotation();
	obj->GetTransform().SetPosition(pos);
	obj->GetTransform().SetRotation(myGameObject->GetTransform().GetRotation());

	aShape.shape.SetLocalPosition({ 0,0,0 });

	obj->AddComponent<FractureComponent>(10.f, 0.8f);
	auto rb = obj->AddComponent<Engine::RigidBodyComponent>(eRigidBodyType::DEFAULT);
	rb->Attach(aShape.shape);
	return rb;
}

void DestructibleComponent::ApplyResponse()
{
	switch (myResponse)
	{
	case Destructible::eDamageType::OnlyAffectHits:
	{
		return;
	}
		break;
	case Destructible::eDamageType::ShatterOnDamage: 
	{
		for (auto it = myShapes.begin(); it != myShapes.end();)
		{
			auto& shape = *it;
			DetachShape(shape);
			it = myShapes.erase(it);
		}
	}
		break;
	case Destructible::eDamageType::RigidOnDamage:
	{
		myRb->SetType(eRigidBodyType::DEFAULT);
	}
		break;
	default: ;
	}

	if (myShapes.size() > 0)
	{
		myRb->SetMass(myMass);
		return;
	}

	// If we're empty destroy our gameobject since we wont need it anymore
	myGameObject->Destroy();
}