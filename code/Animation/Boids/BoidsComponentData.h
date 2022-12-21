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

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

class EntityData;
class IEntityBuilder;

}

namespace traktor::animation
{

class BoidsComponent;

/*!
 * \ingroup Animation
 */
class T_DLLCLASS BoidsComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	BoidsComponentData();

	Ref< BoidsComponent > createComponent(const world::IEntityBuilder* builder) const;

	virtual void setTransform(const world::EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

	const Ref< world::EntityData >& getBoidEntityData() const { return m_boidEntityData; }

private:
	Ref< world::EntityData > m_boidEntityData;
	uint32_t m_boidCount;
	Vector4 m_spawnPositionDiagonal;
	Vector4 m_spawnVelocityDiagonal;
	Vector4 m_constrain;
	float m_followForce;
	float m_repelDistance;
	float m_repelForce;
	float m_matchVelocityStrength;
	float m_centerForce;
	float m_maxVelocity;
};

}
