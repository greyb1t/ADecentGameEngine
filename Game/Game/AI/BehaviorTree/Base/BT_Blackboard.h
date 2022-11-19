#pragma once
#include "AI/Enemies/Base/EnemyUtility.h"
#include "Engine/Animation/AnimationController.h"

namespace Engine
{
	class AudioComponent;
}

class AIState;
class GameObject;

class BT_Blackboard
{
public:
	BT_Blackboard(GameObject* aOwner = nullptr, Engine::AnimationController* aAnimationController = nullptr, AIState* aAIState = nullptr) :
		myOwner(aOwner),
		myAnimationController(aAnimationController),
		myAIState(aAIState)
	{
		int e = 9;
	};

	GameObject* GetOwner();
	void SetAnimationController(Engine::AnimationController* aAnimationController);
	Engine::AnimationController* GetAnimationController();

	void SetAudioComponent(Engine::AudioComponent* aAudioComponent);
	Engine::AudioComponent* GetAudioComponent() const;


	void setBool(std::string key, bool value) { bools[key] = value; }
	bool getBool(std::string key);

	bool hasBool(std::string key) const { return bools.find(key) != bools.end(); }

	void setInt(std::string key, int value) { ints[key] = value; }
	int getInt(std::string key);

	bool hasInt(std::string key) const { return ints.find(key) != ints.end(); }

	void setFloat(std::string key, float value) { floats[key] = value; }
	float& getFloat(std::string key);

	bool hasFloat(std::string key) const { return floats.find(key) != floats.end(); }

	void setDouble(std::string key, double value) { doubles[key] = value; }
	double getDouble(std::string key);

	bool hasDouble(std::string key) const { return doubles.find(key) != doubles.end(); }

	void setString(std::string key, std::string value) { strings[key] = value; }
	std::string getString(std::string key);

	void setVec3f(std::string key, Vec3f value) { vec3fs[key] = value; }
	Vec3f getVec3f(std::string key);

	bool hasString(std::string key) const { return strings.find(key) != strings.end(); }

	//using Ptr = std::shared_ptr<BT_Blackboard>;
	using Ptr = BT_Blackboard*;

	Enemy::eRapidShootData& GetRapidShootData();
	Enemy::eGroundPopcornShootData& GetGroundPopcornShootData();
	Enemy::eBurstShootData& GetBurstShootData();
	Enemy::eMortarShootData& GetMortarShootData();

	void SetGameObject(GameObject* aGameObject);
protected:
	GameObject* myOwner = nullptr;
	Engine::AnimationController* myAnimationController = nullptr;
	Engine::AudioComponent* myAudioComponent = nullptr;
	AIState* myAIState = nullptr;

	std::unordered_map<std::string, bool> bools;
	std::unordered_map<std::string, int> ints;
	std::unordered_map<std::string, float> floats;
	std::unordered_map<std::string, double> doubles;
	std::unordered_map<std::string, std::string> strings;
	std::unordered_map<std::string, Vec3f> vec3fs;

	Enemy::eRapidShootData myRapidShootData;
	Enemy::eGroundPopcornShootData myGroundPopcornShootData;
	Enemy::eBurstShootData myBurstShootData;
	Enemy::eMortarShootData myMortarShootData;
};
