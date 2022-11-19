#pragma once
#include "Engine/GameObject/GameObject.h"

class fmod_audio;
class Transform;

enum class eVolumeType
{
	MAIN = 0,
	SFX,
	MUSIC,
	AMB
};

class AudioManager
{
public:
	static AudioManager* GetInstance();

	AudioManager();
	~AudioManager();

	void SetMusicIntensity(const float& aMusicIntensity);
	
	void Init(float aSoundReach);
	void Update();
	void SetListener(Transform& aTransform);
	void SetListener(Vec3f& aPos, Vec3f& aForward, Vec3f& aUp);
	void PlayEvent2D(const std::string& anEventPath, const int aUUID);
	void PlayEvent3D(const std::string& anEventPath, const int aUUID, const Vec3f& pos, const Vec3f& forward, const Vec3f& up);
	void ChangeParameter(const std::string& aParameterName, float aValue);
	void ChangeParameter(int aUUID, const std::string& aEventpath, const std::string& aParameterName, float aValue);
	void ChangeParameter(const std::string& aParameterName, bool aValue);
	void SetCameraPosition(Vec3f* aCameraPosition);
	void SetReverb();
	void StopAll();
	void SetPauseAll(bool aPaused);
	void SetPauseEvent(const std::string& aEventPath, bool aPaused);
	void ReleaseAudio(const int aUUID);
	void LoadEvent(const int& aUUID, const std::string& aPath);
	bool IsInstanceLoaded(const int& aUUID, const std::string& aPath);

	void Stop(const int aUUID);
	void Stop(const std::string& aEventPath, const int aUUID);

	void SetVolume(float aValue, eVolumeType aVolumeType); 
	void SetPlayer(const Shared<GameObject>& aPlayer);

	void JsonUpdate();

	void SetVolumeWithJSON();

	float GetSoundLength(const std::string& anEventPath);
	
private:
	Owned<fmod_audio> fmod = nullptr;
	Weak<GameObject> myPlayer;
	unsigned int count = 0;
	float musicVolume = 1.0f;
	float sFXVolume = 1.0f;
	Vec3f cameraPosition; //for 3D sound listener
	std::vector<std::string> eventPaths;
	float myMusicIntensity = 0;
	float myOldIntensity = 0;
};

