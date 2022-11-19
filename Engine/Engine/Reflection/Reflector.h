#pragma once

#include "Engine/ResourceManagement/ResourceRef.h"
#include "Engine/Utils/Flags.h"
#include "Engine\Renderer\Material\MaterialType.h"
#include "Engine/Reflection/Enum.h"
#include "Engine/GameObject/GameObjectRef.h"
#include "Reflectable.h"

class Transform;

namespace Engine
{
	class MaterialInstance;
}

namespace Engine
{
	struct AnimationPair;
	struct VisualGraphPair;

	enum ReflectionFlags : uint32_t
	{
		ReflectionFlags_None = 0,
		ReflectionFlags_ReadOnly = 1 << 0,
		ReflectionFlags_IsColor = 1 << 1
	};

	enum ReflectorResult : uint32_t
	{
		ReflectorResult_None = 0,
		ReflectorResult_Changing = 1 << 0, // Value is changing and occurs everytime it was changed
		ReflectorResult_Changed = 1 << 1, // Value was changed, when user released the button
		ReflectorResult_HoveringOverItem = 1 << 2 // When value is possibly being modified (e.g. when we are dragging an ImGui slider)
	};

	CREATE_FLAG_OF_ENUM(ReflectorResult);

	template <typename T>
	class Array;

	class Enum;

	class Reflector
	{
	public:
		virtual ~Reflector() = default;

		// Just a simple helper to make easier syntax
		template <typename T>
		ReflectorResult Reflect(
			std::vector<T>& aArray,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None)
		{
			Array<T> arr(aArray, aFlags);
			// arr.Reflect(*this);
			Reflect(arr, aName, aFlags);
			return ReflectorResult::ReflectorResult_None;
		}

		// Just a simple helper to make easier syntax
		template <typename T>
		ReflectorResult ReflectEnum(
			T& aEnum,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None)
		{
			EnumClass<T> enums = aEnum._to_integral();
			auto result = Reflect(enums, aName, aFlags);
			aEnum._value = enums.GetValue();

			return result;
		}

		// Just a simple helper to make easier syntax
		ReflectorResult ReflectGroup(
			const std::function<void()> aCallback,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None)
		{
			struct Group : public Reflectable
			{
			public:
				Group(const std::function<void()>& aCallback) : myCallback(aCallback) {}

				void Reflect(Reflector& aReflector) override
				{
					myCallback();
				}

			private:
				const std::function<void()>& myCallback;
			};

			Group group(aCallback);

			return Reflect(group, aName, aFlags);
		}

		ReflectorResult Reflect(
			Reflectable& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None)
		{
			++myDepth;
			//IncReflectCounter();
			auto result = ReflectInternal(aValue, aName, aFlags);

			--myDepth;

			myCounter++;

			return result;
		}

		virtual ReflectorResult ReflectInternal(
			Reflectable& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None)
		{
			aValue.Reflect(*this);
			return ReflectorResult::ReflectorResult_None;
		}

		ReflectorResult Reflect(
			int& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None)
		{
			auto result = ReflectInternal(aValue, aName, aFlags);
			myCounter++;
			ResetAttributes();
			return result;
		}

		virtual ReflectorResult ReflectInternal(
			int& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None)
		{
			return ReflectorResult::ReflectorResult_None;
		}

		ReflectorResult Reflect(
			uint64_t& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None)
		{
			auto result = ReflectInternal(aValue, aName, aFlags);
			myCounter++;
			ResetAttributes();
			return result;
		}

		virtual ReflectorResult ReflectInternal(
			uint64_t& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None)
		{
			return ReflectorResult::ReflectorResult_None;
		}

		ReflectorResult Reflect(
			float& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None)
		{
			auto result = ReflectInternal(aValue, aName, aFlags);
			myCounter++;
			ResetAttributes();
			return result;
		}

		virtual ReflectorResult ReflectInternal(
			float& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None)
		{
			return ReflectorResult_None;
		}

		ReflectorResult Reflect(
			Vec2f& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None)
		{
			auto result = ReflectInternal(aValue, aName, aFlags);
			myCounter++;
			ResetAttributes();
			return result;
		}

		virtual ReflectorResult ReflectInternal(
			Vec2f& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None)
		{
			return ReflectorResult::ReflectorResult_None;
		}

		ReflectorResult Reflect(
			Vec3f& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None)
		{
			auto result = ReflectInternal(aValue, aName, aFlags);
			myCounter++;
			ResetAttributes();
			return result;
		}

		virtual ReflectorResult ReflectInternal(
			Vec3f& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None)
		{
			return ReflectorResult::ReflectorResult_None;
		}

		ReflectorResult Reflect(
			Vec4f& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None)
		{
			auto result = ReflectInternal(aValue, aName, aFlags);
			myCounter++;
			ResetAttributes();
			return result;
		}

		virtual ReflectorResult ReflectInternal(
			Vec4f& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None)
		{
			return ReflectorResult::ReflectorResult_None;
		}

		ReflectorResult Reflect(
			bool& aValue, const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None)
		{
			auto result = ReflectInternal(aValue, aName, aFlags);
			myCounter++;
			ResetAttributes();
			return result;
		}

		virtual ReflectorResult ReflectInternal(
			bool& aValue, const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None)
		{
			return ReflectorResult::ReflectorResult_None;
		}

		ReflectorResult Reflect(
			std::string& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None)
		{
			auto result = ReflectInternal(aValue, aName, aFlags);
			myCounter++;
			ResetAttributes();
			return result;
		}

		virtual ReflectorResult ReflectInternal(
			std::string& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None)
		{
			return ReflectorResult::ReflectorResult_None;
		}

		ReflectorResult Reflect(
			ModelRef& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None)
		{
			auto result = ReflectInternal(aValue, aName, aFlags);
			myCounter++;
			ResetAttributes();
			return result;
		}

		virtual ReflectorResult ReflectInternal(
			ModelRef& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None)
		{
			return ReflectorResult::ReflectorResult_None;
		}

		ReflectorResult Reflect(
			MaterialRef& aValue,
			const MaterialType aType,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None)
		{
			auto result = ReflectInternal(aValue, aType, aName, aFlags);
			myCounter++;
			ResetAttributes();
			return result;
		}

		virtual ReflectorResult ReflectInternal(
			MaterialRef& aValue,
			const MaterialType aType,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None)
		{
			return ReflectorResult::ReflectorResult_None;
		}

		ReflectorResult Reflect(
			AnimationPair& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None)
		{
			auto result = ReflectInternal(aValue, aName, aFlags);
			myCounter++;
			ResetAttributes();
			return result;
		}

		virtual ReflectorResult ReflectInternal(
			AnimationPair& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None)
		{
			return ReflectorResult::ReflectorResult_None;
		}

		ReflectorResult Reflect(
			TextureRef& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None)
		{
			auto result = ReflectInternal(aValue, aName, aFlags);
			myCounter++;
			ResetAttributes();
			return result;
		}

		virtual ReflectorResult ReflectInternal(
			TextureRef& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None)
		{
			return ReflectorResult::ReflectorResult_None;
		}

		ReflectorResult Reflect(
			VisualGraphPair& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None)
		{
			auto result = ReflectInternal(aValue, aName, aFlags);
			myCounter++;
			ResetAttributes();
			return result;
		}

		virtual ReflectorResult ReflectInternal(
			VisualGraphPair& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None)
		{
			return ReflectorResult::ReflectorResult_None;
		}

		ReflectorResult Reflect(
			AnimationCurveRef& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None)
		{
			auto result = ReflectInternal(aValue, aName, aFlags);
			myCounter++;
			ResetAttributes();
			return result;
		}

		virtual ReflectorResult ReflectInternal(
			AnimationCurveRef& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None)
		{
			return ReflectorResult::ReflectorResult_None;
		}

		ReflectorResult Reflect(
			GameObjectPrefabRef& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None)
		{
			auto result = ReflectInternal(aValue, aName, aFlags);
			myCounter++;
			ResetAttributes();
			return result;
		}

		virtual ReflectorResult ReflectInternal(
			GameObjectPrefabRef& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None)
		{
			return ReflectorResult::ReflectorResult_None;
		}

		ReflectorResult Reflect(
			VFXRef& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None)
		{
			auto result = ReflectInternal(aValue, aName, aFlags);
			myCounter++;
			ResetAttributes();
			return result;
		}

		virtual ReflectorResult ReflectInternal(
			VFXRef& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None)
		{
			return ReflectorResult::ReflectorResult_None;
		}

		ReflectorResult Reflect(
			Transform& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None)
		{
			auto result = ReflectInternal(aValue, aName, aFlags);
			myCounter++;
			ResetAttributes();
			return result;
		}

		virtual ReflectorResult ReflectInternal(
			Transform& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None)
		{
			return ReflectorResult::ReflectorResult_None;
		}

		ReflectorResult Reflect(
			GameObjectRef& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None)
		{
			auto result = ReflectInternal(aValue, aName, aFlags);
			myCounter++;
			ResetAttributes();
			return result;
		}

		virtual ReflectorResult ReflectInternal(
			GameObjectRef& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None)
		{
			return ReflectorResult::ReflectorResult_None;
		}

		ReflectorResult Reflect(
			Enum& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None)
		{
			auto result = ReflectInternal(aValue, aName, aFlags);
			myCounter++;
			ResetAttributes();
			return result;
		}

		virtual ReflectorResult ReflectInternal(
			Enum& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None)
		{
			return ReflectorResult::ReflectorResult_None;
		}

		virtual void ReflectLambda(const std::function<void(void)>& aLambda) { }

		int GetDepth() const;
		int GetCounter() const;

		// USED IN ImguiReflector
		// E.g. for DragFloat() its speed of values changing when dragging
		virtual void SetNextItemSpeed(const float aSpeed) { }
		virtual void SetNextItemRange(const float aMin, const float aMax) { }
		virtual void SetNextItemTooltip(const std::string& aText) { }
		virtual void SetNextItemHidden() { }
		virtual void Header(const std::string& aHeaderText) { }
		virtual void Separator() { }
		virtual bool Button(const std::string& aText) { return false; }

		virtual void ResetAttributes() { }

	private:
		// The depth into the visit "tree"
		// we can recusively go into deeper reflectables such as Arrays.
		// Therefore, we keep depth here and use it in FindValueReflector
		// to ensure that if a duplicate string exists in the whole Reftectable
		// its on another depth, therefore use the depth to avoid choosing the wrong
		// value
		int myDepth = 0;
		int myCounter = 0;
	};
}

// Must be here because it depends on Reflector
// I am either stupid, or I hate templates
#include "Engine/Reflection/Array.h"