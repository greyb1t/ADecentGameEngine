#include "fmod_audio.h"
#include "iostream"
#include "Common/Math/Math.h"
#include "Common/Log.h"
#include "fmod_errors.h"

bool fmod_audio::init_audio()
{
	FMOD_RESULT result;
	//Debug_Initialize(FMOD_DEBUG_TYPE_TRACE);
	result = Studio::System::create(&fmod_studio_system, FMOD_VERSION); //Create studio system
	if (result != FMOD_OK)
	{
		LOG_ERROR(LogType::Audio) << "FMOD error! " << result << " " << FMOD_ErrorString(result);
		unsigned int version;
		fmod_system->getVersion(&version);
		LOG_ERROR(LogType::Audio) << "FMOD VERSION: " << version;
		return false;
	}

	// Initialize FMOD Studio, which will also initialize FMOD Core
	fmod_studio_system->initialize(MAX_CHANNELS, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, nullptr);
	if (result != FMOD_OK)
	{
		LOG_ERROR(LogType::Audio) << "FMOD error! " << result << " " << FMOD_ErrorString(result);

		return false;
	}

	result = fmod_studio_system->getCoreSystem(&fmod_system);
	if (result != FMOD_OK)
	{
		LOG_ERROR(LogType::Audio) << "FMOD error! " << result << " " << FMOD_ErrorString(result);

		return false;
	}

	fmod_system->setDSPBufferSize(64, 16);

	std::cout << "!! FMOD IS GOOD TO GO !!" << std::endl;
	return true;
}

void fmod_audio::set_sound_reach(float aSoundReach)
{
	sound_reach = aSoundReach;
}

void fmod_audio::set_listener(CU::Vector3f aPos, CU::Vector3f aForward, CU::Vector3f aUp)
{
	FMOD_3D_ATTRIBUTES a;
	a.position.x = aPos.x / sound_reach;
	a.position.y = aPos.y / sound_reach;
	a.position.z = aPos.z / sound_reach;
	a.forward.x = aForward.x;
	a.forward.y = aForward.y;
	a.forward.z = aForward.z;
	a.velocity.x = 0.f;
	a.velocity.y = 0.f;
	a.velocity.z = 0.f;
	a.up.x = aUp.x;
	a.up.y = aUp.y;
	a.up.z = aUp.z;
	listenerAttributes = a;
	fmod_studio_system->setListenerAttributes(0, &a, &a.position); // TODO: Attenuation position
}
void fmod_audio::set_listener()
{
	fmod_studio_system->setListenerAttributes(0, &listenerAttributes, &listenerAttributes.position); // TODO: Attenuation position
}
void fmod_audio::set_main_volume(float aVolume)
{
	myMainVca->setVolume(aVolume);
}
void fmod_audio::set_music_volume(float aVolume)
{
	myMusicVca->setVolume(aVolume);
}
void fmod_audio::set_ambiance_volume(float aVolume)
{
	myAmbianceVca->setVolume(aVolume);
}
void fmod_audio::set_sfx_volume(float aVolume)
{
	mySFXVca->setVolume(aVolume);
}
void fmod_audio::set_reverb_value(float aValue)
{
	fmod_system->getChannelsPlaying(&numChannels);
	std::vector<Channel*> channels;

	for (int i = 0; i < numChannels; i++)
	{
		fmod_system->getChannel(i, &channels[i]);
		//channels[i]->setReverbProperties(FMOD_PRESET_BATHROOM);
	}
}
void fmod_audio::load_event(const float& aUUID, const std::string& aPath)
{

}
float& fmod_audio::get_music_volume()
{
	return musicVolume;
}

float& fmod_audio::get_sfx_volume()
{
	return sFXVolume;
}

void fmod_audio::load_bank(const char* filename)
{
	FMOD_RESULT result;
	std::cout << "LOADING BANK... (FMOD)" << std::endl;
	std::string string = "Assets/Audio/Master.bank";
	filename = string.c_str();
	result = fmod_studio_system->loadBankFile(filename, FMOD_STUDIO_LOAD_BANK_NORMAL, &audio_bank);
	if (result != FMOD_OK)
	{
		LOG_ERROR(LogType::Audio) << "FMOD error! " << result << " " << FMOD_ErrorString(result);

		return;
	}

	eventDescriptionArray = new Studio::EventDescription * [1024];

	result = audio_bank->getEventList(eventDescriptionArray, 1024, &eventDescriptionCount);
	if (result != FMOD_OK)
	{
		LOG_ERROR(LogType::Audio) << "FMOD error! " << result << " " << FMOD_ErrorString(result);
	}

	for (int i = 0; i < eventDescriptionCount; i++)
	{
		int length;
		char path;
		//eventDescriptionMap.insert(std::pair(eventDescriptionArray[i]->getPath(&path, 564), eventDescriptionArray));
	}

	LOG_INFO(LogType::Audio) << "Loaded " << std::to_string(eventDescriptionCount) << " event descriptions " << " (FMOD)";

	string = "Assets/Audio/Master.strings.bank";
	filename = string.c_str();
	result = fmod_studio_system->loadBankFile(filename, FMOD_STUDIO_LOAD_BANK_NORMAL, &audio_bank);
	if (result != FMOD_OK)
	{
		LOG_ERROR(LogType::Audio) << "FMOD error! " << result << " " << FMOD_ErrorString(result);

		return;
	}

	// LOAD VCAs (In studio "Sound Groups")
	result = fmod_studio_system->getVCA("vca:/MainVolume", &myMainVca);
	if (result != FMOD_OK)
	{
		LOG_ERROR(LogType::Audio) << "FMOD error! " << result << " " << FMOD_ErrorString(result);
	}
	result = fmod_studio_system->getVCA("vca:/MusicVolume", &myMusicVca);
	if (result != FMOD_OK)
	{
		LOG_ERROR(LogType::Audio) << "FMOD error! " << result << " " << FMOD_ErrorString(result);
	}
	result = fmod_studio_system->getVCA("vca:/AMBVolume", &myAmbianceVca);
	if (result != FMOD_OK)
	{
		LOG_ERROR(LogType::Audio) << "FMOD error! " << result << " " << FMOD_ErrorString(result);
	}
	result = fmod_studio_system->getVCA("vca:/SFXVolume", &mySFXVca);
	if (result != FMOD_OK)
	{
		LOG_ERROR(LogType::Audio) << "FMOD error! " << result << " " << FMOD_ErrorString(result);
	}

	audio_bank->loadSampleData();
	LOG_INFO(LogType::Audio) << "DONE LOADING BANK!!! (FMOD)";
}

void fmod_audio::play_event(std::string anEventPath, const int aUUID, float aParameterValue, const std::string& aParameterName)
{
	FMOD_RESULT result;
	FMOD_STUDIO_LOADING_STATE state;
	const char* filename;
	result = audio_bank->getLoadingState(&state);
	filename = anEventPath.c_str();

	result = fmod_studio_system->getEvent(filename, &audio_objects[aUUID].eventDescription);
	if (result != FMOD_OK)
	{
		LOG_ERROR(LogType::Audio) << "FMOD error!!" << result << FMOD_ErrorString(result);
		return;
	}

	audio_objects[aUUID].eventDescription->loadSampleData();
	audio_objects[aUUID].eventDescription->createInstance(&audio_objects[aUUID].eventInstance);

	audio_objects[aUUID].attributes.position = listenerAttributes.position;
	audio_objects[aUUID].attributes.velocity = { 0,0,0 };
	audio_objects[aUUID].attributes.forward = { 0.33f,0.33f, 0.33f };
	audio_objects[aUUID].attributes.up = { 0.33f ,0.33f, 0.33f };

	audio_objects[aUUID].eventInstance->set3DAttributes(&audio_objects[aUUID].attributes);

	audio_objects[aUUID].eventInstance->setParameterByName(aParameterName.c_str(), aParameterValue);

	audio_objects[aUUID].eventInstance->start();
}

void fmod_audio::play_event(std::string anEventPath, const int aUUID)
{
	FMOD_RESULT result;
	FMOD_STUDIO_LOADING_STATE state;
	const char* filename;
	result = audio_bank->getLoadingState(&state);
	filename = anEventPath.c_str();


	result = fmod_studio_system->getEvent(filename, &audio_objects[aUUID].eventDescription);
	if (result != FMOD_OK)
	{
		LOG_ERROR(LogType::Audio) << "FMOD error!!" << result << FMOD_ErrorString(result);
		return;
	}

	audio_objects[aUUID].eventDescription->loadSampleData();
	audio_objects[aUUID].eventDescription->createInstance(&audio_objects[aUUID].eventInstance);

	audio_objects[aUUID].attributes.position = listenerAttributes.position;
	audio_objects[aUUID].attributes.velocity = { 0,0,0 };
	audio_objects[aUUID].attributes.forward = { 0.33f,0.33f, 0.33f };
	audio_objects[aUUID].attributes.up = { 0.33f ,0.33f, 0.33f };

	audio_objects[aUUID].eventInstance->set3DAttributes(&audio_objects[aUUID].attributes);
	audio_objects[aUUID].eventInstance->start();
}
void fmod_audio::delete_object(const int aUUID)
{
	audio_objects[aUUID].eventDescription->releaseAllInstances();
	audio_objects.erase(aUUID);
}
void fmod_audio::play_event(std::string anEventPath, const int aUUID, const CU::Vector3f& pos, const CU::Vector3f& forward, const CU::Vector3f& up/*, std::string audioGroup*/)
{
	FMOD_RESULT result;
	FMOD_STUDIO_LOADING_STATE state;
	const char* filename;
	result = audio_bank->getLoadingState(&state);
	filename = anEventPath.c_str();

	result = fmod_studio_system->getEvent(filename, &audio_objects[aUUID].eventDescription);
	if (result != FMOD_OK)
	{
#ifdef _DEBUG
		LOG_ERROR(LogType::Audio) << "FMOD error!!" << result << FMOD_ErrorString(result);
#endif // _DEBUG
		return;
	}

	//audio_objects[aUUID].eventDescription->loadSampleData();
	audio_objects[aUUID].eventDescription->createInstance(&audio_objects[aUUID].eventInstances[anEventPath]);

	audio_objects[aUUID].attributes.position.x = pos.x / sound_reach;
	audio_objects[aUUID].attributes.position.y = pos.y / sound_reach;
	audio_objects[aUUID].attributes.position.z = pos.z / sound_reach;
	audio_objects[aUUID].attributes.velocity = { 0,0,0 };
	audio_objects[aUUID].attributes.forward = { forward.x, forward.y, forward.z };
	audio_objects[aUUID].attributes.up = { up.x, up.y, up.z };

	audio_objects[aUUID].eventInstances[anEventPath]->set3DAttributes(&audio_objects[aUUID].attributes);
	result = audio_objects[aUUID].eventInstances[anEventPath]->start();
	if (result != FMOD_OK)
	{
#ifdef _DEBUG
		LOG_ERROR(LogType::Audio) << "FMOD error!!" << result << FMOD_ErrorString(result);
#endif // _DEBUG
		return;
	}
	audio_objects[aUUID].eventInstances[anEventPath]->release();

	int count = 0;
	int totalInstanceCount = 0;
	int totalAudioObjects = 0;
	for (auto object : audio_objects)
	{
		totalAudioObjects++;
		object.second.eventDescription->getInstanceCount(&count);
		totalInstanceCount += count;
	}

	LOG_WARNING(LogType::Audio) << "Path: " << anEventPath;
	LOG_WARNING(LogType::Audio) << "Instances:" << totalInstanceCount;
	LOG_WARNING(LogType::Audio) << "Audio Objects" << totalAudioObjects;
}

void fmod_audio::set_parameter(std::string aParameterName, float aValue)
{
	FMOD_RESULT result = fmod_studio_system->setParameterByName(aParameterName.c_str(), aValue);
	if (result != FMOD_RESULT::FMOD_OK)
	{
#ifdef _DEBUG
		LOG_ERROR(LogType::Audio) << "FMOD error!!" << result << FMOD_ErrorString(result);
#endif // _DEBUG
	}
}
void fmod_audio::set_parameter(int aUUID, std::string aEventPath, std::string aParameterName, float aValue)
{
	FMOD_RESULT result = audio_objects[aUUID].eventInstances[aEventPath]->setParameterByName(aParameterName.c_str(), aValue);

	if (result != FMOD_RESULT::FMOD_OK)
	{
#ifdef _DEBUG
		LOG_ERROR(LogType::Audio) << "FMOD error!!" << result << FMOD_ErrorString(result);
#endif // _DEBUG
	}
}
void fmod_audio::set_parameter(std::string aParameterName, bool aValue)
{
	fmod_studio_system->setParameterByName(aParameterName.c_str(), aValue);
}
void fmod_audio::stop_events(const int aUUID)
{
	for (auto& object : audio_objects[aUUID].eventInstances)
	{
		object.second->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT);
	}
	audio_objects[aUUID].eventInstances.clear();
}
void fmod_audio::stop_event(std::string aEventPath, const int aUUID)
{
	audio_objects[aUUID].eventInstance->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT);
	audio_objects[aUUID].eventInstances[aEventPath]->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT);
	audio_objects[aUUID].eventInstances.erase(aEventPath);
}
void fmod_audio::stop_all()
{
	for (auto& object : audio_objects)
	{
		audio_objects[object.first].eventInstance->stop(FMOD_STUDIO_STOP_MODE::FMOD_STUDIO_STOP_IMMEDIATE);
	}
	for (auto& object : audio_objects)
	{
		for (auto& instance : object.second.eventInstances)
		{
			instance.second->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT);
		}
	}
}
void fmod_audio::set_pause_event(std::string aEventPath, bool aPaused)
{
	for (auto& object : audio_objects)
	{
		if (object.second.path == aEventPath)
			audio_objects[object.first].eventInstance->setPaused(aPaused);
	}
}
void fmod_audio::set_pause_all(bool aPaused)
{
	for (auto& object : audio_objects)
	{
		audio_objects[object.first].eventInstance->setPaused(aPaused);
	}
}

void fmod_audio::unload_bank()
{
	if (audio_bank != nullptr)
		audio_bank->unload();
	audio_bank = nullptr;
}

void fmod_audio::update_audio()
{
	set_listener();
	/*for (auto e : audio_objects)
	{
		e.second.eventInstance->set3DAttributes(&e.second.attributes);
	}*/
	fmod_studio_system->update();
	// TODO: Update 3dattributes every frame somehow
}

bool fmod_audio::is_instance_loaded(const float& aUUID, const std::string& aPath)
{
	FMOD_STUDIO_LOADING_STATE state;
	FMOD_RESULT result = audio_objects[aUUID].eventDescription->getSampleLoadingState(&state);
	if (result != FMOD_RESULT::FMOD_OK)
	{
#ifdef _DEBUG
		LOG_ERROR(LogType::Audio) << "FMOD error!!" << result << FMOD_ErrorString(result);
#endif // _DEBUG
	}

	if (state == FMOD_STUDIO_LOADING_STATE::FMOD_STUDIO_LOADING_STATE_LOADED)
	{
		return true;
	}
	return false;
}

float fmod_audio::get_sound_length(std::string anEventPath)
{
	// Hello kind programmer. I found a memory leak here. you typed int* length = new int() and then returned a dereferenced copy
	int length;
	Studio::EventDescription* eventDesc;
	fmod_studio_system->getEvent(anEventPath.c_str(), &eventDesc);
	eventDesc->getLength(&length);
	return length;
}

void fmod_audio::stop_audio()
{
	fmod_studio_system->release();
}