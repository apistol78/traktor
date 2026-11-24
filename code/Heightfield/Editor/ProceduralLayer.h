/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
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
class ProceduralBrush;

/*! Procedural layer, area of heightfield which a brush is applied to.
 * \ingroup Heightfield
 */
class T_DLLCLASS ProceduralLayer : public ISerializable
{
	T_RTTI_CLASS;

public:
	/*! Evaluate a row of heightfield samples.
	 *
	 * \param heightfield Heightfield positioned in world.
	 * \param gridZ Row of samples.
	 * \param reach Distance, in metres, outside of layer of interest; samples further outside get -reach.
	 * \param outDistances Signed distance, in metres, to edge of layer, positive inside; one per sample.
	 * \param outHeights Height of layer, in metres, zero if layer has no heights; one per sample.
	 */
	virtual void evaluate(const Heightfield* heightfield, int32_t gridZ, float reach, float* outDistances, float* outHeights) const = 0;

	virtual void serialize(ISerializer& s) override;

	const ProceduralBrush* getBrush() const { return m_brush; }

private:
	Ref< ProceduralBrush > m_brush;
};

}
