#pragma once

// Must match with ResourceTypeStrings 1:1
enum class ResourceType
{
	AnimationClip,
	AnimationCurve,
	AnimationStateMachine,

	PixelShader,
	VertexShader,
	GeometryShader,

	MeshMaterial,

	Texture,
	Model,

	VisualScript,

	VFX,

	GameObjectPrefab,

	Count,
	Unknown
};

// Must match with ResourceType 1:1
const std::string ResourceTypeStrings[] =
{ 
	"AnimationClip",
	"AnimationCurve",
	"AnimationStateMachine",
	"PixelShader",
	"VertexShader",
	"GeometryShader",
	"MeshMaterial",
	"Texture",
	"Model",
	"VisualScript",
	"VFX",
	"GameObjectPrefab",
};

enum class ResourceState
{
	Unloaded,
	Loaded,
	Requested
};

enum class ResourceLoadResult
{
	None,
	Succeeded,
	Failed
};