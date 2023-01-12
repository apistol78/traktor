/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Render/Editor/Texture/BfnTextureAsset.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.render.BfnTextureAsset", 1, BfnTextureAsset, ISerializable)

void BfnTextureAsset::serialize(ISerializer& s)
{
	s >> Member< bool >(L"bestFitFactorOnly", m_bestFitFactorOnly);
	if (s.getVersion() >= 1)
	{
		s >> Member< bool >(L"collapseSymmetry", m_collapseSymmetry);
		s >> Member< int32_t >(L"size", m_size);
	}
}

}
