/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Spray/ModifierData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spray
	{

/*! Vortex modifier persistent data.
 * \ingroup Spray
 */
class T_DLLCLASS VortexModifierData : public ModifierData
{
	T_RTTI_CLASS;

public:
	VortexModifierData();

	virtual Ref< const Modifier > createModifier(resource::IResourceManager* resourceManager) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	Vector4 m_axis;
	float m_tangentForce;			//< Amount of force applied to each particle in tangent direction.
	float m_normalConstantForce;	//< Amount of constant force applied to each particle in normal direction.
	float m_normalDistance;			//< Distance from axis for each particle, scaled together with m_normalDistanceForce in order to apply different amount of force based on distance.
	float m_normalDistanceForce;
	bool m_world;
};

	}
}

