#pragma once

#include "Reflector.h"
#include "Engine/ResourceReferences.h"

namespace Engine
{
	class GameObjectPrefabReflector : public Reflector
	{
	public:
		GameObjectPrefabReflector(Reflectable& aTargetReflectable, Reflectable* aBeforeChangedReflectable);

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

		ReflectorResult ReflectInternal(
			GameObjectRef& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None) override;

		ReflectorResult ReflectInternal(
			Enum& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None) override;

	private:
		template <typename T>
		void DoIt(T& aValue, const std::string& aName)
		{
			if (aName == "Beginner Min Max")
			{
				int test = 0;
			}

			const int currentDepth = GetDepth();
			const int currentCounter = GetCounter();

			FindValueReflector refl(
				[&aValue, &aName, this, currentDepth, currentCounter](void* aPtr) -> ReflectorResult
				{
					bool wasPrefabValueChanged = false;
					bool isInstanceValueOverriden = false;

					// Find the value and compare it against the GameObject/Component BEFORE the modifications
					FindValueReflector refl2([aPtr, &wasPrefabValueChanged, &isInstanceValueOverriden, &aValue](void* aUnchangedValuePtr) -> ReflectorResult
						{
							// Compare the unchanged version of the prefab value, and the new prefab value
							// If changed, we below outside this lambda update it
							if (*reinterpret_cast<T*>(aUnchangedValuePtr) != aValue)
							{
								wasPrefabValueChanged = true;
							}

							if (*reinterpret_cast<T*>(aUnchangedValuePtr) != *reinterpret_cast<T*>(aPtr))
							{
								isInstanceValueOverriden = true;
							}

							return ReflectorResult_None;
						}, aName, currentDepth, currentCounter);

					myBeforeChangedReflectable->Reflect(refl2);

					// We only want to modify the unchanged values to keep the instance specific value unchanged
					if (wasPrefabValueChanged && !isInstanceValueOverriden)
					{
						*reinterpret_cast<T*>(aPtr) = aValue;
						return ReflectorResult_Changed;
					}

					return ReflectorResult_None;
				}, aName, currentDepth, currentCounter);

			// Modify the target value with this value
			myTargetReflectable.Reflect(refl);
		}

	private:
		Reflectable& myTargetReflectable;
		Reflectable* myBeforeChangedReflectable = nullptr;
	};
}
