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
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Render/Shader.h"
#include "Render/Image2/ImagePassOpData.h"
#include "Resource/Member.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ImagePassOpData", ImagePassOpData, ISerializable)

void ImagePassOpData::serialize(ISerializer& s)
{
	s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> MemberAlignedVector< Source, MemberComposite< Source > >(L"sources", m_sources);
}

void ImagePassOpData::Source::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"textureId", textureId);
	s >> Member< std::wstring >(L"parameter", parameter);
}

}
