/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_HEIGHTFIELD_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::hf
{

class Heightfield;
class ProceduralLayer;

/*! Procedural brush, modifies heights inside of a layer.
 * \ingroup Heightfield
 */
class T_DLLCLASS ProceduralBrush : public ISerializable
{
	T_RTTI_CLASS;

public:
	//! Apply brush to heightfield inside of layer; by default each sample within reach of layer is modified by sample().
	virtual void apply(Heightfield* heightfield, const ProceduralLayer* layer) const;

protected:
	//! Distance, in metres, outside of layer which brush reach.
	virtual float getReach() const;

	/*! New height of a sample.
	 *
	 * \param worldX Position of sample.
	 * \param worldZ Position of sample.
	 * \param spacing Distance, in metres, between samples.
	 * \param height Current height, in metres.
	 * \param distance Signed distance, in metres, to edge of layer, positive inside.
	 * \param layerHeight Height of layer, in metres.
	 * \return New height, in metres.
	 */
	virtual float sample(float worldX, float worldZ, float spacing, float height, float distance, float layerHeight) const;

	//! Smooth weight; one inside of layer, fading to zero at falloff distance outside.
	static float falloffWeight(float distance, float falloff);
};

}
