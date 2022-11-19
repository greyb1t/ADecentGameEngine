#pragma once

#include "AnimationState.h"

namespace Engine
{
	class Model;
	class AnimationClip;
}

namespace Engine
{
	struct AnimationStateDesc;
	class Playable;
	class AnimationController;
	class AnimationMachineLayer;
	class PlayableVisitor;

	class AnimationMachine
	{
	public:
		AnimationMachine();
		~AnimationMachine();

		void ResetToInitialState();
		void SetController(AnimationController& aController);

		bool InitFromJson2(const nlohmann::json& aJson);
		nlohmann::json ToJson() const;

		AnimationTransformResult Update(const float aDeltaTime, AnimationController& aController);

		const AnimationClipRef& GetFirstAnimationClip() const;

		void Trigger(const std::string& aTriggerName);
		void RemoveTrigger(const std::string& aTriggerName);
		bool HasTrigger(const std::string& aTriggerName) const;
		bool HasBool(const std::string& aConditionName) const;

		void SetBool(const std::string& aBoolName, const bool aValue);
		bool GetBool(const std::string& aBoolName) const;
		void RemoveBool(const std::string& aBoolName);

		void SetFloat(const std::string& aFloatName, const float aValue);
		float GetFloat(const std::string& aFloatName) const;
		void RemoveFloat(const std::string& aFloatName);

		void SetInt(const std::string& aIntName, const int aValue);
		int GetInt(const std::string& aIntName) const;
		void RemoveInt(const std::string& aIntName);

		void SetVec2(const std::string& aVecName, const Vec2f& aValue);
		const Vec2f& GetVec2(const std::string& aVecName) const;
		void RemoveVec2(const std::string& aVec2Name);

		const std::unordered_map<std::string, Owned<bool>>& GetTriggers() const;
		const std::unordered_map<std::string, Owned<bool>>& GetBools() const;
		const std::unordered_map<std::string, Owned<float>>& GetFloats() const;
		const std::unordered_map<std::string, Owned<Vec2f>>& GetVec2s() const;
		const std::unordered_map<std::string, Owned<int>>& GetInts() const;

		std::vector<Owned<AnimationMachineLayer>>& GetLayers();
		AnimationMachineLayer* FindLayer(const std::string& aName);
		void RemoveLayer(const AnimationMachineLayer* aLayer);

		bool IsInitialized() const;

		void ClearBindPose();
		void GenerateBindPose(const Model& aModel);
		static void GenerateBindPose(const Model& aModel, const AnimationClip& aClip, AnimationTransformResult& aBindPose);

		void VisitPlayables(PlayableVisitor& aVisitor);

		const AnimationTransformResult& GetBindPose() const;

	private:
		friend struct AnimationStateDesc;
		friend class AnimationNodeEditorWindow;
		friend class AnimationState;
		friend class BlendNode1D;
		friend class NormalAnimation;
		friend class AnimationController;
		friend class BlendTree2DEditor;

		AnimationClipRef myFirstAnimationClip;
		ModelRef myModel;

		std::unordered_map<std::string, Owned<bool>> myTriggers;
		std::unordered_map<std::string, Owned<bool>> myBools;
		std::unordered_map<std::string, Owned<float>> myFloats;
		std::unordered_map<std::string, Owned<Vec2f>> myVec2s;
		std::unordered_map<std::string, Owned<int>> myInts;

		// Must use vector because I need to change the order myself
		std::vector<Owned<AnimationMachineLayer>> myLayers;

		bool myIsInitialized = false;

		std::vector<ResourceRef<AnimationClipResource>> myAnimationClips;

		AnimationTransformResult myBindPose;
	};
}