#pragma once
#include "Engine/VFX/Description/ParticleEmitterDescription.h"
#include "Engine/VFX/Description/PhysicsParticleEmitterDescription.h"
#include "Engine/VFX/Description/PhysicsEmitterDescription.h"
#include <string>

#include "PrefabEmitterDescription.h"
#include "Engine/VFX/Time/ObjectTimeline.h"
#include "Engine/VFX/Time/Timeline.h"
#include "Engine/ResourceManagement/Resources/VertexShaderResource.h"
#include "Engine/ResourceManagement/Resources/PixelShaderResource.h"
#include "Engine/ResourceManagement/Resources/GeometryShaderResource.h"
#include "Engine/ResourceManagement/Resources/TextureResource.h"

namespace VFX
{
	struct VFXDescription
	{
		ObjectTimeline<ParticleEmitterDescription> particleEmitters;
		ObjectTimeline<PrefabEmitterDescription> prefabEmitters;
		std::string title = "VFX_DEFAULT";
		eSpace space = eSpace::LOCALSPACE;
		float duration = 1.f;
		bool looping = false;

		// Functions
		void Add(const ParticleEmitterDescription& aDesc, const Timeline::Key& aKey)
		{
			particleEmitters.Add(aDesc, aKey);
		}
		void Add(const PrefabEmitterDescription& aDesc, const Timeline::Key& aKey)
		{
			prefabEmitters.Add(aDesc, aKey);
		}
	};
}
