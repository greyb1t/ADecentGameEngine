#pragma once
#include "VFXEditorEvent.h"
#include "Engine/VFX/Description/ParticleEmitterDescription.h"
#include "Engine/Reflection/ImguiReflector.h"

namespace Engine
{
	class VFXEditorParticlesEvent :
		public VFXEditorEvent
	{
	public:
		VFXEditorParticlesEvent();

		bool StartDraw() override;
		void EndDraw() override;

		void Header();

		void DrawAddVariableSpawn();
		void DrawAddVariableUpdate();
		void DrawSpawnVariables();
		void DrawUpdateVariables();

		void DrawTransform();
		void DrawGeometry();
		void DrawTimeline();

		void AddVariable(VFX::ParticleStartVariables::eVariables  aVariable);
		void AddVariable(VFX::ParticleUpdateVariables::eVariables  aVariable);

		void DrawVariable(VFX::ParticleStartVariables::eVariables  aVariable);
		void DrawVariable(VFX::ParticleUpdateVariables::eVariables  aVariable);

		void SetPropertyVariable(VFX::ParticleStartVariables::eVariables aVariable);
		void SetPropertyVariable(VFX::ParticleUpdateVariables::eVariables aVariable);

		const VFX::ParticleEmitterDescription& GetDescription();
		void SetDescription(const VFX::ParticleEmitterDescription&);
	private:
		VFX::ParticleEmitterDescription myEmitterDesc;
		VFX::ParticleDescription myParticleDesc;
		ImguiReflector myImguiReflector;
		MaterialRef myMaterialRef;
		TextureRef myTextureRef;
		AnimationCurveRef myMaterialCurveRef1;
		AnimationCurveRef myMaterialCurveRef2;
		AnimationCurveRef myMaterialCurveRef3;
		AnimationCurveRef myMaterialCurveRef4;

		AnimationCurveRef mySizeCurve;
		AnimationCurveRef myAccelerationCurve;
	};
}