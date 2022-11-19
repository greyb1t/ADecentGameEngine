#pragma once

namespace Engine
{
class ResourceManager;
class DebugDrawer;
class Scene;
}

// Contains global shortcuts to most used stuff to avoid code bloat
// Add here as you please
// You may say that this is a bad idea, but, I do NOT care what you say :)

// Scene that is being iterated upon
extern Engine::Scene* GScene;

// Debug drawer for the scene being currently iterated upon
// Meaning, inside Component::Update() it will get the Scene that contains this component
// NOT, the scene that is the main scene
extern Engine::DebugDrawer* GDebugDrawer;

// Should be safe to access from any thread
extern Engine::ResourceManager* GResourceManager;