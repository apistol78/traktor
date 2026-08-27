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
#include "World/IEntityComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

class DisplacementEntityComponent;

/*! Displacement volume component data.
 * \ingroup World
 */
class T_DLLCLASS DisplacementEntityComponentData : public IEntityComponentData
{
	T_RTTI_CLASS;

public:
	Ref< DisplacementEntityComponent > createComponent() const;

	virtual int32_t getOrdinal() const override final;

	virtual void setTransform(const EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

	float getRadius() const { return m_radius; }

	float getStrength() const { return m_strength; }

	float getFalloff() const { return m_falloff; }

	float getPress() const { return m_press; }

private:
	float m_radius = 1.0f;	 //!< Radius of influence, in world units.
	float m_strength = 1.0f; //!< Peak coverage written into the mask.
	float m_falloff = 2.0f;	 //!< Exponent of the radial falloff; higher is tighter.
	float m_press = 1.0f;	 //!< Vertical press written into the mask.
};

}
