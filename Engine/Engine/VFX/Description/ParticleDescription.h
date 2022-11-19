#pragma once
#include "Common/AnimationCurve.h"

namespace VFX
{
	enum ePropertySetting
	{
		eSTATIC = 0,
		eLERP = (1 << 0),
		eMODIFIER = (1 << 1),

		PROPERTY_SETTINGS_SIZE = (1 << 2)
	};

	enum eAttractionSetting
	{
		eEmitterLocal,
		eGlobalPoint,
	};

	enum eNoiseType
	{
		SOFT,
		HARD
	};

	struct ParticleDescription
	{
		unsigned	startVariables	= 0;
		unsigned	updateVariables	= 0;


		// Start variables
		std::string	texturePath = "Assets/Sprites/VFX/Default_Particle.dds";

		unsigned	colorSetting = 0;
		Vec4f		color = { 1, 1, 1, 1 };
		Vec4f		endColor = { 1, 1, 1, 1};
		float		colorIntensity = 1;


		int			maxParticles = 500;
		
		unsigned	sizeSetting = 0;
		Vec2f		size = { 100, 100 };
		Vec2f		endSize = { 100, 100 };
		Vec2f		sizeModifier = { 0, 0 };

		std::string	sizeCurve;

		float		lifetime = 1.f;
		float		lifetimeModifier = 0;

		struct {
			unsigned	settings = 0;
			float		force = 0.f;
			float		modifier = 0.f;
		} startVelocity;

		struct {
			unsigned	settings = 0;
			float speed = 100;
			std::string	curve;
		} acceleration;

		struct {
			float drag = 1;
			float airDensity = 10000;
		} drag;

		struct {
			float rotation = 0;
			float modifier = 0;
		} startRotation;

		// Update variables
		bool		wrapPosition = false;
		Vec3f		gravity = { 0, 0, 0 };

		struct
		{
			float speed = 0;
			float modifier = 0;
		} rotate;

		struct
		{
			eAttractionSetting setting = eAttractionSetting::eEmitterLocal;
			float force = 100;
			float range = 100;
			float collectRange = 10;
		} attraction;

		struct
		{
			float units = 100;
			float modifier = 0;
		} emitOnMove;

		struct
		{
			Vec3f force = { 100, 100, 100 };
			Vec3f scrollSpeed = { 1, 1.2f, -1 };
			float size = 10.f;
			eNoiseType setting = eNoiseType::HARD;

		} noise;

		std::string materialPath = "Assets/Engine/Materials/Default.particlemat";

		std::string	materialShaderRefPath1;
		std::string	materialShaderRefPath2;
		std::string	materialShaderRefPath3;
		std::string	materialShaderRefPath4;
	};

	namespace ParticleStartVariables
	{
		const static inline char* titles[] = {
				"Lifetime",
				"Color",
				"Size",
				"Rotation",
				"Max Particles",
				"Velocity",
				"Texture",
				"Material",
		};

		enum eVariables
		{
			LIFETIME	= (1 << 0),
			COLOR		= (1 << 1),
			SIZE		= (1 << 2),
			ROTATION	= (1 << 3),
			MAXPARTICLES= (1 << 4),
			VELOCITY	= (1 << 5),
			TEXTURE		= (1 << 6),
			MATERIAL	= (1 << 7),
		};
	};
	
	namespace ParticleUpdateVariables
	{
		const static inline char* titles[] = {
				"Gravity",
				"Velocity",
				"Drag",
				"Rotate",
				"Attraction",
				"Move Emit",
				"Noise",
				"Wrap Position",
				"Acceleration",
		};

		enum eVariables
		{
			GRAVITY			= (1 << 0),
			ADD_FORCE		= (1 << 1),
			DRAG			= (1 << 2),
			ROTATE			= (1 << 3),
			ATTRACT			= (1 << 4),
			EMIT_ON_MOVE	= (1 << 5),
			NOISE			= (1 << 6),
			WRAP			= (1 << 7),
			ACCELERATION	= (1 << 8),
		};
	};

}
