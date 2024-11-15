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
#include "Render/Resource/ProgramResource.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_VERSION_CLASS(L"traktor.render.ProgramResource", 0, ProgramResource, ISerializable)

void ProgramResource::serialize(ISerializer& s)
{
    s >> Member< bool >(L"requireRayTracing", m_requireRayTracing);
}

}
