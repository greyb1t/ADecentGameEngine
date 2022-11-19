#pragma once

#include "Inspectable.h"
#include "Engine\Editor\Selection/Selection.h"
#include "InspectGameObject.h"

class GameObject;

namespace Engine
{
	class InspectableSelection : public Inspectable
	{
	public:
		InspectableSelection(
			Editor& aEditor, 
			InspectorWindow& aInspectorWindow,
			const Selection& aSelection);

		void Draw() override;

		std::string GetInspectingType() const override { return "GameObject Selection"; }

	private:

	private:
		const Selection* mySelection = nullptr;

		InspectGameObject myInspectGameObject;
	};
}