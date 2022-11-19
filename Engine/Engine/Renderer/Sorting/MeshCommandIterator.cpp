#include "pch.h"
#include "MeshCommandIterator.h"
#include "Engine/ResourceManagement/Resources/ModelResource.h"

Engine::MeshCommandIterator::MeshCommandIterator(
	const RenderCommandList& aCommandList)
	: myMeshCommands(aCommandList.myMeshCommands),
	mySize(aCommandList.myMeshCommands.size())
{
	myValue = &myMeshCommands[0];

	Update();
}

bool Engine::MeshCommandIterator::NotFinished() const
{
	return myIndex < mySize;
}

void Engine::MeshCommandIterator::Next()
{
	++myIndex;

	if (myIndex < mySize)
	{
		myValue = &myMeshCommands[myIndex];
		Update();
	}
}

bool Engine::MeshCommandIterator::EffectPassChanged() const
{
	return myEffectPassChanged;
}

bool Engine::MeshCommandIterator::ModelChanged() const
{
	return myModelChanged;
}

bool Engine::MeshCommandIterator::MeshIndexChanged() const
{
	return myMeshIndexChanged;
}

void Engine::MeshCommandIterator::Update()
{
	//myEffectPassChanged = myBoundPassID != myValue->myMeshData.myPass->GetID();
	//myModelChanged = myBoundModelID != myValue->myMeshData.myModel->GetID();
	//myMeshIndexChanged = myMeshIndex != myValue->myMeshData.myMeshIndex;

	//if (myEffectPassChanged)
	//{
	//	myBoundPassID = myValue->myMeshData.myPass->GetID();
	//}

	//if (myModelChanged)
	//{
	//	myBoundModelID = myValue->myMeshData.myModel->GetID();

	//	// if the model changed, means the mesh index 
	//	// theoretically changed as well
	//	myMeshIndexChanged = true;
	//}

	//if (myMeshIndexChanged)
	//{
	//	myMeshIndex = myValue->myMeshData.myMeshIndex;
	//}
}
