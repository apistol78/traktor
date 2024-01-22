/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Vector4.h"
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

/*! Gravity modifier persistent data.
 * \ingroup Spray
 */
class T_DLLCLASS GravityModifierData : public ModifierData
{
	T_RTTI_CLASS;

public:
	virtual Ref< const Modifier > createModifier(resource::IResourceManager* resourceManager) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	Vector4 m_gravity = Vector4(0.0f, -9.12f, 0.0f);
	bool m_world = true;
};

}
