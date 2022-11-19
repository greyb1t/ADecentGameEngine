#pragma once

#include "Component.h"
#include <Engine/GraphManager/GraphInstance.h>
#include "Engine/ResourceManagement/ResourceRef.h"

namespace Engine
{
	class ScriptsComponent : public Component
	{
	public:
		COMPONENT(ScriptsComponent, "ScriptsComponent");

		ScriptsComponent() = default;
		ScriptsComponent(GameObject* aGameObject);

		GraphInstance* GetGraphInstance();

		void Reflect(Reflector& aReflector) override;

		void Start() override;

		void Execute(eEngineOrder aOrder) override;

	private:
		GraphInstance* myGraphInstance = nullptr;
		VisualScriptRef myGraph;
	};
}