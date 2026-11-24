/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
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
class ProceduralLayer;

/*! Procedural heightfield asset.
 * \ingroup Heightfield
 *
 * Rules, as layers each with a brush, which heights
 * of a heightfield asset are generated from.
 */
class T_DLLCLASS ProceduralHeightfieldAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	//! Generate heights; heightfield is flattened to world height zero, then layers are applied in order.
	void generate(Heightfield* heightfield) const;

	virtual void serialize(ISerializer& s) override final;

private:
	RefArray< ProceduralLayer > m_layers;
};

}
