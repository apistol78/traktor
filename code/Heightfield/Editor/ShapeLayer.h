/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Vector4.h"
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

/*! Layer which stroke a path, along a spline through keys, setting heights of layer below.
 * \ingroup Heightfield
 *
 * Height along path is interpolated from height of keys;
 * outside of path it fades into heights of layer below.
 */
class T_DLLCLASS ShapeLayer : public HeightfieldAssetLayer
{
	T_RTTI_CLASS;

public:
	virtual bool apply(const TileStorage* storage, int32_t layer, const Vector4& worldExtent, int32_t tileX, int32_t tileZ, Heightfield* heightfield) const override final;

	virtual Ref< Heightfield > read(const TileStorage* storage, int32_t layer, const Vector4& worldExtent, int32_t tileX, int32_t tileZ, AlignedVector< uint16_t >* outMask) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	AlignedVector< Vector4 > m_keys;	//!< Keys in world, X and Z position and Y height in metres.
	float m_width = 10.0f;				//!< Width of path in metres.
	float m_falloff = 10.0f;			//!< Distance, in metres, outside of path over which heights fade out.
};

}
