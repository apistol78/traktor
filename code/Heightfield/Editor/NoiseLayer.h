/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Heightfield/Editor/HeightfieldAssetLayer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_HEIGHTFIELD_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::hf
{

/*! Procedural layer which add fractal noise onto heights of layer below where its mask cover.
 * \ingroup Heightfield
 *
 * Added height is offset plus amplitude times noise, noise is in [-1, 1].
 * Mask is stored in heightfield instance and cover entire heightfield unless stored.
 */
class T_DLLCLASS NoiseLayer : public HeightfieldAssetLayer
{
	T_RTTI_CLASS;

public:
	virtual bool apply(const TileStorage* storage, int32_t layer, const Vector4& worldExtent, int32_t tileX, int32_t tileZ, Heightfield* inoutTile) const override final;

	virtual Ref< Heightfield > read(const TileStorage* storage, int32_t layer, const Vector4& worldExtent, int32_t tileX, int32_t tileZ, AlignedVector< uint16_t >* outMask) const override final;

	virtual bool haveMask() const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	int32_t m_seed = 0;
	float m_scale = 500.0f;	   //!< Size, in metres, of largest noise features.
	int32_t m_octaves = 4;	   //!< Number of octaves, each half the size and amplitude of previous.
	float m_amplitude = 10.0f; //!< Metres which noise vary height by.
	float m_offset = 0.0f;	   //!< Metres added to height everywhere.
};

}
