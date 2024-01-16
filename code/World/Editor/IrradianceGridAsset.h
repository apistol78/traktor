/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Editor/Asset.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{
	
/*!
 * \ingroup World
 */
class T_DLLCLASS IrradianceGridAsset : public editor::Asset
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) override final;

	float getIntensity() const { return m_intensity; }

	float getSaturation() const { return m_saturation; }

	bool shouldCancelSun() const { return m_cancelSun; }

private:
	float m_intensity = 1.0f;
	float m_saturation = 1.0f;
	bool m_cancelSun = true;
};
	
}
