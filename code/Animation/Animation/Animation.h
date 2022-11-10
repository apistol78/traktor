/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Animation/Pose.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace animation
	{

/*! Key framed animation poses.
 * \ingroup Animation
 */
class T_DLLCLASS Animation : public ISerializable
{
	T_RTTI_CLASS;

public:
	struct T_DLLCLASS KeyPose
	{
		float at = 0.0f;
		Pose pose;

		void serialize(ISerializer& s);
	};

	/*! Add key pose to animation.
	 *
	 * \param pose Key pose.
	 * \return Index of added key pose.
	 */
	uint32_t addKeyPose(const KeyPose& pose);

	/*! Remove key pose from animation.
	 *
	 * \param poseIndex Index of key pose.
	 */
	void removeKeyPose(uint32_t poseIndex);

	/*! Return true if animation doesn't contain any poses.
	 *
	 * \return True if animation is empty.
	 */
	bool empty() const;

	/*! Get number of key poses in animation.
	 *
	 * \return Number of key poses.
	 */
	uint32_t getKeyPoseCount() const;

	/*! Get reference to key pose.
	 *
	 * \param poseIndex Index of key pose.
	 * \return Key pose.
	 */
	KeyPose& getKeyPose(uint32_t poseIndex);

	/*! Get reference to key pose.
	 *
	 * \param poseIndex Index of key pose.
	 * \return Key pose.
	 */
	const KeyPose& getKeyPose(uint32_t poseIndex) const;

	/*! Get last key pose.
	 *
	 * \return Last key pose.
	 */
	const KeyPose& getLastKeyPose() const;

	/*! Get key pose from time.
	 *
	 * \param at Time
	 * \param linear Use linear interpolation.
	 * \param indexHint Key frame index hint.
	 * \param outPose Output key pose.
	 * \return True if found key pose.
	 */
	bool getPose(float at, bool linear, int32_t& indexHint, Pose& outPose) const;

	virtual void serialize(ISerializer& s) override final;

private:
	AlignedVector< KeyPose > m_poses;
};

	}
}
