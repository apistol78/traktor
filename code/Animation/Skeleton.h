/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <functional>
#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Serialization/ISerializable.h"
#include "Render/Types.h"

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

class Joint;

/*! Animation skeleton.
 * \ingroup Animation
 */
class T_DLLCLASS Skeleton : public ISerializable
{
	T_RTTI_CLASS;

public:
	int32_t addJoint(Joint* joint);

	void removeJoint(Joint* joint);

	bool findJoint(render::handle_t name, uint32_t& outIndex) const;

	void findChildren(uint32_t index, const std::function< void(uint32_t) >& fn) const;

	void findAllChildren(uint32_t index, const std::function< void(uint32_t) >& fn) const;

	virtual void serialize(ISerializer& s) override final;

	uint32_t getJointCount() const { return (uint32_t)m_joints.size(); }

	Joint* getJoint(uint32_t index) const { return m_joints[index]; }

private:
	RefArray< Joint > m_joints;
	mutable SmallMap< render::handle_t, uint32_t > m_jointMap;
};

	}
}

