#pragma once

#include "Engine/Scene/FolderScene.h"
#include "Engine/VFX/Time/ObjectTimeline.h"
#include "Engine/VFX/Time/Timeline.h"
#include "Engine/VFX/VFXSystem.h"
#include "Engine/VFX/Editor/VFXEditor.h"

namespace Engine
{
	class VFXComponent;
	class RigidBodyComponent;
}

class ViktorTestScene : public Engine::FolderScene
{
public:
	Result OnRuntimeInit() override;

	ViktorTestScene* Clone() const override;

	void Update(TimeStamp ts) override;
private:
};
