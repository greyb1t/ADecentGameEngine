#include "pch.h"
#include "AudioManager.h"

#include "DebugManager/DebugMenu.h"
#include "Engine.h"
#include "Engine/GameObject/Transform.h"
#include "Engine/Scene/SceneManager.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/Scene/Scene.h"

#include "../FmodTest/FMOD/fmod_audio.h"

static AudioManager* ourInstance = nullptr;

AudioManager* AudioManager::GetInstance()
{
	if (!ourInstance)
	{
		ourInstance = DBG_NEW AudioManager();
		// SET TO 100 FOR FINAL RELEASE
		ourInstance->Init(100.0f);
		return ourInstance;
	}
	return ourInstance;
}

AudioManager::AudioManager()
{
}

AudioManager::~AudioManager()
{
}

void AudioManager::Init(float aSoundReach)
{
	fmod = MakeOwned<fmod_audio>();

	fmod->init_audio();
	fmod->set_sound_reach(aSoundReach);
	fmod->load_bank("");
}
void AudioManager::SetPauseAll(bool aPaused)
{
	fmod->set_pause_all(aPaused);
}
void AudioManager::SetPauseEvent(const std::string& aEventPath, bool aPaused)
{
	fmod->set_pause_event(aEventPath, aPaused);
}
void AudioManager::ReleaseAudio(const int aUUID)
{
	fmod->delete_object(aUUID);
}
void AudioManager::LoadEvent(const int& aUUID, const std::string& aPath)
{
	fmod->load_event(aUUID, aPath);
}
bool AudioManager::IsInstanceLoaded(const int& aUUID, const std::string& aPath)
{
	return fmod->is_instance_loaded(aUUID, aPath);
}
void AudioManager::SetMusicIntensity(const float& aMusicIntensity)
{
	myMusicIntensity = aMusicIntensity;
}
void AudioManager::Update()
{
	ZoneScopedN("AudioManager::Update");

	JsonUpdate();

	if (GetEngine().GetSceneManager().GetMainScene())
	{
		if (GetEngine().GetSceneManager().GetSceneBeingUpdatedNow() != nullptr)
		{
			if (auto g = myPlayer.lock())
			{
				SetListener(g->GetTransform());
			}
		}
	}

	//Modifying music intensity layering (0-100, musicintensity)
	if (myOldIntensity != myMusicIntensity)
	{
		assert(myMusicIntensity <= 1 && myMusicIntensity >= 0, "Music intensity value out of range");
		fmod->set_parameter("Tension", myMusicIntensity);
		myOldIntensity = myMusicIntensity;
	}

	fmod->update_audio();
}
void AudioManager::SetPlayer(const Shared<GameObject>& aPlayer)
{
	myPlayer = aPlayer;
}

void AudioManager::SetListener(Transform& aTransform)
{
	fmod->set_listener(aTransform.GetPosition(), aTransform.Forward(), aTransform.Up());
	//LOG_INFO(LogType::Audio) << "Listener Pos: X: " << aTransform.GetPosition().x << ", Y: " << aTransform.GetPosition().y << ", Z: " << aTransform.GetPosition().z;
}

void AudioManager::PlayEvent2D(const std::string& anEventPath, const int aUUID)
{
	fmod->play_event(anEventPath, aUUID);
}

void AudioManager::PlayEvent3D(const std::string& anEventPath,
	const int aUUID,
	const Vec3f& pos,
	const Vec3f& forward,
	const Vec3f& up)
{
	fmod->play_event(anEventPath, aUUID, pos, forward, up);
	//LOG_INFO(LogType::Audio) << "Sound Pos: X: " << pos.x << ", Y: " << pos.y << ", Z: " << pos.z;
}

void AudioManager::ChangeParameter(const std::string& aParameterName, float aValue)
{
	fmod->set_parameter(aParameterName, aValue);
}

void AudioManager::ChangeParameter(int aUUID, const std::string& aEventPath, const std::string& aParameterName, float aValue)
{
	fmod->set_parameter(aUUID, aEventPath, aParameterName, aValue);
}

void AudioManager::ChangeParameter(const std::string& aParameterName, bool aValue)
{
}

void AudioManager::SetCameraPosition(Vec3f* aCameraPosition)
{
	cameraPosition = cameraPosition;
}

void AudioManager::StopAll()
{
	fmod->stop_all();
}
void AudioManager::Stop(const int aUUID)
{
	fmod->stop_events(aUUID);
}
void AudioManager::Stop(const std::string& aEventPath, const int aUUID)
{
	fmod->stop_event(aEventPath, aUUID);
}
void AudioManager::SetVolume(float aValue, eVolumeType aVolumeType)
{
	switch (static_cast<int>(aVolumeType))
	{
	case static_cast<int>(eVolumeType::MAIN):

		fmod->set_main_volume(aValue);
		break;

	case static_cast<int>(eVolumeType::SFX):

		fmod->set_sfx_volume(aValue);
		break;

	case static_cast<int>(eVolumeType::MUSIC):

		fmod->set_music_volume(aValue);
		break;

	case static_cast<int>(eVolumeType::AMB):

		fmod->set_ambiance_volume(aValue);
		break;
	}
}

void AudioManager::JsonUpdate()
{
	struct AudioSettingsJsonStruct
	{
		float myMusicVolume;
		float mySFXVolume;

		void Deserialize(nlohmann::json& aJson)
		{
			myMusicVolume = aJson["MusicVolume"];
			mySFXVolume = aJson["SFXVolume"];
		}
	};

	Engine::DebugMenu::AddMenuEntry("Audio Settings",
		[this]()
		{
			const AudioSettingsJsonStruct* audioSettingsJsonStruct
				= GetEngine().GetJsonManager().DeserializeAs<AudioSettingsJsonStruct>(
					"Assets/Json/AudioSettings.json");

			if (ImGui::SliderFloat("MusicVolume: %f", &fmod->get_music_volume(), 0.0f, 1.0f))
			{
				//fmod->set_volume(fmod->get_music_volume());
			}
		});
}

void AudioManager::SetVolumeWithJSON()
{
	std::ifstream i("Assets/Json/AudioSettings.json");
	nlohmann::json j;
	i >> j;

	fmod->set_main_volume(j["MainVolume"]);
	fmod->set_music_volume(j["MusicVolume"]);
	fmod->set_ambiance_volume(j["AmbianceVolume"]);
	fmod->set_sfx_volume(j["SFXVolume"]);
}

//void AudioManager::SetMusicVolume(float aVolumeValue)
//{
//	fmod->set_volume(aVolumeValue);
//}
//
//void AudioManager::SetSFXVolume(float aVolumeValue)
//{
//	fmod->set_volume(aVolumeValue);
//}
float AudioManager::GetSoundLength(const std::string& anEventPath)
{
	return fmod->get_sound_length(anEventPath) * 0.001f;
}
