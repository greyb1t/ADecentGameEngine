#pragma once

#include <string>

namespace FB
{
	namespace Constants
	{
		const static std::string JabTriggerName = "Jab";
		const static std::string JabIsFinishedBool = "IsJabFinished";
		const static std::string JabStateIntName = "JabState";

		const static std::string SlamTriggerName = "StartSlam";
		const static std::string SlamStateIntName = "SlamState";

		const static std::string HomingLaserTriggerName = "HomingLaser";

		const static std::string GatlingEyeTriggerName = "GatlingEye";
		const static std::string GatlingEyeStateIntName = "GatlingEyeState";

		const static std::string CrawlEyeTriggerName = "Crawl";
		const static std::string CrawlEyeStateIntName = "CrawlState";

		const static std::string DeathTriggerName = "Death";
		const static std::string PhaseSwitchTriggerName = "SwitchPhase";

		const static std::string EnragedTriggerName = "Enraged";

		const static std::string LaserTriggerName = "Laser";
		const static std::string MortarTriggerName = "Mortar";
		const static std::string IntroTriggerName = "Intro";
		const static std::string PoisonTriggerName = "Poison";

		const static std::string GigaLaserSpawnPosBoneName = "Eye";

		const static float RaycastDistance = 1000000.f;

		const static float CloseEnough = 50.f * 50.f;
	}
}