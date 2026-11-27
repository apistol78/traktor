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

/*! Layer which terrace heights of layer below where its mask cover.
 * \ingroup Heightfield
 *
 * Heights are shaped into steps, flat treads at each terrace level with risers between;
 * noise vary height of terrace levels across heightfield. Terraces are only within range
 * of heights, faded out over one step outside of range.
 * Mask is stored in heightfield instance and cover entire heightfield unless stored.
 */
class T_DLLCLASS TerraceLayer : public HeightfieldAssetLayer
{
	T_RTTI_CLASS;

public:
	virtual bool apply(const TileStorage* storage, int32_t layer, const Vector4& worldExtent, int32_t tileX, int32_t tileZ, Heightfield* inoutTile) const override final;

	virtual Ref< Heightfield > read(const TileStorage* storage, int32_t layer, const Vector4& worldExtent, int32_t tileX, int32_t tileZ, AlignedVector< uint16_t >* outMask) const override final;

	virtual bool haveMask() const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	float m_stepHeight = 20.0f;	   //!< Height, in metres, between terrace levels.
	float m_offset = 0.0f;		   //!< Height, in metres, of one terrace level.
	float m_sharpness = 0.75f;	   //!< Zero keep heights, one give flat treads and vertical risers.
	float m_noise = 0.0f;		   //!< Metres which height of terrace levels vary by noise.
	float m_noiseScale = 250.0f;   //!< Size, in metres, of noise features.
	float m_minHeight = -10000.0f; //!< Lowest height, in metres, which is terraced.
	float m_maxHeight = 10000.0f;  //!< Highest height, in metres, which is terraced.
};

}
