#include "pch.h"
#include "VFXUtilities.h"

#include <Engine/VFX/Flatbuffer/vfxsystem_generated.h>
#include "Engine/Shortcuts.h"
#include "Engine/ResourceManagement/ResourceManager.h"
#include "Engine/ResourceManagement/Resources/VertexShaderResource.h"
#include "Engine/ResourceManagement/Resources/PixelShaderResource.h"
#include "Engine/Renderer/VertexTypes.h"
#include "Engine/ResourceManagement/Resources/GeometryShaderResource.h"
#include "Engine/ResourceManagement/Resources/TextureResource.h"
#include "Engine/Engine.h"
#include "Engine/Renderer/GraphicsEngine.h"
#include "Engine/Renderer/Directx11Framework.h"
#include "Description/ParticleEmitterDescription.h"

bool CreateParticleEmitterRenderData(
	const VFX::ParticleEmitterDescription& aEmitterDesc,
	VFX::ParticleEmitterDescription::ParticleData& aParticleDataOut)
{
	HRESULT result = 0;

	//VertexShaderRef vertexShaderResource =
	//	GResourceManager->CreateRef<Engine::VertexShaderResource>(
	//		"Particles/ParticleVertexShader",
	//		ParticleVertex::ourInputElements,
	//		ParticleVertex::ourElementCount);
	//vertexShaderResource->Load();

	//if (!vertexShaderResource->IsValid())
	//{
	//	LOG_ERROR(LogType::Resource) << "Failed to load particle vertex shader";
	//	return false;
	//}

	//PixelShaderRef pixelShaderResource =
	//	GResourceManager->CreateRef<Engine::PixelShaderResource>(
	//		"Particles/ParticlePixelShader");
	//pixelShaderResource->Load();

	//if (!pixelShaderResource->IsValid())
	//{
	//	LOG_ERROR(LogType::Resource) << "Failed to load particle pixel shader";
	//	return false;
	//}

	GeometryShaderRef geometryShaderResource =
		GResourceManager->CreateRef<Engine::GeometryShaderResource>(
			"Assets/Shaders/Engine/Particles/ParticleGeometryShader");
	geometryShaderResource->Load();

	if (!geometryShaderResource->IsValid())
	{
		LOG_ERROR(LogType::Resource) << "Failed to load particle geometry shader";
		return false;
	}


	TextureRef textureRef =
		GResourceManager->CreateRef<Engine::TextureResource>(
			aEmitterDesc.particleDescription.texturePath);
	textureRef->Load();

	if (!textureRef->IsValid())
	{
		LOG_ERROR(LogType::Resource) << "Failed to load particle geometry shader";
		return false;
	}

	// Vertex buffer
	ID3D11Buffer* vertexBuffer = nullptr;
	{
		D3D11_BUFFER_DESC vertexBufferDesc = {};
		{
			//vertexBufferDesc.ByteWidth = aNumberOfParticles * sizeof(ParticleEmitter::ParticleVertex);
			//const float maxParticleAtAnyGivenTime =
			//	aOutEmitter->GetSettings().myParticleLifetime *
			//	(1.f / aOutEmitter->GetSettings().mySpawnRate) *
			//	sizeof(ParticleVertex);
			
			const int MaxParticles = aEmitterDesc.particleDescription.maxParticles;
			vertexBufferDesc.ByteWidth =
				static_cast<UINT>(MaxParticles * sizeof(ParticleVertex));

			/*
				LifeTimeSec = 2
				SpawnRate = 0.5
				Hur många kan finnas samtidigt: 4

				2 * 1 / 0.5 = 4
			*/

			/*
			vertexBufferDesc.ByteWidth =
				aOutEmitter->GetSettings().myParticleLifetime *
				aOutEmitter->GetSettings().mySpawnRate *
				sizeof(ParticleEmitter::ParticleVertex);
			*/

			vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
			vertexBufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
			vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		}

		auto device = GetEngine().GetGraphicsEngine().GetDxFramework().GetDevice();

		result = device->CreateBuffer(&vertexBufferDesc, nullptr, &vertexBuffer);

		if (FAILED(result))
		{
			return false;
		}
	}

	aParticleDataOut.myStride = sizeof(ParticleVertex);
	aParticleDataOut.myOffset = 0;
	aParticleDataOut.myParticleVertexBuffer = vertexBuffer;
	//aParticleDataOut.myVSShader = vertexShaderResource;
	aParticleDataOut.myGSShader = geometryShaderResource;
	//aParticleDataOut.myPSShader = pixelShaderResource;
	aParticleDataOut.myPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
	aParticleDataOut.myTexture = textureRef;

	return true;
}

void VFX::Save(const VFXDescription& aDescription)
{
	flatbuffers::FlatBufferBuilder builder(1024);

	auto title = builder.CreateString(aDescription.title);

	flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<FBS::VFX::ParticleTimelineKey>>> particleKeys;
	{ // Emitter vector
		auto timelineKeys = aDescription.particleEmitters.GetKeys();

		// //std::vector<FBS::VFX::ParticleTimelineKey> keys;

		std::vector<flatbuffers::Offset<FBS::VFX::ParticleTimelineKey>> particleKeysVector;

		for (int i = 0; i < timelineKeys.size(); i++)
		{
			auto timelineKey = timelineKeys[i];
			auto& emitter = timelineKey.data.base;

			// EVENT KEY BUILDER
			flatbuffers::Offset<FBS::VFX::Key> keyFinished;
			{
				FBS::VFX::KeyBuilder keyBuilder(builder);
				keyBuilder.add_time(timelineKey.time);
				keyBuilder.add_timerandom(timelineKey.timeRandom);

				keyBuilder.add_amount(timelineKey.amount);
				keyBuilder.add_amountrandom(timelineKey.amountRandom);

				keyBuilder.add_repeattimes(timelineKey.repeatTimes);
				keyBuilder.add_repeatdelay(timelineKey.repeatDelay);
				keyBuilder.add_repeatdelayrandom(timelineKey.repeatDelayRandom);
				keyFinished = keyBuilder.Finish();
			}
			// ~EVENT KEY BUILDER


			// GEOMETRY BUILDER
			flatbuffers::Offset<FBS::VFX::EmitterGeometry> finishedGeometry;
			{

				auto geometry = emitter.geometry;
				auto rot = geometry.GetRotation();
				FBS::VFX::Quatf fbsRotation = FBS::VFX::Quatf(rot.myW, rot.myVector.x, rot.myVector.y, rot.myVector.z);

				switch (emitter.geometry.GetShape())
				{
				case eEmitShape::POINT: break;
				case eEmitShape::SPHERE:
				{
					auto sphere = FBS::VFX::CreateSphereProperties(
						builder,
						geometry.properties.sphere.radius,
						geometry.properties.sphere.minRadius
					);
					FBS::VFX::EmitterGeometryBuilder geometryBuilder(builder);

					geometryBuilder.add_properties_type
					(FBS::VFX::GeometryProperties_SphereProperties);
					geometryBuilder.add_properties(sphere.Union());
					geometryBuilder.add_emitdirectiontype(
						static_cast<uint32_t>(geometry.GetEmitDirection()));
					geometryBuilder.add_emitfromtype(
						static_cast<uint32_t>(geometry.GetEmitFrom()));
					geometryBuilder.add_rotation(&fbsRotation);
					finishedGeometry = geometryBuilder.Finish();
				}
				break;
				case eEmitShape::BOX:
				{
					auto halfSize = FBS::VFX::Vec3(geometry.properties.box.halfSize.x, geometry.properties.box.halfSize.y, geometry.properties.box.halfSize.z);

					auto box = FBS::VFX::CreateBoxProperties(
						builder,
						&halfSize
					);
					FBS::VFX::EmitterGeometryBuilder geometryBuilder(builder);

					geometryBuilder.add_properties_type
					(FBS::VFX::GeometryProperties_BoxProperties);
					geometryBuilder.add_properties(box.Union());
					geometryBuilder.add_emitdirectiontype(
						static_cast<uint32_t>(geometry.GetEmitDirection()));
					geometryBuilder.add_emitfromtype(
						static_cast<uint32_t>(geometry.GetEmitFrom()));
					geometryBuilder.add_rotation(&fbsRotation);
					finishedGeometry = geometryBuilder.Finish();
				}
				break;
				case eEmitShape::CONE:
				{
					auto cone = FBS::VFX::CreateConeProperties(
						builder,
						geometry.properties.cone.angle,
						geometry.properties.cone.radius
					);
					FBS::VFX::EmitterGeometryBuilder geometryBuilder(builder);

					geometryBuilder.add_properties_type
					(FBS::VFX::GeometryProperties_ConeProperties);
					geometryBuilder.add_properties(cone.Union());
					geometryBuilder.add_emitdirectiontype(
						static_cast<uint32_t>(geometry.GetEmitDirection()));
					geometryBuilder.add_emitfromtype(
						static_cast<uint32_t>(geometry.GetEmitFrom()));
					geometryBuilder.add_rotation(&fbsRotation);
					finishedGeometry = geometryBuilder.Finish();
				}
				break;
				default:
					break;
				}
			}
			// ~GEOMETRY BUILDER


			// TIMELINE BUILDER
			flatbuffers::Offset<FBS::VFX::Timeline> finishedTimeline;
			{
				auto& timeline = emitter.timeline;
				// KEYS BUILDER 
				flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<FBS::VFX::Key>>>  finishedKeys;
				{
					std::vector<flatbuffers::Offset<FBS::VFX::Key>> keys;
					for (const auto& key : timeline.GetKeys())
					{
						FBS::VFX::KeyBuilder keyBuilder(builder);
						keyBuilder.add_time(key.time);
						keyBuilder.add_timerandom(key.timeRandom);

						keyBuilder.add_amount(key.amount);
						keyBuilder.add_amountrandom(key.amountRandom);

						keyBuilder.add_looping(key.isLooping);
						keyBuilder.add_looptimespersecond(key.timesPerSecond);
						keyBuilder.add_repeattimes(key.repeatTimes);
						keyBuilder.add_repeatdelay(key.repeatDelay);
						keyBuilder.add_repeatdelayrandom(key.repeatDelayRandom);
						keys.emplace_back(keyBuilder.Finish());
					}
					finishedKeys = builder.CreateVector(keys);
				}
				// ~KEYS BUILDER 

				FBS::VFX::TimelineBuilder timelineBuilder(builder);
				timelineBuilder.add_keys(finishedKeys);

				finishedTimeline = timelineBuilder.Finish();
			}
			// ~TIMELINE BUILDER

			// TRANSFORM BUILDER

			// ~TRANSFORM BUILDER
			FBS::VFX::Transform transform = FBS::VFX::Transform(FBS::VFX::Vec3(emitter.position.x, emitter.position.y, emitter.position.z), FBS::VFX::Quatf(emitter.rotation.myVector.x, emitter.rotation.myVector.y, emitter.rotation.myVector.z, emitter.rotation.myW));

			// EMITTER BUILDER
			flatbuffers::Offset<FBS::VFX::Emitter> finishedEmitter;
			{
				FBS::VFX::EmitterBuilder emitterBuilder(builder);
				emitterBuilder.add_geometry(finishedGeometry);
				emitterBuilder.add_duration(emitter.duration);
				emitterBuilder.add_timeline(finishedTimeline);
				emitterBuilder.add_transform(&transform);
				emitterBuilder.add_space(static_cast<unsigned>(emitter.space));
				emitterBuilder.add_keepalive(emitter.keepAlive);

				finishedEmitter = emitterBuilder.Finish();
			}
			// ~EMITTER BUILDER


			// PARTICLE BUILDER
			flatbuffers::Offset<FBS::VFX::Particle> finishedParticle;
			{
				auto& particle = timelineKey.data.particleDescription;

				auto material = builder.CreateString(particle.materialPath);
				auto materialCurve1 = builder.CreateString(particle.materialShaderRefPath1);
				auto materialCurve2 = builder.CreateString(particle.materialShaderRefPath2);
				auto materialCurve3 = builder.CreateString(particle.materialShaderRefPath3);
				auto materialCurve4 = builder.CreateString(particle.materialShaderRefPath4);

				auto texture = builder.CreateString(particle.texturePath);

				auto color = FBS::VFX::Vec4(particle.color.x, particle.color.y, particle.color.z, particle.color.w);
				auto endColor = FBS::VFX::Vec4(particle.endColor.x, particle.endColor.y, particle.endColor.z, particle.endColor.w);

				auto size = FBS::VFX::Vec2(particle.size.x, particle.size.y);
				auto endSize = FBS::VFX::Vec2(particle.endSize.x, particle.endSize.y);
				auto sizemodifier = FBS::VFX::Vec2(particle.sizeModifier.x, particle.sizeModifier.y);
				auto sizeCurve = builder.CreateString(particle.sizeCurve);

				auto accelerationCurve = builder.CreateString(particle.acceleration.curve);

				auto noiseForce = FBS::VFX::Vec3(particle.noise.force.x, particle.noise.force.y, particle.noise.force.z);
				auto noiseScrollSpeed = FBS::VFX::Vec3(particle.noise.scrollSpeed.x, particle.noise.scrollSpeed.y, particle.noise.scrollSpeed.z);

				auto gravity = FBS::VFX::Vec3(particle.gravity.x, particle.gravity.y, particle.gravity.z);

				FBS::VFX::ParticleBuilder particleBuilder(builder);
				particleBuilder.add_startvariables(particle.startVariables);
				particleBuilder.add_updatevariables(particle.updateVariables);
				particleBuilder.add_material(material);
				particleBuilder.add_materialcurve1(materialCurve1);
				particleBuilder.add_materialcurve2(materialCurve2);
				particleBuilder.add_materialcurve3(materialCurve3);
				particleBuilder.add_materialcurve4(materialCurve4);
				particleBuilder.add_texture(texture);
				particleBuilder.add_colorsetting(particle.colorSetting);
				particleBuilder.add_color(&color);
				particleBuilder.add_endcolor(&endColor);
				particleBuilder.add_colorintensity(particle.colorIntensity);

				particleBuilder.add_sizesetting(particle.sizeSetting);
				particleBuilder.add_size(&size);
				particleBuilder.add_endsize(&endSize);
				particleBuilder.add_sizemodifier(&sizemodifier);
				particleBuilder.add_sizecurve(sizeCurve);

				particleBuilder.add_lifetime(particle.lifetime);
				particleBuilder.add_lifetimemod(particle.lifetimeModifier);
				particleBuilder.add_startvelsettings(particle.startVelocity.settings);
				particleBuilder.add_startvelocity(particle.startVelocity.force);
				particleBuilder.add_startvelmodifier(particle.startVelocity.modifier);
				particleBuilder.add_startrotation(particle.startRotation.rotation);
				particleBuilder.add_startrotmodifier(particle.startRotation.modifier);

				particleBuilder.add_gravity(&gravity);
				particleBuilder.add_rotatespeed(particle.rotate.speed);
				particleBuilder.add_rotatemodifier(particle.rotate.modifier);

				particleBuilder.add_dragconstant(particle.drag.drag);
				particleBuilder.add_dragairdensity(particle.drag.airDensity);

				particleBuilder.add_attractsetting(particle.attraction.setting);
				particleBuilder.add_attractforce(particle.attraction.force);
				particleBuilder.add_attractrange(particle.attraction.range);
				particleBuilder.add_attractcollectdist(particle.attraction.collectRange);

				particleBuilder.add_unitsperparticle(particle.emitOnMove.units);
				particleBuilder.add_unitsperpmodifier(particle.emitOnMove.modifier);

				particleBuilder.add_wrapposition(particle.wrapPosition);

				particleBuilder.add_noisesetting(particle.noise.setting);
				particleBuilder.add_noiseforce(&noiseForce);
				particleBuilder.add_noisescrollspeed(&noiseScrollSpeed);
				particleBuilder.add_noisesize(particle.noise.size);

				particleBuilder.add_maxparticles(particle.maxParticles);

				particleBuilder.add_accelerationsetting(particle.acceleration.settings);
				particleBuilder.add_acceleration(particle.acceleration.speed);
				particleBuilder.add_accelerationcurve(accelerationCurve);


				finishedParticle = particleBuilder.Finish();
			}
			// ~PARTICLE BUILDER

			// PARTICLE EMITTER BUILDER
			flatbuffers::Offset<FBS::VFX::ParticleEmitter> finishedParticleEmitter;
			{
				FBS::VFX::ParticleEmitterBuilder particleEmitterBuilder(builder);
				particleEmitterBuilder.add_emitter(finishedEmitter);
				particleEmitterBuilder.add_particle(finishedParticle);
				finishedParticleEmitter = particleEmitterBuilder.Finish();
			}
			// PARTICLE EMITTER BUILDER


			FBS::VFX::ParticleTimelineKeyBuilder timelineBuilder(builder);
			timelineBuilder.add_particleemitter(finishedParticleEmitter);
			timelineBuilder.add_key(keyFinished);

			//keys.emplace_back(timelineBuilder.Finish());
			particleKeysVector.emplace_back(timelineBuilder.Finish());
		}

		particleKeys = builder.CreateVector(particleKeysVector);
	}

	FBS::VFX::VFXDescriptionBuilder vfxBuilder(builder);
	vfxBuilder.add_title(title);
	vfxBuilder.add_space(static_cast<unsigned>(aDescription.space));
	vfxBuilder.add_duration(aDescription.duration);
	vfxBuilder.add_looping(aDescription.looping);
	vfxBuilder.add_particleemitters(particleKeys);

	builder.Finish(vfxBuilder.Finish());

	// This must be called after `Finish()`
	uint8_t* buf = builder.GetBufferPointer();
	unsigned size = builder.GetSize();

	// Save buf to file
	std::ofstream file;
	std::string path = "Assets/VFX/" + aDescription.title;
	path.append(".vfx");

	file.open(path, std::ios::out | std::ios::trunc);
	file.write(reinterpret_cast<char*>(buf), size);
	file.close();
}

VFX::VFXDescription VFX::Load(const std::string& aFilePath, bool* aSucceeded)
{
	VFX::VFXDescription description;
	std::ifstream file(aFilePath);
	if (!file.is_open())
	{
		std::cout << "________________\n\nERROR: Couldn't open vfx save file: " << aFilePath << std::endl;
		//assert(false && "Coudlnt open vfx save file!");

		*aSucceeded = false;

		return description;
	}

	file.seekg(0, std::ios::end);
	const int size = file.tellg();
	file.seekg(0, std::ios::beg);

	char* buffer = DBG_NEW char[size];
	file.read(buffer, size);
	file.close();

	// Get a pointer to the root object inside the buffer.
	auto fbsDescription = FBS::VFX::GetVFXDescription(buffer);

	description.title = fbsDescription->title()->c_str();
	description.space = static_cast<eSpace>(fbsDescription->space());
	description.duration = fbsDescription->duration();
	description.looping = fbsDescription->looping();

	auto fbsParticleEmitterTimelineKeys = fbsDescription->particleemitters();
	for (int i = 0; i < fbsParticleEmitterTimelineKeys->size(); i++)
	{
		const auto* fbsParticleKey = static_cast<const FBS::VFX::ParticleTimelineKey*>(fbsParticleEmitterTimelineKeys->Get(i));

		const auto fbsKey = fbsParticleKey->key();
		const auto fbsParticleEmitter = fbsParticleKey->particleemitter();

		VFX::ParticleEmitterDescription particleEmitterDescription;
		{
			auto& particleDesc = particleEmitterDescription.particleDescription;
			{
				const auto fbsParticle = fbsParticleEmitter->particle();

				particleDesc.startVariables = fbsParticle->startvariables();
				particleDesc.updateVariables = fbsParticle->updatevariables();

				particleDesc.maxParticles = fbsParticle->maxparticles();

				auto material = fbsParticle->material();
				if (material)
					particleDesc.materialPath = material->c_str();

				auto texture = fbsParticle->texture();
				if (texture)
					particleDesc.texturePath = texture->c_str();

				const auto color = fbsParticle->color();
				const auto endColor = fbsParticle->endcolor();
				particleDesc.colorSetting = fbsParticle->colorsetting();
				if (color)
					particleDesc.color = { color->x(), color->y(), color->z(), color->w() };
				if (endColor)
					particleDesc.endColor = { endColor->x(), endColor->y(), endColor->z(), endColor->w() };
				particleDesc.colorIntensity = fbsParticle->colorintensity();

				const auto fbsSize = fbsParticle->size();
				const auto fbsEndSize = fbsParticle->endsize();
				const auto fbsSizeModifier = fbsParticle->sizemodifier();
				particleDesc.sizeSetting = fbsParticle->sizesetting();
				if (fbsSize)
					particleDesc.size = { fbsSize->x(), fbsSize->y() };
				if (fbsSizeModifier)
					particleDesc.sizeModifier = { fbsSizeModifier->x(), fbsSizeModifier->y() };
				if (fbsEndSize)
					particleDesc.endSize = { fbsEndSize->x(), fbsEndSize->y() };

				const auto fbsSizeCurve = fbsParticle->sizecurve();
				if (fbsSizeCurve)
					particleDesc.sizeCurve = fbsSizeCurve->c_str();

				particleDesc.acceleration.settings = fbsParticle->accelerationsetting();
				particleDesc.acceleration.speed = fbsParticle->acceleration();
				const auto fbsAccelerationCurve = fbsParticle->accelerationcurve();
				if (fbsAccelerationCurve)
					particleDesc.acceleration.curve = fbsAccelerationCurve->c_str();

				particleDesc.lifetime = fbsParticle->lifetime();
				particleDesc.lifetimeModifier = fbsParticle->lifetimemod();

				particleDesc.startVelocity.settings = fbsParticle->startvelsettings();
				particleDesc.startVelocity.force = fbsParticle->startvelocity();
				particleDesc.startVelocity.modifier = fbsParticle->startvelmodifier();

				particleDesc.startRotation.rotation = fbsParticle->startrotation();
				particleDesc.startRotation.modifier = fbsParticle->startrotmodifier();

				const auto gravity = fbsParticle->gravity();
				if (gravity)
					particleDesc.gravity = Vec3f(gravity->x(), gravity->y(), gravity->z());
				particleDesc.rotate.speed = fbsParticle->rotatespeed();
				particleDesc.rotate.modifier = fbsParticle->rotatemodifier();

				particleDesc.drag.drag = fbsParticle->dragconstant();
				particleDesc.drag.airDensity = fbsParticle->dragairdensity();

				particleDesc.attraction.setting = static_cast<eAttractionSetting>(fbsParticle->attractsetting());
				particleDesc.attraction.force = fbsParticle->attractforce();
				particleDesc.attraction.range = fbsParticle->attractrange();
				particleDesc.attraction.collectRange = fbsParticle->attractcollectdist();

				particleDesc.emitOnMove.units = fbsParticle->unitsperparticle();
				particleDesc.emitOnMove.modifier = fbsParticle->unitsperpmodifier();

				particleDesc.wrapPosition = fbsParticle->wrapposition();

				const auto noiseforce = fbsParticle->noiseforce();
				const auto noisescrollspeed = fbsParticle->noisescrollspeed();
				if (noiseforce)
					particleDesc.noise.force = Vec3f(noiseforce->x(), noiseforce->y(), noiseforce->z());
				if (noisescrollspeed)
					particleDesc.noise.scrollSpeed = Vec3f(noisescrollspeed->x(), noisescrollspeed->y(), noisescrollspeed->z());

				particleDesc.noise.setting = static_cast<eNoiseType>(fbsParticle->noisesetting());
				particleDesc.noise.size = fbsParticle->noisesize();


				auto materialShaderRefPath1 = fbsParticle->materialcurve1();
				if (materialShaderRefPath1)
					particleDesc.materialShaderRefPath1 = materialShaderRefPath1->c_str();

				auto materialShaderRefPath2 = fbsParticle->materialcurve2();
				if (materialShaderRefPath2)
					particleDesc.materialShaderRefPath2 = materialShaderRefPath2->c_str();

				auto materialShaderRefPath3 = fbsParticle->materialcurve3();
				if (materialShaderRefPath3)
					particleDesc.materialShaderRefPath3 = materialShaderRefPath3->c_str();

				auto materialShaderRefPath4 = fbsParticle->materialcurve4();
				if (materialShaderRefPath4)
					particleDesc.materialShaderRefPath4 = materialShaderRefPath4->c_str();
			}

			auto& emitterDesc = particleEmitterDescription.base;
			{
				const auto fbsEmitter = fbsParticleEmitter->emitter();
				{
					const auto fbsGeometry = fbsEmitter->geometry();
					eEmitFrom from = static_cast<eEmitFrom>(fbsGeometry->emitfromtype());
					eEmitDirection direction = static_cast<eEmitDirection>(fbsGeometry->emitdirectiontype());

					const auto fbsRotation = fbsGeometry->rotation();
					Quatf rotation = Quatf(fbsRotation->w(), { fbsRotation->x(), fbsRotation->y(), fbsRotation->z() });
					//const auto fbsGeometryProperty;
					switch (fbsGeometry->properties_type())
					{
					case FBS::VFX::GeometryProperties_NONE:
						break;
					case FBS::VFX::GeometryProperties_ConeProperties:
					{
						const auto cone = fbsGeometry->properties_as_ConeProperties();

						emitterDesc.geometry = EmitterGeometry::Cone(
							cone->radius(),
							cone->angle(),
							from,
							direction
						);
					}
					break;
					case FBS::VFX::GeometryProperties_SphereProperties:
					{
						const auto sphere = fbsGeometry->properties_as_SphereProperties();

						emitterDesc.geometry = EmitterGeometry::Sphere(
							sphere->radius(),
							sphere->minradius(),
							from,
							direction
						);
					}
					break;
					case FBS::VFX::GeometryProperties_BoxProperties:
					{
						const auto box = fbsGeometry->properties_as_BoxProperties();

						emitterDesc.geometry = EmitterGeometry::Box(
							{ box->boxhalfsize()->x(), box->boxhalfsize()->y(), box->boxhalfsize()->z() },
							from,
							direction
						);
					}
					break;
					default:;
					}
					emitterDesc.geometry.SetRotation(rotation);

				}

				emitterDesc.duration = fbsEmitter->duration();
				emitterDesc.space = static_cast<eSpace>(fbsEmitter->space());
				emitterDesc.keepAlive = fbsEmitter->keepalive();

				const auto fbstransform = fbsEmitter->transform();
				if (fbstransform) 
				{
					const auto& fbspos = fbsEmitter->transform()->position();
					const auto& fbsrot = fbsEmitter->transform()->rotation();

					emitterDesc.position = Vec3f(fbspos.x(), fbspos.y(), fbspos.z());
					emitterDesc.rotation = Quatf(fbsrot.w(), Vec3f(fbsrot.x(), fbsrot.y(), fbsrot.z()));
				}

				// Timeline
				{
					auto& timeline = emitterDesc.timeline;

					const auto fbsTimeline = fbsEmitter->timeline();
					const auto fbsKeys = fbsTimeline->keys();

					for (int k = 0; k < fbsKeys->size(); k++)
					{
						const auto fbsKeyNew = fbsKeys->Get(k);
						timeline.Add(Timeline::Key::Create(fbsKeyNew->time(), fbsKeyNew->timerandom(), fbsKeyNew->amount(), fbsKeyNew->amountrandom(), fbsKeyNew->repeattimes(), fbsKeyNew->repeatdelay(), fbsKeyNew->repeatdelayrandom(), fbsKeyNew->looping(), fbsKeyNew->looptimespersecond()));
					}
				}


			}
		}

		Timeline::Key key = Timeline::Key::Create(fbsKey->time(), fbsKey->timerandom(), fbsKey->amount(), fbsKey->amountrandom(), fbsKey->repeattimes(), fbsKey->repeatdelay(), fbsKey->repeatdelayrandom(), fbsKey->looping(), fbsKey->looptimespersecond());

		if (!CreateParticleEmitterRenderData(particleEmitterDescription, particleEmitterDescription.myRenderData))
		{
			LOG_ERROR(LogType::Resource) << "Failed to create render data for emitter";
			if (aSucceeded)
			{
				*aSucceeded = false;
			}
		}

		description.particleEmitters.Add(particleEmitterDescription, key);
	}
	delete[] buffer;

	if (aSucceeded)
	{
		*aSucceeded = true;
	}

	return description;
}