/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Shader/Experiment/SxData.h"

#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_VERSION_CLASS(L"traktor.render.SxData", 0, SxData, ISerializable)

void SxData::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", m_name);
}

}
