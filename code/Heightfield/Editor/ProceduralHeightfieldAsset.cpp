/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Heightfield/Editor/ProceduralHeightfieldAsset.h"

#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Heightfield/Editor/ProceduralBrush.h"
#include "Heightfield/Editor/ProceduralLayer.h"
#include "Heightfield/Heightfield.h"

namespace traktor::hf
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.hf.ProceduralHeightfieldAsset", 0, ProceduralHeightfieldAsset, ISerializable)

void ProceduralHeightfieldAsset::generate(Heightfield* heightfield) const
{
	const int32_t size = heightfield->getSize();
	const float zero = heightfield->worldToUnit(0.0f);
	for (int32_t z = 0; z < size; ++z)
		for (int32_t x = 0; x < size; ++x)
			heightfield->setGridHeight(x, z, zero);

	for (const ProceduralLayer* layer : m_layers)
	{
		if (layer && layer->getBrush())
			layer->getBrush()->apply(heightfield, layer);
	}
}

void ProceduralHeightfieldAsset::serialize(ISerializer& s)
{
	s >> MemberRefArray< ProceduralLayer >(L"layers", m_layers);
}

}
