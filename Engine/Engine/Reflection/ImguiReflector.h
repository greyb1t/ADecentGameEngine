#pragma once

#include "Reflector.h"
#include "Engine/GameObject/Components/Component.h"
#include "Engine/GameObject/ComponentSystem/ComponentFactory.h"

namespace Engine
{
	class Scene;
	class Editor;

	class ImguiReflector : public Reflector
	{
	public:
		ImguiReflector(
			Editor& aEditor,
			Reflectable* aCompareAgainst,
			GameObject* aPrefabInstanceGameObject);

		virtual ~ImguiReflector() = default;

		ReflectorResult ReflectInternal(
			Reflectable& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None) override;

		ReflectorResult ReflectInternal(
			int& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None) override;

		ReflectorResult ReflectInternal(
			uint64_t& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None) override;

		ReflectorResult ReflectInternal(
			float& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None) override;

		ReflectorResult ReflectInternal(
			Vec2f& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None) override;

		ReflectorResult ReflectInternal(
			Vec3f& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None) override;

		ReflectorResult ReflectInternal(
			Vec4f& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None) override;

		ReflectorResult ReflectInternal(
			bool& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None) override;

		ReflectorResult ReflectInternal(
			std::string& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None) override;

		ReflectorResult ReflectInternal(
			ModelRef& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None) override;

		ReflectorResult ReflectInternal(
			MaterialRef& aValue,
			const MaterialType aType,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None) override;

		ReflectorResult ReflectInternal(
			AnimationPair& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None) override;

		ReflectorResult ReflectInternal(
			TextureRef& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None) override;

		ReflectorResult ReflectInternal(
			VisualGraphPair& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None) override;

		ReflectorResult ReflectInternal(
			AnimationCurveRef& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags) override;

		ReflectorResult ReflectInternal(
			GameObjectPrefabRef& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags) override;

		ReflectorResult ReflectInternal(
			VFXRef& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags) override;

		ReflectorResult ReflectInternal(
			Transform& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None) override;

		virtual ReflectorResult ReflectInternal(
			GameObjectRef& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None) override;

		ReflectorResult ReflectInternal(
			Enum& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None) override;

		void ReflectLambda(const std::function<void(void)>& aLambda) override;

		void SetNextItemSpeed(const float aSpeed) override;
		void SetNextItemRange(const float aMin, const float aMax) override;
		void SetNextItemTooltip(const std::string& aText) override;
		void SetNextItemHidden() override;
		void Header(const std::string& aHeaderText) override;
		void Separator() override;
		bool Button(const std::string& aText) override;

		void ResetAttributes() override;

	private:
		template <typename T>
		bool IsEqualToPrefab(const T& aValue, const std::string& aName)
		{
			bool equals = false;

			FindValueReflector refl(
				[&equals, &aValue](void* aPtr) -> ReflectorResult
				{
					if (*reinterpret_cast<T*>(aPtr) == aValue)
					{
						equals = true;
					}

					return ReflectorResult_None;
				}, aName, GetDepth(), GetCounter());

			// Modify the target value with this value
			myCompareAgainst->Reflect(refl);

			return equals;
		}

		template <typename T>
		void DrawName(const std::string& aName, T& aValue, const ReflectionFlags aFlags)
		{
			std::string sModifierTemp;

			if (!(aFlags & ReflectionFlags_ReadOnly))
			{
				if (myCompareAgainst)
				{
					if (!IsEqualToPrefab<T>(aValue, aName))
					{
						sModifierTemp = " *";

						auto cp = ImGui::GetCursorPos();
						ImGui::PushFont(GraphicsEngine::ourFont16Bold);
						ImGuiHelper::AlignedWidget2(aName + sModifierTemp, Editor::ourAlignPercent);
						ImVec2 textSize = ImGui::CalcTextSize((aName + sModifierTemp).c_str());
						ImGui::PopFont();

						auto cp2 = ImGui::GetCursorPos();

						ImGui::SetCursorPos(cp);
						if (ImGui::InvisibleButton("modifierbutton", textSize))
						{
						}

						ImGuiHelper::AlignedWidget2("", Editor::ourAlignPercent);

						ImGui::SetCursorPos(cp2);
						//return;

						if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
						{
							ImGui::OpenPopup("RightClickedModifiedValue");
						}

						if (ImGui::BeginPopupContextItem("RightClickedModifiedValue"))
						{
							// TODO: Show the value to revert to "Revert value to: %f"
							if (ImGui::MenuItem("Revert to prefab value"))
							{
								FindValueReflector refl(
									[&aValue](void* aPtr) -> ReflectorResult
									{
										aValue = *reinterpret_cast<T*>(aPtr);
										return ReflectorResult_Changed | ReflectorResult_Changing;
									}, aName, GetDepth(), GetCounter());

								// Modify the target value with this value
								myCompareAgainst->Reflect(refl);
							}

							if (ImGui::MenuItem("Apply to prefab"))
							{
								LOG_ERROR(LogType::Editor) << "Unimplemented";
								//ApplyToPrefab();
							}

							ImGui::EndPopup();
						}

						return;
					}
				}
			}

			ImGuiHelper::AlignedWidget2(aName, Editor::ourAlignPercent);
		}

		void ApplyToPrefab();

	private:
		Editor& myEditor;

		// Used to show overridden values in the prefab instances
		// Assumed to be an exact 1:1 Reflect() hierarchy as the current thing being reflected upon because we use ReflectCounter value to know 
		// how many values we have reflected
		Reflectable* myCompareAgainst = nullptr;

		GameObject* myPrefabInstanceGameObject = nullptr;

		// Attributes
		float mySpeed = 1.f;

		float myRangeMin = 0.f;
		float myRangeMax = 0.f;

		std::string myTooltipText;

		bool myHideNextItem = false;

		bool myDraw = true;
	};
}
