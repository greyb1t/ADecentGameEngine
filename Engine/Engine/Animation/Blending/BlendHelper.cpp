#include "pch.h"
#include "BlendHelper.h"

#include "BlendMask.h"

Engine::AnimationTransformResult BlendHelper::Blend(
	const Engine::AnimationTransformResult& aResult1,
	const Engine::AnimationTransformResult& aResult2,
	const float aBlendPercent)
{
	ZoneScopedN("BlendHelper::Blend1");

	Engine::AnimationTransformResult result;

	assert(aResult2.myEntries.size() == aResult1.myEntries.size()
		&& "cannot blend results from that does not have same sekelton");

	for (size_t i = 0; i < aResult1.myEntries.size(); ++i)
	{
		const auto& entry1 = aResult1.myEntries[i];
		const auto& entry2 = aResult2.myEntries[i];

		const Vec3f scaling0 = entry1.myScale;
		const Vec3f scaling1 = entry2.myScale;
		const Vec3f blendedScaling = scaling0 * (1.f - aBlendPercent) + scaling1 * aBlendPercent;

		const Quatf rotationQ0 = entry1.myRotation;
		const Quatf rotationQ1 = entry2.myRotation;
		const Quatf blendedRotation = Quatf::Slerp(rotationQ0, rotationQ1, aBlendPercent);

		const Vec3f translation0 = entry1.myTranslation;
		const Vec3f translation1 = entry2.myTranslation;
		const Vec3f blendedTranslation = translation0 * (1.f - aBlendPercent) + translation1 * aBlendPercent;

		Engine::AnimationTransform entry;
		{
			entry.myScale = blendedScaling;
			entry.myRotation = blendedRotation;
			entry.myTranslation = blendedTranslation;
		}

		result.AddEntry(entry);
	}

	return result;
}

Engine::AnimationTransformResult BlendHelper::Blend(
	const Engine::AnimationTransformResult& aResult1,
	const Engine::AnimationTransformResult& aResult2,
	const Engine::AnimationTransformResult& aResult3,
	const Engine::AnimationTransformResult& aBindPose,
	const float aU,
	const float aV,
	const float aW)
{
	ZoneScopedN("BlendHelper::Blend2");

	Engine::AnimationTransformResult result;

	assert(aResult2.myEntries.size() == aResult1.myEntries.size() &&
		aResult1.myEntries.size() == aResult3.myEntries.size() &&
		"cannot blend results from that does not have same sekelton");

	for (size_t i = 0; i < aResult1.myEntries.size(); ++i)
	{
		const auto& entry1 = aResult1.myEntries[i];
		const auto& entry2 = aResult2.myEntries[i];
		const auto& entry3 = aResult3.myEntries[i];

		const Vec3f scaling0 = entry1.myScale;
		const Vec3f scaling1 = entry2.myScale;
		const Vec3f scaling2 = entry3.myScale;
		const Vec3f blendedScaling = scaling0 * aU + scaling1 * aV + scaling2 * aW;

		const Quatf rotationQ0 = entry1.myRotation;
		const Quatf rotationQ1 = entry2.myRotation;
		const Quatf rotationQ2 = entry3.myRotation;
		auto res0 = Quatf::Slerp({}, rotationQ0, aU);
		auto res1 = Quatf::Slerp({}, rotationQ1, aV);
		auto res2 = Quatf::Slerp({}, rotationQ2, aW);
		const Quatf blendedRotation = res0 * res1 * res2;

		const Vec3f translation0 = entry1.myTranslation;
		const Vec3f translation1 = entry2.myTranslation;
		const Vec3f translation2 = entry3.myTranslation;
		const Vec3f blendedTranslation = translation0 * aU + translation1 * aV + translation2 * aW;

		Engine::AnimationTransform entry;
		{
			entry.myScale = blendedScaling;
			entry.myRotation = blendedRotation;
			entry.myTranslation = blendedTranslation;
		}

		result.AddEntry(entry);
	}

	return result;
}

Engine::AnimationTransformResult BlendHelper::BlendAdditive(
	const Engine::AnimationTransformResult& aResult1,
	const Engine::AnimationTransformResult& aResult2,
	const Engine::AnimationTransformResult& aBindPose,
	const float aBlendPercent)
{
	ZoneScopedN("BlendHelper::BlendAdditive");

	Engine::AnimationTransformResult result;

	assert(aResult2.myEntries.size() == aResult1.myEntries.size()
		&& "cannot blend results from that does not have same sekelton");

	assert(aBindPose.myEntries.size() == aResult1.myEntries.size()
		&& "cannot blend results from that does not have same sekelton");

	for (size_t i = 0; i < aBindPose.myEntries.size(); ++i)
	{
		const auto& entry1 = aResult1.myEntries[i];
		const auto& entry2 = aResult2.myEntries[i];
		const auto& bind = aBindPose.myEntries[i];

		const Vec3f scaling0 = entry1.myScale;
		const Vec3f scaling1 = entry2.myScale;
		const Vec3f blendedScaling = scaling0 + (scaling1 - bind.myScale) * aBlendPercent;

		const Quatf rotationQ0 = entry1.myRotation;
		const Quatf rotationQ1 = entry2.myRotation;
		const Quatf blendedRotationTemp = rotationQ0 * (bind.myRotation.GetInverse() * rotationQ1);
		const Quatf blendedRotation = Quatf::Slerp(rotationQ0, blendedRotationTemp, aBlendPercent);

		const Vec3f translation0 = entry1.myTranslation;
		const Vec3f translation1 = entry2.myTranslation;
		const Vec3f blendedTranslation = translation0 + (translation1 - bind.myTranslation) * aBlendPercent;

		Engine::AnimationTransform entry;
		{
			entry.myScale = blendedScaling;
			entry.myRotation = blendedRotation;
			entry.myTranslation = blendedTranslation;
		}

		result.AddEntry(entry);
	}

	return result;
}

Engine::AnimationTransformResult BlendHelper::BlendMasked(
	const Engine::AnimationTransformResult& aResult1,
	const Engine::AnimationTransformResult& aResult2,
	const float aBlendPercent,
	const Engine::BlendMask& aMask)
{
	ZoneScopedN("BlendHelper::BlendMasked");

	Engine::AnimationTransformResult result;

	assert(aResult2.myEntries.size() == aResult1.myEntries.size()
		&& "cannot blend results from that does not have same sekelton");

	for (size_t i = 0; i < aResult1.myEntries.size(); ++i)
	{
		if (!aMask.IsEnabled(i))
		{
			// Returns the unblended result
			result.AddEntry(aResult1.myEntries[i]);
			continue;
		}

		const auto& entry1 = aResult1.myEntries[i];
		const auto& entry2 = aResult2.myEntries[i];

		const Vec3f scaling0 = entry1.myScale;
		const Vec3f scaling1 = entry2.myScale;
		const Vec3f blendedScaling = scaling0 * (1.f - aBlendPercent) + scaling1 * aBlendPercent;

		const Quatf rotationQ0 = entry1.myRotation;
		const Quatf rotationQ1 = entry2.myRotation;
		const Quatf blendedRotation = Quatf::Slerp(rotationQ0, rotationQ1, aBlendPercent);

		const Vec3f translation0 = entry1.myTranslation;
		const Vec3f translation1 = entry2.myTranslation;
		const Vec3f blendedTranslation = translation0 * (1.f - aBlendPercent) + translation1 * aBlendPercent;

		Engine::AnimationTransform entry;
		{
			entry.myScale = blendedScaling;
			entry.myRotation = blendedRotation;
			entry.myTranslation = blendedTranslation;
		}

		result.AddEntry(entry);
	}

	return result;
}

Engine::AnimationTransformResult BlendHelper::BlendAdditiveMasked(
	const Engine::AnimationTransformResult& aResult1,
	const Engine::AnimationTransformResult& aResult2,
	const Engine::AnimationTransformResult& aBindPose,
	const float aBlendPercent,
	const Engine::BlendMask& aMask)
{
	ZoneScopedN("BlendHelper::BlendAdditiveMasked");

	Engine::AnimationTransformResult result;

	assert(aResult2.myEntries.size() == aResult1.myEntries.size()
		&& "cannot blend results from that does not have same sekelton");

	assert(aBindPose.myEntries.size() == aResult1.myEntries.size()
		&& "cannot blend results from that does not have same sekelton");

	for (size_t i = 0; i < aBindPose.myEntries.size(); ++i)
	{
		if (!aMask.IsEnabled(i))
		{
			// Returns the unblended result
			result.AddEntry(aResult1.myEntries[i]);
			continue;
		}

		const auto& entry1 = aResult1.myEntries[i];
		const auto& entry2 = aResult2.myEntries[i];
		const auto& bind = aBindPose.myEntries[i];

		const Vec3f scaling0 = entry1.myScale;
		const Vec3f scaling1 = entry2.myScale;
		const Vec3f blendedScaling = scaling0 + (scaling1 - bind.myScale) * aBlendPercent;

		const Quatf rotationQ0 = entry1.myRotation;
		const Quatf rotationQ1 = entry2.myRotation;
		const Quatf blendedRotationTemp = rotationQ0 * (bind.myRotation.GetInverse() * rotationQ1);
		const Quatf blendedRotation = Quatf::Slerp(rotationQ0, blendedRotationTemp, aBlendPercent);

		const Vec3f translation0 = entry1.myTranslation;
		const Vec3f translation1 = entry2.myTranslation;
		const Vec3f blendedTranslation = translation0 + (translation1 - bind.myTranslation) * aBlendPercent;

		Engine::AnimationTransform entry;
		{
			entry.myScale = blendedScaling;
			entry.myRotation = blendedRotation;
			entry.myTranslation = blendedTranslation;
		}

		result.AddEntry(entry);
	}

	return result;
}
