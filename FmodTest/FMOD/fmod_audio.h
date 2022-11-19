
#pragma once
#include <map>
#include <string>
#include <vector>
#include "Common/Math/Math.h"
#include "../FMOD/fmod_studio.hpp"
#include "../FMOD/fmod.h"
#include "../FMOD/fmod_errors.h"


using namespace FMOD;

struct audio_object
{
	Studio::EventDescription* eventDescription;
	Studio::EventInstance* eventInstance;
	std::map<std::string, Studio::EventInstance*> eventInstances;
	FMOD_3D_ATTRIBUTES attributes;
	std::string id;
	std::string path;
};

class fmod_audio
{
public:

	bool init_audio();

	void set_sound_reach(float aSoundReach);

	void set_listener(CU::Vector3f aPos, CU::Vector3f aForward, CU::Vector3f aUp);

	void set_listener();

	void set_main_volume(float aVolume);

	void set_sfx_volume(float aVolume);

	void set_music_volume(float aVolume);

	void set_ambiance_volume(float aVolume);

	void set_reverb_value(float aValue);

	float& get_music_volume();

	float& get_sfx_volume();

	void update_audio();

	bool is_instance_loaded(const float& aUUID, const std::string& aPath);

	void load_event(const float& aUUID, const std::string& aPath);
	//STOPS PROGRAM UNTIL DONE LOADING (FLAG NORMAL)
	void load_bank(const char* filename);

	void play_event(std::string anEventPath, const int aUUID, float aParameterValue, const std::string& aParameterName);

	void play_event(std::string anEventPath, int aUUID);

	void delete_object(const int aUUID);

	void play_event(std::string anEventPath, int aUUID, const CU::Vector3f& pos, const CU::Vector3f& forward, const CU::Vector3f& up/*, std::string audioGroup*/);

	void set_parameter(std::string aParameterName, float aValue);

	void set_parameter(int aUUID, std::string aEventPath, std::string aParameterName, float aValue);

	void set_parameter(std::string aParameterName, bool aValue);

	void stop_events(int aUUID);

	void stop_event(std::string aEvent, int aUUID);

	void stop_all();

	void set_pause_event(std::string aEventPath, bool aPaused);

	void set_pause_all(bool aPaused);

	void unload_bank();

	float get_sound_length(std::string anEventPath);

	//IMPORTATNT TO STOP AUDIO BEFORE CLOSEING APPLICATION FOR SOME REASON
	void stop_audio();
private:
	const int MAX_CHANNELS = 128;
	int numChannels = 0;

	float musicVolume = 1.0f;
	float sFXVolume = 1.0f;
	//FMOD::System* fmod_studio_system;

	System* fmod_system;

	Studio::System* fmod_studio_system;
	Studio::Bank* audio_bank = nullptr;

	std::map<int, audio_object> audio_objects;

	FMOD_3D_ATTRIBUTES listenerAttributes;

	float sound_reach = 0.f;

	std::map<std::string, Studio::EventDescription*> eventDescriptions;
	std::map<std::string, Studio::EventInstance*> eventInstances;

	FMOD::Studio::VCA* myMainVca = nullptr;
	FMOD::Studio::VCA* myMusicVca = nullptr;
	FMOD::Studio::VCA* myAmbianceVca = nullptr;
	FMOD::Studio::VCA* mySFXVca = nullptr;

	Studio::EventDescription** eventDescriptionArray;
	std::map<std::string, Studio::EventDescription*> eventDescriptionMap;
	int eventDescriptionCount = 0;

};