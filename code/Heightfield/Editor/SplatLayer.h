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

/*! Layer which rewrite splats of layer below from its heights and slopes, where its mask cover.
 * \ingroup Heightfield
 *
 * Each rule weights a splat channel by how well height and slope fit its ranges; weights of
 * all rules are normalized into splats, samples which no rule cover keep their splats.
 * Mask is stored in heightfield instance and cover entire heightfield unless stored.
 */
class T_DLLCLASS SplatLayer : public HeightfieldAssetLayer
{
	T_RTTI_CLASS;

public:
	//! Splat channel weighted by ranges of height and slope, fading out outside of ranges.
	struct Rule
	{
		int32_t channel = 0;
		float minHeight = -10000.0f;	//!< Metres.
		float maxHeight = 10000.0f;		//!< Metres.
		float heightBlend = 10.0f;		//!< Metres outside of height range over which weight fade out.
		float minSlope = 0.0f;			//!< Degrees.
		float maxSlope = 90.0f;			//!< Degrees.
		float slopeBlend = 5.0f;		//!< Degrees outside of slope range over which weight fade out.

		void serialize(ISerializer& s);
	};

	virtual bool apply(const TileStorage* storage, int32_t layer, const Vector4& worldExtent, int32_t tileX, int32_t tileZ, Heightfield* inoutTile) const override final;

	virtual Ref< Heightfield > read(const TileStorage* storage, int32_t layer, const Vector4& worldExtent, int32_t tileX, int32_t tileZ, AlignedVector< uint16_t >* outMask) const override final;

	virtual bool haveMask() const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	AlignedVector< Rule > m_rules;
};

}
