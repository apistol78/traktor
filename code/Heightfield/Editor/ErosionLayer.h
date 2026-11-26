/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Heightfield/Editor/ErosionFilter.h"
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

/*! Layer which erode heights of layer below where its mask cover.
 * \ingroup Heightfield
 *
 * Erosion is calculated over entire heightfield at once, at resolution of layer.
 * Mask is stored in heightfield instance and cover entire heightfield unless stored.
 */
class T_DLLCLASS ErosionLayer : public HeightfieldAssetLayer
{
	T_RTTI_CLASS;

public:
	virtual bool apply(const TileStorage* storage, int32_t layer, const Vector4& worldExtent, int32_t tileX, int32_t tileZ, Heightfield* inoutTile) const override final;

	virtual Ref< Heightfield > read(const TileStorage* storage, int32_t layer, const Vector4& worldExtent, int32_t tileX, int32_t tileZ, AlignedVector< uint8_t >* outMask) const override final;

	virtual bool isGlobal() const override final;

	virtual bool haveMask() const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	ErosionFilter::Parameters m_parameters;
	float m_resolution = 4.0f; //!< Distance, in metres, between erosion samples; zero to erode at resolution of heightfield.

	//! Read entire heightfield, of layers below merged, at resolution of layer, both before and after it's eroded; cached.
	bool readEroded(const TileStorage* storage, int32_t layer, const Vector4& worldExtent, Ref< const Heightfield >& outBelow, Ref< const Heightfield >& outEroded) const;
};

}
