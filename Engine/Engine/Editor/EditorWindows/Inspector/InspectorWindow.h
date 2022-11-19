#pragma once

#include "Engine/Editor/EditorWindows/EditorWindow.h"
#include "Engine/ResourceManagement/ResourceRef.h"

class GameObject;
class Transform;

namespace Engine
{
	class Inspectable;
	class ResourceReferences;

	class InspectorWindow : public EditorWindow
	{
	public:
		InspectorWindow(Editor& aEditor);

		bool Init(ResourceReferences& aResRefs);

		void InspectObject(Owned<Inspectable> aInspectable);
		Inspectable* GetInspectable();

		void Draw(const float aDeltaTime) override;

		TextureRef GetSearchResourceIcon();

		std::string GetName() const override { return "Inspector"; }

		void Save() override;

	private:
		void DrawTransform(Transform& aTransform);
		void UpdateKeypresses();

	private:
		friend class Editor;

		TextureRef mySearchResourceIcon;

		Owned<Inspectable> myInspectable;
	};
}
