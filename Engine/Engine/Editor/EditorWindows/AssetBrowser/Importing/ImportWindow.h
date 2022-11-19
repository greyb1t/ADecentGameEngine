#pragma once

#include "Engine\Editor\EditorWindows\EditorWindow.h"
#include "Engine\Renderer\Utils\Observer.h"
#include "CreationStates\ResourceImportState.h"

namespace Engine
{
	class CreationState;
	class Editor;

	class ImportWindow : public EditorWindow, public Observer
	{
	public:
		ImportWindow(Editor& aEditor);
		~ImportWindow();

		void EnterImportState(const ResourceImportState& aCreationState, const std::filesystem::path& aPath);
		void EnterImportState(const ResourceImportState& aCreationState);

		std::string GetName() const override;


		void Receive(const EventType aEventType, const std::any& aValue) override;

		void Draw(const float aDeltaTime) override;

	private:
		// Importing
		ResourceImportState myCreationStateType = ResourceImportState::None;
		Owned<CreationState> myImportState;
	};
}