/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "World/IEntityComponentData.h"

#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::physics
{

class PhysicsManager;
class JointComponent;
class JointDesc;

/*!
 * \ingroup Physics
 */
class T_DLLCLASS JointComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	JointComponentData() = default;

	explicit JointComponentData(JointDesc* jointDesc);

	Ref< JointComponent > createComponent(PhysicsManager* physicsManager) const;

	virtual void setTransform(const world::EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

	const JointDesc* getJointDesc() const { return m_jointDesc; }

private:
	Ref< JointDesc > m_jointDesc;
};

}
