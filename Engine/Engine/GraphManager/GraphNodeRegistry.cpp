#include "pch.h"
#include "GraphNodeRegistry.h"

#include "Nodes/Event/GraphNode_EventCall.h"
#include "Nodes/Event/GraphNode_OnEvent.h"
#include "Nodes/Math/GraphNode_MathAdd.h"
#include "Nodes/Misc/GraphNode_Print.h"
#include "Nodes/Misc/GraphNode_Update.h"
#include "Nodes/Misc/GraphNode_Timer.h"
#include "Nodes/Math/GraphNode_MathMul.h"
#include "Nodes/Math/GraphNode_MathSub.h"
#include "Nodes/Math/GraphNode_MathCos.h"
#include "Nodes/Math/GraphNode_MathAbs.h"
#include "Nodes/Math/GraphNode_MathAtan2.h"
#include "Nodes/Math/GraphNode_MathLength.h"
#include "Nodes/Math/GraphNode_MathMax.h"
#include "Nodes/Math/GraphNode_MathMin.h"
#include "Nodes/Flow/GraphNode_Branch.h"
#include "Nodes/Flow/GraphNode_ForLoop.h"
#include "Nodes/Math/GraphNode_MathSin.h"
#include "Nodes/Misc/GraphNode_StopTimer.h"
#include "Nodes/Misc/GraphNode_OnBeginPlay.h"
#include "Nodes/Misc/GraphNode_Variable.h"
#include <Engine/GameObject/Components/Component.h>
#include "Engine/GameObject/ComponentSystem/ComponentFactory.h"
#include <Engine/GraphManager/Nodes/Misc/GraphNode_ComponentGet.h>
#include <Engine/GraphManager/Nodes/Misc/GraphNode_ComponentSet.h>
#include <Engine/GraphManager/Nodes/Misc/GraphNode_EqualsInt.h>
#include <Engine/GraphManager/Nodes/Misc/GraphNode_EqualsFloat.h>
#include <Engine/GraphManager/Nodes/Misc/GraphNode_EqualsString.h>
#include <Engine/GraphManager/Nodes/Misc/GraphNode_EqualsGameObject.h>
#include <Engine/GraphManager/Nodes/Misc/GraphNode_GetGameObjectTransform.h>
#include <Engine/GraphManager/Nodes/Misc/GraphNode_SetGameObjectTransform.h>
#include <Engine/GraphManager/Nodes/Event/GraphNode_OnButtonClicked.h>
#include <Engine/GraphManager/Nodes/Math/GraphNode_Clamp.h>
#include <Engine/GraphManager/Nodes/Misc/GraphNode_GetGameObjectID.h>
#include <Engine/GraphManager/Nodes/Event/GraphNode_OnButtonHoverExit.h>
#include <Engine/GraphManager/Nodes/Event/GraphNode_OnButtonHoverEnter.h>
#include <Engine/GraphManager/Nodes/Misc/GraphNode_GetGameObjectChild.h>
#include <Engine/GraphManager/Nodes/Misc/GraphNode_GetGameObjectParent.h>
#include <Engine/GraphManager/Nodes/Misc/GraphNode_Start.h>
#include <Engine/GraphManager/Nodes/Misc/GraphNode_GetGameObjectPosition.h>
#include <Engine/GraphManager/Nodes/Misc/GraphNode_GetGameObjectLocalPosition.h>
#include <Engine/GraphManager/Nodes/Misc/GraphNode_GetGameObjectRotation.h>
#include <Engine/GraphManager/Nodes/Misc/GraphNode_GetGameObjectScale.h>
#include <Engine/GraphManager/Nodes/Misc/GraphNode_SetGameObjectPosition.h>
#include <Engine/GraphManager/Nodes/Misc/GraphNode_SetGameObjectLocalPosition.h>
#include <Engine/GraphManager/Nodes/Misc/GraphNode_SetGameObjectRotation.h>
#include <Engine/GraphManager/Nodes/Misc/GraphNode_SetGameObjectScale.h>
#include <Engine/GraphManager/Nodes/Misc/GraphNode_LoadScene.h>
#include <Engine/GraphManager/Nodes/Misc/GraphNode_AnimationTrigger.h>
#include <Engine/GraphManager/Nodes/Misc/GraphNode_SetCondition.h>
#include <Engine/GraphManager/Nodes/Misc/GraphNode_AddAudioEvent.h>
#include <Engine/GraphManager/Nodes/Misc/GraphNode_PlayAudioEvent.h>
#include <Engine/GraphManager/Nodes/Event/GraphNode_OnTriggerEvent.h>
#include <Engine/GraphManager/Nodes/Misc/GraphNode_SetResolution.h>
#include <Engine/GraphManager/Nodes/Misc/GraphNode_SetWindowedMode.h>
#include <Engine/GraphManager/Nodes/Misc/GraphNode_QuitProgram.h>
#include <Engine/GraphManager/Nodes/Misc/GraphNode_FadeAllChildren.h>
#include <Engine/GraphManager/Nodes/Misc/GraphNode_SetParameter.h>
#include <Engine/GraphManager/Nodes/Misc/GraphNode_FindGameObjectID.h>
#include <Engine/GraphManager/Nodes/Misc/GraphNode_GetMaterialValue.h>
#include <Engine/GraphManager/Nodes/Misc/GraphNode_SetMaterialValue.h>

void GraphNodeRegistry::PopulateTypes()
{
	RegisterType<GraphNode_Update>("GraphNode_Start");
	RegisterType<GraphNode_Start>("GraphNode_Init");
	RegisterType<GraphNode_Print>("GraphNode_Print");

	RegisterType<GraphNode_MathAdd>("GraphNode_MathAdd");
	RegisterType<GraphNode_MathMul>("GraphNode_MathMul");
	RegisterType<GraphNode_MathSub>("GraphNode_MathSub");
	RegisterType<GraphNode_MathCos>("GraphNode_MathCos");
	RegisterType<GraphNode_MathSin>("GraphNode_MathSin");
	RegisterType<GraphNode_MathAbs>("GraphNode_MathAbs");
	RegisterType<GraphNode_MathAtan2>("GraphNode_MathAtan2");
	RegisterType<GraphNode_MathLength>("GraphNode_MathLength");
	RegisterType<GraphNode_MathMax>("GraphNode_MathMax");
	RegisterType<GraphNode_MathMin>("GraphNode_MathMin");
	RegisterType<GraphNode_MathClamp>("GraphNode_MathClamp");

	RegisterType<GraphNode_Timer>("GraphNode_Timer");
	RegisterType<GraphNode_Branch>("GraphNode_Branch");
	RegisterType<GraphNode_StopTimer>("GraphNode_StopTimer");
	RegisterType<GraphNode_OnBeginPlay>("GraphNode_OnBeginPlay");
	RegisterType<GraphNode_ForLoop>("GraphNode_ForLoop");
	RegisterType<GraphNode_Get>("GraphNode_Get");
	RegisterType<GraphNode_Set>("GraphNode_Set");

	RegisterType<GraphNode_FadeAllChildren>("GraphNode_FadeAllChildren");
	RegisterType<GraphNode_QuitProgram>("GraphNode_QuitProgram");
	RegisterType<GraphNode_SetResolution>("GraphNode_SetResolution");
	RegisterType<GraphNode_SetWindowedMode>("GraphNode_SetWindowedMode");

	RegisterType<GraphNode_OnButtonClicked>("GraphNode_OnButtonClicked");
	RegisterType<GraphNode_OnButtonHoverEnter>("GraphNode_OnButtonHoverEnter");
	RegisterType<GraphNode_OnButtonHoverExit>("GraphNode_OnButtonHoverExit");
	RegisterType<GraphNode_OnTriggerEvent>("GraphNode_OnTriggerEvent");

	RegisterType<GraphNode_AnimationTrigger>("GraphNode_AnimationTrigger");
	RegisterType<GraphNode_SetCondition>("GraphNode_SetCondition");

	RegisterType<GraphNode_AddAudioEvent>("GraphNode_AddAudioEvent");
	RegisterType<GraphNode_PlayAudioEvent>("GraphNode_PlayAudioEvent");
	RegisterType<GraphNode_SetParameter>("GraphNode_SetAudioParameter");

	RegisterType<GraphNode_EqualsInt>("GraphNode_EqualsInt");
	RegisterType<GraphNode_EqualsFloat>("GraphNode_EqualsFloat");
	RegisterType<GraphNode_EqualsString>("GraphNode_EqualsString");
	RegisterType<GraphNode_EqualsGameObject>("GraphNode_EqualsGameObject");

	RegisterType<GraphNode_GetMaterialValue>("GraphNode_GetMaterialValue");
	RegisterType<GraphNode_SetMaterialValue>("GraphNode_SetMaterialValue");

	RegisterType<GraphNode_FindGameObjectID>("GraphNode_FindGameObjectID");
	RegisterType<GraphNode_GetGameObjectID>("GraphNode_GetGameObjectID");
	RegisterType<GraphNode_GetGameObjectChild>("GraphNode_GetGameObjectChild");
	RegisterType<GraphNode_GetGameObjectParent>("GraphNode_GetGameObjectParent");
	RegisterType<GraphNode_GetGameObjectTransform>("GraphNode_GetGameObjectTransform");
	RegisterType<GraphNode_GetGameObjectPosition>("GraphNode_GetGameObjectPosition");
	RegisterType<GraphNode_GetGameObjectLocalPosition>("GraphNode_GetGameObjectLocalPosition");
	RegisterType<GraphNode_GetGameObjectRotation>("GraphNode_GetGameObjectRotation");
	RegisterType<GraphNode_GetGameObjectScale>("GraphNode_GetGameObjectScale");
	RegisterType<GraphNode_SetGameObjectPosition>("GraphNode_SetGameObjectPosition");
	RegisterType<GraphNode_SetGameObjectLocalPosition>("GraphNode_SetGameObjectLocalPosition");
	RegisterType<GraphNode_SetGameObjectRotation>("GraphNode_SetGameObjectRotation");
	RegisterType<GraphNode_SetGameObjectScale>("GraphNode_SetGameObjectScale");

	RegisterType<GraphNode_SetGameObjectTransform>("GraphNode_SetGameObjectTransform");

	RegisterType<GraphNode_LoadScene>("GraphNode_LoadScene");


	RegisterComponentNodes();
	{
		std::vector<DynamicPin> pins = {
			{"intvalue", DataType::Int},
		};
		RegisterEvent("Dick", pins);
	}
	{
		std::vector<DynamicPin> pins = {
			{"Opened", DataType::Bool},
		};
		RegisterEvent("InteractDoorKnob", pins);
	}
	{
		std::vector<DynamicPin> pins = {
			{"KeyCardLevel", DataType::Int},
		};
		RegisterEvent("KeyCardPickup", pins);
	}
}
void GraphNodeRegistry::RegisterComponentNodes()
{
	for (int i = 0; i < Engine::ComponentFactory::GetInstance().GetRegisteredComponentsCount(); i++)
	{
		auto regComp = Engine::ComponentFactory::GetInstance().GetRegisteredComponents()[i];
		if (!regComp)
		{
			continue;
		}
		Component* component = regComp->myComponent;

		RegisterType<GraphNode_ComponentGet>("Get Component " + regComp->myName, regComp->myName, component);
		RegisterType<GraphNode_ComponentSet>("Set Component " + regComp->myName, regComp->myName, component);
	}
}
GraphNodeBase* GraphNodeRegistry::GetNodeTypeFromID(size_t aTypeId)
{
	return myTypes[aTypeId];
}

std::unordered_map<size_t, GraphNodeBase*>& GraphNodeRegistry::GetAllNodeTypes()
{
	return myTypes;
}

size_t GraphNodeRegistry::GetNodeTypeCount()
{
	return myTypes.size();
}

void GraphNodeRegistry::RegisterEvent(const std::string& aEventName, std::vector<DynamicPin>& aPins)
{
	RegisterType<GraphNode_EventCall>("GraphNode_EventCall_" + aEventName, aEventName, aPins);
	RegisterType<GraphNode_OnEvent>("GraphNode_OnEvent_" + aEventName, aEventName, aPins);
}

void GraphNodeRegistry::Destroy()
{
	for (auto& [id, type] : myTypes)
	{
		delete type;
		type = nullptr;
	}
}
