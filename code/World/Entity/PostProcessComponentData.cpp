/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Entity/PostProcessComponentData.h"

#include "Core/Serialization/ISerializer.h"
#include "Render/Image2/ImageGraph.h"
#include "Resource/Member.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.PostProcessComponentData", 0, PostProcessComponentData, IWorldComponentData)

void PostProcessComponentData::serialize(ISerializer& s)
{
	s >> resource::Member< render::ImageGraph >(L"imageGraph", m_imageGraph);
}

}
