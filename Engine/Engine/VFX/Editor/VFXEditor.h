#pragma once
#include "Engine/VFX/Description/VFXDescription.h"
#include "VFXEditorEvent.h"
#include "Engine/GameObject/GameObject.h"

namespace Engine
{
	class VFXEditor
	{
	public:
		VFXEditor();
		void Init();
		void CreateDummyObject();

		void Tick();

		void CreateSystem();
		void Save();
		void Load(const std::string& aFilePath);

		void ReloadResource();

		void ObserveSave(const std::function<void(void)>& aObserver);

		const VFX::VFXDescription& GetDescription();
		//VFXRef GetVFXRef();
		GameObject* GetGameObject();

		static VFXEditor& GetInstance();
	private:
		void RegenerateEditor();

		void AddParticleEmitter();
		void AddPrefabEmitter();

		void AddParticleEmitter(const VFX::ParticleEmitterDescription& aDescription, const Timeline::Key& aKey);

		void DrawHeader();
		void DrawFooter();

		void RenderEvents();
		void DrawAddEvent();

		void DeleteEvents();
	private:
		VFX::VFXDescription myDescription;
		VFXRef myVFXRef;
		std::vector<VFXEditorEvent*> myParticleEmitterEvents;

		GameObject* myGameObject = nullptr;
		std::function<void(void)> mySaveObserver = nullptr;
	};
}
