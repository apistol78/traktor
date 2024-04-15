/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Spray/Editor/PointSetAsset.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spray.PointSetAsset", 0, PointSetAsset, editor::Asset)

bool PointSetAsset::fromFaces() const
{
	return m_fromFaces;
}

void PointSetAsset::serialize(ISerializer& s)
{
	editor::Asset::serialize(s);
	s >> Member< bool >(L"fromFaces", m_fromFaces);
}

}
