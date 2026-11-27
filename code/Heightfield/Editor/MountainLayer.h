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

/*! Procedural layer which add ridged mountains onto heights of layer below where its mask cover.
 * \ingroup Heightfield
 *
 * Mountains are ridged noise squared, scaled by height and by coverage; coverage is
 * low frequency noise which vary how tall mountains are across heightfield.
 * Heights above cap height are compressed asymptotically toward cap height plus cap softness.
 * Mask is stored in heightfield instance and cover entire heightfield unless stored.
 */
class T_DLLCLASS MountainLayer : public HeightfieldAssetLayer
{
	T_RTTI_CLASS;

public:
	virtual bool apply(const TileStorage* storage, int32_t layer, const Vector4& worldExtent, int32_t tileX, int32_t tileZ, Heightfield* inoutTile) const override final;

	virtual Ref< Heightfield > read(const TileStorage* storage, int32_t layer, const Vector4& worldExtent, int32_t tileX, int32_t tileZ, AlignedVector< uint16_t >* outMask) const override final;

	virtual bool haveMask() const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	int32_t m_seed = 0;
	float m_scale = 1100.0f;  //!< Size, in metres, of largest mountain features.
	int32_t m_octaves = 6;	  //!< Number of octaves of ridges.
	float m_height = 1400.0f; //!< Height, in metres, of mountains at full coverage.
	int32_t m_coverageSeed = 1;
	float m_coverageScale = 2000.0f;  //!< Size, in metres, of coverage features.
	float m_coverage = 0.6f;		  //!< Mean coverage, zero no mountains and one full height.
	float m_coverageVariation = 0.4f; //!< Coverage vary by this much around mean.
	float m_capHeight = 850.0f;		  //!< Height, in metres, above which heights are compressed.
	float m_capSoftness = 150.0f;	  //!< Metres which compressed heights approach above cap height; zero clamp at cap height.
};

}
