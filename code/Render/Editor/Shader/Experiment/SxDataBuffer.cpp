/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Shader/Experiment/SxDataBuffer.h"

#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Render/Editor/Shader/StructDeclaration.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.render.SxDataBuffer", 0, SxDataBuffer, SxData)

void SxDataBuffer::serialize(ISerializer& s)
{
    SxData::serialize(s);
	s >> Member< Guid >(L"structDeclaration", m_structDeclaration, AttributeType(type_of< StructDeclaration >()));
	s >> Member< int32_t >(L"count", m_count);
}

}
