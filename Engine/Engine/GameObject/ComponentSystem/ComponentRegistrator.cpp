#include "pch.h"
#include "ComponentRegistrator.h"

#include <Engine/GameObject/Components/CharacterControllerComponent.h>
#include "ComponentSystem.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "Engine/GameObject/Components/BasicMovementComponent.h"
#include "Engine/GameObject/Components/CameraComponent.h"
#include "Engine/GameObject/Components/ModelComponent.h"
#include "Engine/GameObject/Components/RigidBodyComponent.h"
#include "Engine/GameObject/Components/SpriteComponent.h"

#include "Engine/GameObject/SingletonComponent.h"
#include "Engine/GameObject/Components/CameraShakeComponent.h"
#include "Engine/GameObject/Components/DecalComponent.h"
#include "Engine/GameObject/Components/EnvironmentLightComponent.h"
#include "Engine/GameObject/Components/FollowAnimatedBoneComponent.h"
#include "Engine/GameObject/Components/LogoFader.h"
#include "Engine/GameObject/Components/MeshComponent.h"
#include "Engine/GameObject/Components/PointLightComponent.h"
#include "Engine/GameObject/Components/PostProcessComponent.h"
#include "Engine/GameObject/Components/ReflectionCaptureComponent.h"
#include "Engine/GameObject/Components/ScriptsComponent.h"
#include "Engine/GameObject/Components/SpotLightComponent.h"
#include "Engine/GameObject/Components/TextComponent.h"
#include "Engine/Navmesh/NavmeshComponent.h"
#include "Engine/GameObject/Components/DecalComponent.h"
#include "Engine/GameObject/SingletonComponent.h"
#include "Engine/GameObject/Components/PostProcessComponent.h"
#include "Engine/GameObject/Components/ReflectionCaptureComponent.h"
#include "Engine/GameObject/Components/LogoFader.h"
#include "Engine/GameObject/Components/DecalComponent.h"
#include "Engine/GameObject/SingletonComponent.h"
#include "Engine/GameObject/Components/Collider.h"
#include "Engine/GameObject/Components/PostProcessComponent.h"
#include "Engine/GameObject/Components/ReflectionCaptureComponent.h"
#include "Engine/GameObject/Components/LogoFader.h"

void Engine::RegistrateComponents()
{
	//ComponentFactory::GetInstance().RegisterComponent<NavmeshComponent>("NavmeshComponent", UPDATE);

	ComponentFactory::GetInstance().RegisterComponent<RigidBodyComponent>("RigidBodyComponent", EARLY_PHYSICS | LATE_PHYSICS);
	ComponentFactory::GetInstance().RegisterComponent<Collider>("Collider");
}
