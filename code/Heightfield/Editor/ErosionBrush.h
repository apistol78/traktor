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

/*! Procedural brush which erode heights, fading out outside of layer.
 * \ingroup Heightfield
 */
class T_DLLCLASS ErosionBrush : public ProceduralBrush
{
	T_RTTI_CLASS;

public:
	virtual void apply(Heightfield* heightfield, const ProceduralLayer* layer) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	int32_t m_iterations = 100000;	//!< Number of water droplets.
	float m_resolution = 4.0f;		//!< Distance, in metres, between erosion samples.
	float m_falloff = 10.0f;		//!< Distance, in metres, outside of layer over which brush fade out.
};

}
