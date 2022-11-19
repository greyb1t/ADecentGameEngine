#pragma once

namespace Engine
{
	struct PerlinShakeDesc;
	struct KickShakeDesc;

	struct ShakesJson
	{
		void Deserialize(nlohmann::json& aJson);

		std::unordered_map<std::string, PerlinShakeDesc*> myPerlinShakes;
		std::unordered_map<std::string, KickShakeDesc*> myKickShakes;
	};

	enum class ShakeType
	{
		PerlinShake,
		KickShake
	};

	class CameraShakeManager
	{
	public:
		bool Init(JsonManager& aJsonManager);

		const PerlinShakeDesc& GetPerlinShake(const std::string& aName) const;
		const KickShakeDesc& GetKickShake(const std::string& aName) const;

		const std::unordered_map<std::string, PerlinShakeDesc*>& GetPerlinShakes() const;

		std::vector<std::string> GetPerlinShakeNames() const;
		std::vector<std::string> GetKickShakeNames() const;

	private:
		const ShakesJson* myShakesJson = nullptr;
	};
}