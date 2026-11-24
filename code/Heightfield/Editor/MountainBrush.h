/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Heightfield/Editor/ProceduralBrush.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_HEIGHTFIELD_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::hf
{

/*! Procedural brush which raise a ridged mountain, without erosion, inside of layer.
 * \ingroup Heightfield
 */
class T_DLLCLASS MountainBrush : public ProceduralBrush
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) override final;

protected:
	virtual float sample(float worldX, float worldZ, float spacing, float height, float distance, float layerHeight) const override final;

private:
	float m_height = 200.0f;	//!< Height, in metres, of mountain.
	float m_width = 500.0f;		//!< Distance, in metres, inside edge of layer where mountain reach full height.
	float m_scale = 1000.0f;	//!< Size, in metres, of largest ridges.
	float m_roughness = 0.75f;	//!< Amount of ridges, zero is a smooth mountain.
	uint32_t m_seed = 0;
};

}
