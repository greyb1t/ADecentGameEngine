#include "pch.h"
#include "FileTypes.h"

FileType::FileType FileType::ExtensionToFileType(std::string aExtension)
{
	aExtension = StringUtilities::ToLower(aExtension);

	if (aExtension == Extension_DDS)
	{
		return FileType::DDS;
	}
	else if (aExtension == Extension_HDR)
	{
		return FileType::HDR;
	}
	else if (aExtension == Extension_MeshMaterial)
	{
		return FileType::MeshMaterial;
	}
	else if (aExtension == Extension_DecalMaterial)
	{
		return FileType::DecalMaterial;
	}
	else if (aExtension == Extension_SpriteMaterial)
	{
		return FileType::SpriteMaterial;
	}
	else if (aExtension == Extension_ParticleMaterial)
	{
		return FileType::ParticleMaterial;
	}
	else if (aExtension == Extension_FBX)
	{
		return FileType::FBX;
	}
	else if (aExtension == Extension_AnimationStateMachine)
	{
		return FileType::AnimationStateMachine;
	}
	else if (aExtension == Extension_Model)
	{
		return FileType::Model;
	}
	else if (aExtension == Extension_VisualScript)
	{
		return FileType::VisualScript;
	}
	else if (aExtension == Extension_GameObjectPrefab)
	{
		return FileType::PrefabGameObject;
	}
	else if (aExtension == Extension_AnimationClip)
	{
		return FileType::AnimationClip;
	}
	else if (aExtension == Extension_AnimationCurve)
	{
		return FileType::AnimationCurve;
	}
	else if (aExtension == Extension_Scene)
	{
		return FileType::Scene;
	}
	else if (aExtension == Extension_VFX)
	{
		return FileType::VFX;
	}
	else if (aExtension == Extension_VertexShader)
	{
		return FileType::VertexShader;
	}
	else if (aExtension == Extension_PixelShader)
	{
		return FileType::PixelShader;
	}
	else if (aExtension == Extension_GeometryShader)
	{
		return FileType::GeometryShader;
	}
	else if (aExtension == Extension_Landscape)
	{
		return FileType::Landscape;
	}

	return FileType::Unknown;
}

std::string FileType::FileTypeToExtension(const FileType aFileType)
{
	switch (aFileType)
	{
	case FileType::DDS: return Extension_DDS;
	case FileType::HDR: return Extension_HDR;
	case FileType::MeshMaterial: return Extension_MeshMaterial;
	case FileType::SpriteMaterial: return Extension_SpriteMaterial;
	case FileType::DecalMaterial: return Extension_DecalMaterial;
	case FileType::ParticleMaterial: return Extension_ParticleMaterial;
	case FileType::FBX: return Extension_FBX;
	case FileType::AnimationStateMachine: return Extension_AnimationStateMachine;
	case FileType::Model: return Extension_Model;
	case FileType::VisualScript: return Extension_VisualScript;
	case FileType::PrefabGameObject: return Extension_GameObjectPrefab;
	case FileType::AnimationClip: return Extension_AnimationClip;
	case FileType::AnimationCurve: return Extension_AnimationCurve;
	case FileType::Scene: return Extension_Scene;
	case FileType::VFX: return Extension_VFX;
	case FileType::VertexShader: return Extension_VertexShader;
	case FileType::PixelShader: return Extension_PixelShader;
	case FileType::GeometryShader: return Extension_GeometryShader;
	case FileType::Landscape: return Extension_Landscape;
	default:
		assert(false);
		break;
	}

	return "YEET";
}

const std::string& FileType::FileTypeToString(const FileType aFileType)
{
	return GlobalFileTypeStrings[static_cast<int>(aFileType)];
}
