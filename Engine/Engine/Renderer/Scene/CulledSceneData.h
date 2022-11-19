#pragma once

#include "Engine/UUID.h"

namespace Engine
{
	class SpriteComponent;
	class SpotLightComponent;
	class PointLightComponent;
	class MeshComponent;
	class DecalComponent;
	class ParticleEmitterComponent;
	class ReflectionCaptureComponent;
}

namespace Engine
{
	class PointLight;
	class SpotLight;
	class ModelInstance;
	class ParticleEmitterInstance;
	class Text;
	class MeshInstance;
	class EffectPass;

	struct CulledMeshInstance
	{
		MeshInstance* myMeshInstance = nullptr;
		UUID myUUID{};

		// We divide the meshes by their effect in the culling stage
		// so we can use different renderers for each shader render pass
		// This also opens up ability to optimize based on material and dx states =)
		EffectPass* myEffectPass = nullptr;
	};

	struct CulledMeshComponent
	{
		MeshComponent* myMeshComponent = nullptr;
		UUID myUUID{};

		// We divide the meshes by their effect in the culling stage
		// so we can use different renderers for each shader render pass
		// This also opens up ability to optimize based on material and dx states =)
		EffectPass* myEffectPass = nullptr;
	};

	struct CulledDecalComponent
	{
		DecalComponent* myDecalComponent = nullptr;

		EffectPass* myEffectPass = nullptr;
	};

	struct CulledSceneData
	{
		struct CulledPointLightData
		{
			PointLightComponent* myPointLight = nullptr;

			std::vector<CulledMeshInstance> myNewWholeSceneAllDirectionsShadowCastersMeshes;

			// Each index is the side of the cube
			std::array<std::vector<CulledMeshInstance>, 6> myPointLightShadowCastersMeshes;
		};

		struct CulledSpotLightData
		{
			SpotLightComponent* mySpotLight = nullptr;

			std::vector<CulledMeshInstance> mySpotLightShadowCastersMeshes;
		};

		std::vector<CulledMeshInstance> myDeferredMeshes;
		std::vector<CulledMeshInstance> myForwardMeshes;
		std::vector<CulledMeshInstance> myForwardTransparentMeshes;

		std::vector<CulledMeshComponent> myDeferredMeshComponents;
		std::vector<CulledMeshComponent> myForwardMeshComponents;
		std::vector<CulledMeshComponent> myForwardTransparentMeshComponents;

		std::vector<CulledDecalComponent> myDeferredDecalComponents;

		std::vector<ParticleEmitterComponent*> emitters;

		std::vector<SpriteComponent*> spriteInstances;
		std::vector<SpriteComponent*> my3DSprites;

		std::vector<Text*> texts;
		std::vector<Text*> my2DTexts;

		std::vector<CulledSpotLightData> frustumCulledSpotLights;
		std::vector<CulledPointLightData> frustumCulledPointLights;

		// The models that will cast shadows on the environment light
		std::vector<std::vector<CulledMeshInstance>> myEnvironmentLightShadowCastersMeshes;

		std::vector<ReflectionCaptureComponent*> myReflectionCaptureComponents;
	};
}
