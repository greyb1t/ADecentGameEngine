#pragma once

namespace Engine
{
	struct AnimationTransform
	{
		Vec3f myScale = { 1.f, 1.f, 1.f };
		Quatf myRotation;
		Vec3f myTranslation;
		std::string myDebugData;

		void InitFromJson(const nlohmann::json& aJson)
		{
			myScale.x = aJson["Scale"][0];
			myScale.y = aJson["Scale"][1];
			myScale.z = aJson["Scale"][2];

			myTranslation.x = aJson["Translation"][0];
			myTranslation.y = aJson["Translation"][1];
			myTranslation.z = aJson["Translation"][2];

			myRotation.myVector.x = aJson["RotQuat"][0];
			myRotation.myVector.y = aJson["RotQuat"][1];
			myRotation.myVector.z = aJson["RotQuat"][2];
			myRotation.myW = aJson["RotQuat"][3];

			if (aJson.contains("DebugData"))
			{
				myDebugData = aJson["DebugData"];
			}
		}

		nlohmann::json ToJson() const
		{
			nlohmann::json j;

			j["Scale"][0] = myScale.x;
			j["Scale"][1] = myScale.y;
			j["Scale"][2] = myScale.z;

			j["Translation"][0] = myTranslation.x;
			j["Translation"][1] = myTranslation.y;
			j["Translation"][2] = myTranslation.z;

			j["RotQuat"][0] = myRotation.myVector.x;
			j["RotQuat"][1] = myRotation.myVector.y;
			j["RotQuat"][2] = myRotation.myVector.z;
			j["RotQuat"][3] = myRotation.myW;

			j["DebugData"] = myDebugData;

			return j;
		}
	};

	class AnimationTransformResult
	{
	public:
		void InitFromJson(const nlohmann::json& aJson);
		nlohmann::json ToJson() const;

		void AddEntry(const AnimationTransform& aEntry)
		{
			myEntries.push_back(aEntry);
		}

		std::vector<AnimationTransform> myEntries;
	};
}