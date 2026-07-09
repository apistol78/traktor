/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Transform.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::physics
{

class DynamicBodyDesc;
class JointDesc;

}

namespace traktor::animation
{

/*!
 * \ingroup Animation
 */
class T_DLLCLASS RagDollSkeleton : public ISerializable
{
	T_RTTI_CLASS;

public:
	struct Influence
	{
		std::wstring joint;
		float weight = 1.0f;

		void serialize(ISerializer& s);
	};

	struct Limb
	{
		Transform transform;
		Ref< physics::DynamicBodyDesc > body;
		AlignedVector< Influence > influences;

		void serialize(ISerializer& s);
	};

	struct Joint
	{
		Ref< physics::JointDesc > joint;
		int32_t body1 = -1;
		int32_t body2 = -1;

		void serialize(ISerializer& s);
	};

	virtual void serialize(ISerializer& s) override;

	const AlignedVector< Limb >& getLimbs() const { return m_limbs; }

	const AlignedVector< Joint >& getJoints() const { return m_joints; }

private:
	friend class RagDollSkeletonPipeline;

	AlignedVector< Limb > m_limbs;
	AlignedVector< Joint > m_joints;
};

}
