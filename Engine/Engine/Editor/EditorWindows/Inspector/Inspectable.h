#pragma once

namespace Engine
{
	class Editor;
	class InspectorWindow;

	class Inspectable
	{
	public:
		Inspectable(Editor& aEditor, InspectorWindow& aInspectorWindow);

		virtual ~Inspectable();

		virtual void Draw();

		virtual void Save();

		virtual std::string GetInspectingType() const = 0;

	protected:
		Editor& myEditor;
		InspectorWindow& myInspectorWindow;
	};
}