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

/*! Layer which data is edited by user, in heightfield editor or in scene editor.
 * \ingroup Heightfield
 *
 * Data of layer is stored in heightfield instance and replace output of
 * layer below where its mask cover; mask cover nothing until edited.
 * Base layer has no mask and cover entire heightfield.
 * Heights of delta layer are stored relative to output of layer below,
 * thus follow when layers below change.
 */
class T_DLLCLASS EditLayer : public HeightfieldAssetLayer
{
	T_RTTI_CLASS;

public:
	//! Stored height, in units, of zero delta; world zero, as on empty tiles.
	static constexpr uint16_t ZeroDelta = 32768;

	virtual bool apply(const TileStorage* storage, int32_t layer, const Vector4& worldExtent, int32_t tileX, int32_t tileZ, Heightfield* inoutTile) const override final;

	virtual Ref< Heightfield > read(const TileStorage* storage, int32_t layer, const Vector4& worldExtent, int32_t tileX, int32_t tileZ, AlignedVector< uint16_t >* outMask) const override final;

	virtual bool haveData() const override final;

	virtual bool haveMask() const override final;

	virtual void serialize(ISerializer& s) override final;

	/*! Convert stored heights of layer into deltas, or deltas into heights; output of layer is kept.
	 * \param delta True to convert heights into deltas, false to convert deltas into heights.
	 */
	bool convertHeights(const TileStorage* storage, int32_t layer, const Vector4& worldExtent, bool delta) const;

	bool isDelta() const { return m_delta; }

private:
	bool m_delta = false; //!< Heights are stored relative to output of layer below.
};

}
