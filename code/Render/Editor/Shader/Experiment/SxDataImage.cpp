/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Shader/Experiment/SxDataImage.h"

#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.render.SxDataImage", 0, SxDataImage, SxData)

void SxDataImage::serialize(ISerializer& s)
{
    SxData::serialize(s);
	s >> Member< int32_t >(L"width", m_width);
	s >> Member< int32_t >(L"height", m_height);
}

}
