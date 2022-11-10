/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
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

namespace traktor
{
	namespace world
	{
	
class T_DLLCLASS IrradianceGridAsset : public editor::Asset
{
	T_RTTI_CLASS;

public:
	IrradianceGridAsset();

	virtual void serialize(ISerializer& s) override final;

	float getIntensity() const { return m_intensity; }

private:
	float m_intensity;
};
	
	}
}