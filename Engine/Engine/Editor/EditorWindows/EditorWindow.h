#pragma once

#include "Engine/Editor/EditorMode.h"

namespace Engine
{
	class Editor;

	class EditorWindow
	{
	public:
		enum Flags : int
		{
			None = 0,

			// Handles Ctrl+S to call Save()
			// Warns user when closing window if still have unsaved progress
			CanBeUnsaved = 1 << 0,

			// Draws a close button and automatic functionality for it (if its an "extra window")
			// Also if window tries to be closed, it shows a popup to save before closing
			CanBeClosed = 1 << 1
		};

		EditorWindow(Editor& aEditor, int aFlags = Flags::None)
			: myEditor(aEditor),
			myFlags(static_cast<Flags>(aFlags))
		{
		}

		virtual ~EditorWindow() = default;

		void UpdateInternal(const float aDeltaTime);

		void BringToFront(bool aFocus = true);
		void ShowFeedback();
		void MakeUnsaved();
		void Close();
		bool IsOpen() const;

		virtual void OnEditorChangeMode(const EditorMode aMode) { }

		virtual std::string GetName() const = 0;

		Editor& GetEditor();

		// Override to make it save on Ctrl+S
		virtual void Save();

	protected:
		virtual bool Begin(const float aDeltaTime);
		virtual void Draw(const float aDeltaTime) = 0;
		virtual void End(const float aDeltaTime);

		// override if want to call ImGui::Begin() with specific flags
		virtual ImGuiWindowFlags GetWindowFlags() const { return 0; }

		// It called if the window is focused
		virtual void UpdateFocused(const float aDeltaTime) { }

	protected:
		Editor& myEditor;

		Flags myFlags = Flags::None;

		bool myBringToFront = false;
		bool myBringToFrontFocus = false;

		float myFeedbackDownTimer = 0.f;
		static inline const float ourFeedbackDownDuration = 0.5f;
		Vec4f myFeedbackColor = Vec4f(0.2f, 0.2f, 0.2f, 1.f);
		bool myIsOpen = true;

		bool myHasSaved = true;
	};
}