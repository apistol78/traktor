/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

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

/*! Procedural layer covering entire heightfield.
 * \ingroup Heightfield
 */
class T_DLLCLASS AllLayer : public ProceduralLayer
{
	T_RTTI_CLASS;

public:
	virtual void evaluate(const Heightfield* heightfield, int32_t gridZ, float reach, float* outDistances, float* outHeights) const override final;

	virtual void serialize(ISerializer& s) override final;
};

}
