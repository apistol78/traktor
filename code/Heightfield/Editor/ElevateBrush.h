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

/*! Procedural brush which increase height, fading out outside of layer.
 * \ingroup Heightfield
 */
class T_DLLCLASS ElevateBrush : public ProceduralBrush
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) override final;

protected:
	virtual float getReach() const override final;

	virtual float sample(float worldX, float worldZ, float spacing, float height, float distance, float layerHeight) const override final;

private:
	float m_height = 10.0f;		//!< Height, in metres, added; negative lowers.
	float m_falloff = 10.0f;	//!< Distance, in metres, outside of layer over which brush fade out.
};

}
