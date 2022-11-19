#include "pch.h"
#include "RenderCommandList.h"
#include "Engine\Renderer\Scene\CulledSceneData.h"
#include "Engine\Renderer\Model\MeshInstance.h"
#include "Engine\Renderer\Model\ModelInstance.h"
#include "Engine\ResourceManagement\Resources\ModelResource.h"
#include "Engine\Renderer\Model\Model.h"
#include "Engine\GameObject\Components\MeshComponent.h"

void Engine::RenderCommandList::Reset()
{
	myMeshCommands.clear();
	mySpotlightCommands.clear();
	myPointLightCommands.clear();
}

Engine::SingleMeshData2::SingleMeshData2(const CulledMeshInstance& aCulledMeshInstance)
{
	Model& mdl = aCulledMeshInstance.myMeshInstance->GetModelInstance()->GetModel()->Get();

	myMaterialInstance = &aCulledMeshInstance.myMeshInstance->GetMaterialInstance();

	myCastShadows = aCulledMeshInstance.myMeshInstance->IsCastingShadows();
	myMaterial = aCulledMeshInstance.myMeshInstance->GetMaterialInstance().GetMaterial();
	myTransform = aCulledMeshInstance.myMeshInstance->GetModelInstance()->GetTransformMatrix();
	myMeshIndex = aCulledMeshInstance.myMeshInstance->GetMeshIndex();
	//myModel = aCulledMeshInstance.myMeshInstance->GetModelInstance()->GetModel();
	myModel = &mdl;
	myPass = aCulledMeshInstance.myEffectPass;
	myHasSkeleton = mdl.IsSkinned();
	if (myHasSkeleton)
	{
		myAnimTransforms = &aCulledMeshInstance.myMeshInstance->GetModelInstance()->GetFinalAnimTransforms();
	}
}

Engine::SingleMeshData2::SingleMeshData2(const CulledMeshComponent& aCulledMeshComponent)
{
	Model& mdl = aCulledMeshComponent.myMeshComponent->GetModel()->Get();

	myMaterialInstance = &aCulledMeshComponent.myMeshComponent->GetMaterialInstance();

	myCastShadows = aCulledMeshComponent.myMeshComponent->IsCastingShadows();
	myMaterial = aCulledMeshComponent.myMeshComponent->GetMaterialInstance().GetMaterial();
	myTransform = aCulledMeshComponent.myMeshComponent->GetTransform().GetMatrix();
	myMeshIndex = aCulledMeshComponent.myMeshComponent->GetMeshIndex();
	//myModel = aCulledMeshInstance.myMeshInstance->GetModelInstance()->GetModel();
	myModel = &mdl;
	myPass = aCulledMeshComponent.myEffectPass;
	myHasSkeleton = mdl.IsSkinned();
	if (myHasSkeleton)
	{
		myAnimTransforms = nullptr;
		// TODO(filip): mesh components does not have animations atm
		// myAnimTransforms = &aCulledMeshComponent.myMeshInstance->GetModelInstance()->GetFinalAnimTransforms();
	}
}
