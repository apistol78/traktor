/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberSmallMap.h"
#include "Render/Editor/Texture/TextureSet.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.render.TextureSet", 0, TextureSet, ISerializable)

void TextureSet::serialize(ISerializer& s)
{
	s >> MemberSmallMap< std::wstring, Guid >(L"textures", m_textures);
}

}
