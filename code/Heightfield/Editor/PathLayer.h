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
#include "Heightfield/Editor/ProceduralLayer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_HEIGHTFIELD_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::hf
{

/*! Procedural layer covering a path along a spline through keys.
 * \ingroup Heightfield
 *
 * Height of layer is interpolated from height of keys.
 */
class T_DLLCLASS PathLayer : public ProceduralLayer
{
	T_RTTI_CLASS;

public:
	virtual void evaluate(const Heightfield* heightfield, int32_t gridZ, float reach, float* outDistances, float* outHeights) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	AlignedVector< Vector4 > m_keys;	//!< Keys in world, X and Z position and Y height in metres.
	float m_width = 10.0f;			//!< Width of path in metres.
};

}
