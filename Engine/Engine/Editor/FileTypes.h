#pragma once

namespace FileType
{
	enum class FileType
	{
		DDS, // .dds
		HDR, // .hdr
		MeshMaterial, // .meshmat
		DecalMaterial, // .decalmat
		SpriteMaterial, // .spritemat
		ParticleMaterial, // .particlemat
		FBX, // .fbx
		AnimationStateMachine, // .animation
		Model, // .model
		VisualScript, // .vgraph
		PrefabGameObject, // .prefabg
		AnimationClip, // .anim
		AnimationCurve, // .animcurve
		Scene, // .scene
		VFX, // .vfx
		VertexShader, // .vs
		PixelShader, // .ps
		GeometryShader, // .gs
		Landscape, //Landscape

		Count,
		Unknown
	};

	// NOTE(filip): must be in same order as the FileType enum
	const std::string GlobalFileTypeStrings[] =
	{
		"DDS",
		"HDR",
		"MeshMaterial",
		"DecalMaterial",
		"SpriteMaterial",
		"ParticleMaterial",
		"FBX",
		"AnimationStateMachine",
		"Model",
		"VisualGraph",
		"PrefabGameObject",
		"AnimationClip",
		"AnimationCurve",
		"Scene",
		"VFX",
		"VertexShader",
		"PixelShader",
		"GeometryShader",
		"landscape",
		"Unknown"
	};

	const std::string Extension_DDS = ".dds";
	const std::string Extension_HDR = ".hdr";
	const std::string Extension_MeshMaterial = ".meshmat";
	const std::string Extension_DecalMaterial = ".decalmat";
	const std::string Extension_SpriteMaterial = ".spritemat";
	const std::string Extension_ParticleMaterial = ".particlemat";
	const std::string Extension_FBX = ".fbx";
	const std::string Extension_AnimationStateMachine = ".animation";
	const std::string Extension_Model = ".model";
	const std::string Extension_VisualScript = ".vscript";
	const std::string Extension_GameObjectPrefab = ".prefabg";
	const std::string Extension_AnimationClip = ".anim";
	const std::string Extension_AnimationCurve = ".animcurve";
	const std::string Extension_Scene = ".scene";
	const std::string Extension_VFX = ".vfx";
	const std::string Extension_VertexShader = ".vs";
	const std::string Extension_PixelShader = ".ps";
	const std::string Extension_GeometryShader = ".gs";	
	const std::string Extension_Landscape = ".landscape";

	// aExtension can be e.g .dds 
	FileType ExtensionToFileType(std::string aExtension);

	std::string FileTypeToExtension(const FileType aFileType);

	const std::string& FileTypeToString(const FileType aFileType);
}