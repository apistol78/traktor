/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Plane.h"
#include "Spray/ModifierData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::spray
{

/*! Plane collision modifier persistent data.
 * \ingroup Spray
 */
class T_DLLCLASS PlaneCollisionModifierData : public ModifierData
{
	T_RTTI_CLASS;

public:
	virtual Ref< const Modifier > createModifier(resource::IResourceManager* resourceManager) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	Plane m_plane = Plane(0.0f, 1.0f, 0.0f, 0.0f);
	float m_radius = 1.0f;
	float m_restitution = 1.0f;
};

}
