/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Render/Editor/Texture/TextureAsset.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.render.TextureAsset", 17, TextureAsset, editor::Asset)

void TextureAsset::serialize(ISerializer& s)
{
	editor::Asset::serialize(s);
	s >> MemberComposite< TextureOutput, false >(L"output", m_output);
}

}
