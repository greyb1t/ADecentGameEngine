#pragma once
#include "Engine\Scene\FolderScene.h"
#include "Engine/VFX/Editor/VFXEditor.h"

namespace Engine
{
	class VFXComponent;

	class VFXEditorScene :
		public FolderScene
	{
	public:
		VFXEditorScene();

		Result OnRuntimeInit() override;

		VFXEditorScene* Clone() const override;

		void Play();

		void Update(TimeStamp ts) override;
		bool myAttraction = false;
		Vec3f myAttractionPoint = { -100, 0, -100 };

		Weak<GameObject> vfxObj;
		bool autoPlay = false;
		float autoPlayTimer = 0;

		bool moving = false;

		float sinusSpeed = .5f;
		float sinusRange = 500;

		float rotX = 0;
		float rotY = 0;
		float rotZ = 0;
	};
}
