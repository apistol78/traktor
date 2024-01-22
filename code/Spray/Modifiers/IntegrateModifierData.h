/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
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

namespace traktor::spray
{

/*! Integrate modifier persistent data.
 * \ingroup Spray
 */
class T_DLLCLASS IntegrateModifierData : public ModifierData
{
	T_RTTI_CLASS;

public:
	virtual Ref< const Modifier > createModifier(resource::IResourceManager* resourceManager) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	float m_timeScale = 1.0f;
	bool m_linear = true;
	bool m_angular = true;
};

}
