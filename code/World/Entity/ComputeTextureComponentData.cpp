/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Render/ITexture.h"
#include "Resource/Member.h"
#include "World/Entity/ComputeTextureComponentData.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.ComputeTextureComponentData", 0, ComputeTextureComponentData, IWorldComponentData)

void ComputeTextureComponentData::serialize(ISerializer& s)
{
	s >> MemberAlignedVector< resource::Id< render::ITexture >, resource::Member< render::ITexture > >(L"textures", m_textures);
}

}
